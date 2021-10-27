#include <cJSON.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_sntp.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include <math.h>
#include <nvs_flash.h>
#include <sys/time.h>
#include "cloud_client.h"
#include "motors_controller.h"
#include "sensor.h"
#include "sun_calculator.h"
#include "types.h"
#include "wifi_connector.h"

#define PI 3.14159265358979323846
#define rad (PI / 180.f)

typedef enum control_mode_t
{
    AUTOMATIC,
    MANUAL
} control_mode_t;

typedef struct control_config_t
{
    control_mode_t control_mode;
    orientation_t manual_orientation;
} control_config_t;

typedef struct system_state_t
{
    quaternion_t platform_rotation;
    orientation_t panel_orientation;
    orientation_t motors_rotation;
} system_state_t;

static const float azimuth_angular_speed = 180.f;
static const float inclination_angular_speed = 180.f;
static const float latitude = 10.75f;
static const float longitude = 106.75f;

static control_config_t control_config = {
    .control_mode = MANUAL,
};
static system_state_t system_state = {
    .platform_rotation = QUATERNION_IDENTITY,
};
static bool time_updated;

static void initialize_sntp();
static void initialize_timezone();
static void notify_sntp_sync(struct timeval *tv);
static void cloud_client_data_handler(const char *data, int length);
static void rotate_motors(void *params);
static void update_platform_rotation(TimerHandle_t timer);
static void upload_system_state(TimerHandle_t timer);
static orientation_t compensate_platform_rotation(orientation_t orientation);
static void rotate_step(orientation_t *current_orientation, const orientation_t desired_orientation, const float delta_time);
static double gettimeofday_combined();

void app_main(void)
{
    initialize_timezone();

    ESP_ERROR_CHECK(nvs_flash_init());

    bool is_connected = false;
    initialise_wifi(&is_connected);

    motors_init(GPIO_NUM_13, GPIO_NUM_4,
                GPIO_NUM_16, GPIO_NUM_17);
    sensor_init();

    TimerHandle_t update_platform_rotation_handle = xTimerCreate("Update platform rotation", pdMS_TO_TICKS(80), pdTRUE, NULL, update_platform_rotation);
    xTimerStart(update_platform_rotation_handle, 0);
    ESP_LOGI("Platform rotation", "Timer started.");

    ESP_LOGI("Wifi", "Waiting...");
    while (!is_connected)
        vTaskDelay(pdMS_TO_TICKS(100));
    ESP_LOGI("Wifi", "Started.");

    initialize_sntp();

    cloud_client_init(cloud_client_data_handler);

    TaskHandle_t motors_task;
    xTaskCreate(rotate_motors, "Motors", 8196, NULL, tskIDLE_PRIORITY, &motors_task);
    ESP_LOGI("Motors", "Task created.");

    TimerHandle_t upload_system_state_handle = xTimerCreate("Upload system state", pdMS_TO_TICKS(200), pdTRUE, NULL, upload_system_state);
    xTimerStart(upload_system_state_handle, pdMS_TO_TICKS(1000));
    ESP_LOGI("System state", "Will be uploaded in 1 second.");

    ESP_LOGI("Solar tracker", "Initialized.");
}

static void initialize_sntp()
{
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_sync_interval(15000);
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
    sntp_set_time_sync_notification_cb(notify_sntp_sync);
    sntp_init();
}

static void initialize_timezone()
{
    setenv("TZ", "GMT+7", 1);
    tzset();
}

static void notify_sntp_sync(struct timeval *tv)
{
    time_updated = true;
    ESP_LOGI("SNTP", "Received time from server: %ld", tv->tv_sec);
}

static void cloud_client_data_handler(const char *data, int length)
{
    cJSON *root = cJSON_Parse(data);
    cJSON *event = cJSON_GetObjectItem(root, "event");

    if (strcmp(event->valuestring, "UPDATE_CONFIG") == 0)
    {
        cJSON *payload = cJSON_GetObjectItem(root, "payload");

        cJSON *control_mode = cJSON_GetObjectItem(payload, "controlMode");
        control_config.control_mode = strcmp(control_mode->valuestring, "MANUAL") == 0 ? MANUAL : AUTOMATIC;

        cJSON *manual_orientation = cJSON_GetObjectItem(payload, "manualOrientation");
        control_config.manual_orientation.azimuth = cJSON_GetObjectItem(manual_orientation, "azimuth")->valuedouble;
        control_config.manual_orientation.inclination = cJSON_GetObjectItem(manual_orientation, "inclination")->valuedouble;
    }
    else
    {
        ESP_LOGW("Cloud client", "Event not supported: %s", event->valuestring);
    }

    cJSON_Delete(root);
}

bool is_in_dead_zone(orientation_t orientation)
{
    return (-5.f < orientation.azimuth && orientation.azimuth < 5.f) ||
           (175.f < orientation.azimuth && orientation.azimuth < 185.f) ||
           (355.f < orientation.azimuth && orientation.azimuth < 365.f);
}

static void rotate_motors(void *params)
{
    ESP_LOGI("Motors", "Started.");

    for (;;)
    {
        static double last_time = 0.f;
        double current_time = gettimeofday_combined();
        float delta_time = current_time - last_time;
        last_time = current_time;

        if (!time_updated)
            continue;

        if (control_config.control_mode == AUTOMATIC)
        {
            time_t t;
            time(&t);
            system_state.panel_orientation = get_sun_orientation(t, latitude, longitude);
        }
        else
        {
            system_state.panel_orientation = control_config.manual_orientation;
        }

        orientation_t desired_motors_rotation = compensate_platform_rotation(system_state.panel_orientation);
        // orientation_t desired_motors_rotation = system_state.panel_orientation;

        if (desired_motors_rotation.azimuth > 180.f)
        {
            desired_motors_rotation.azimuth -= 180.f;
            desired_motors_rotation.inclination = -desired_motors_rotation.inclination;
        }

        desired_motors_rotation.inclination = fmin(fmax(desired_motors_rotation.inclination, -90.f), 90.f);

        if (is_in_dead_zone(desired_motors_rotation) && is_in_dead_zone(system_state.motors_rotation))
            continue;

        rotate_step(&system_state.motors_rotation, desired_motors_rotation, delta_time);
        motors_rotate(system_state.motors_rotation);
    }
}

static void update_platform_rotation(TimerHandle_t timer)
{
    if (!time_updated)
        return;

    vector3_t accel = {0.f, 0.f, 1.f};
    vector3_t gyro = {0.f, 0.f, 0.f};
    vector3_t magnet = {1.f, 0.f, 0.f};

    sensor_read(&accel, &gyro, &magnet);

    gyro.x *= rad;
    gyro.y *= rad;
    gyro.z *= rad;

    static double last_time = 0.f;
    double current_time = gettimeofday_combined();
    float delta_time = last_time == 0.f ? 0.f : current_time - last_time;
    last_time = current_time;

    static vector3_t error;
    quaternion_mahony_update(&system_state.platform_rotation, &error, accel, gyro, magnet, delta_time / 4.f);
}

static void upload_system_state(TimerHandle_t timer)
{
    char buffer[512];
    int length = sprintf(
        buffer,
        "{"
        "\"event\":\"UPDATE_STATE\","
        "\"payload\":{"
        "\"platformRotation\":{\"w\":%.6f,\"x\":%.6f,\"y\":%.6f,\"z\":%.6f},"
        "\"panelOrientation\":{\"azimuth\":%.6f,\"inclination\":%.6f},"
        "\"motorsRotation\":{\"azimuth\":%.6f,\"inclination\":%.6f},"
        "\"timestamp\":%lld"
        "}}",
        system_state.platform_rotation.w, system_state.platform_rotation.x, system_state.platform_rotation.y, system_state.platform_rotation.z,
        system_state.panel_orientation.azimuth, system_state.panel_orientation.inclination,
        system_state.motors_rotation.azimuth, system_state.motors_rotation.inclination,
        (long long)(gettimeofday_combined() * 1000.L));

    if (length <= 0)
    {
        ESP_LOGE("Solar tracker", "Cannot format system state data for upload.");
        return;
    }

    cloud_client_send(buffer, length, portMAX_DELAY);
}

static orientation_t compensate_platform_rotation(orientation_t orientation)
{
    vector3_t world_direction = quaternion_rotate(
        quaternion_from_orientation(orientation),
        VECTOR3_UP);
    vector3_t local_direction = quaternion_rotate(
        quaternion_inverse(system_state.platform_rotation),
        world_direction);

    local_direction = vector3_normalize(local_direction);

    // Nearly align with the platform normal
    if (fabs(local_direction.x) < .1f && fabs(local_direction.y) < .1f)
    {
        return (orientation_t){
            .azimuth = system_state.motors_rotation.azimuth,
            .inclination = 0.f,
        };
    }

    return (orientation_t){
        .azimuth = -atan2(local_direction.y, local_direction.x) / rad + (local_direction.y >= 0.f ? 360.f : 0.f),
        .inclination = acos(local_direction.z) / rad,
    };
}

static void rotate_step(orientation_t *current_orientation, const orientation_t desired_orientation, const float delta_time)
{
    float azimuth_offset = desired_orientation.azimuth - current_orientation->azimuth;
    float inclination_offset = desired_orientation.inclination - current_orientation->inclination;
    float delta_azimuth = fmin(fabs(azimuth_offset), azimuth_angular_speed * delta_time);
    float delta_inclination = fmin(fabs(inclination_offset), inclination_angular_speed * delta_time);
    current_orientation->azimuth += copysign(delta_azimuth, azimuth_offset);
    current_orientation->inclination += copysign(delta_inclination, inclination_offset);
}

static double gettimeofday_combined()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec * 1e-6;
}
