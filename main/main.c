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
    float solar_panel_current;
    float solar_panel_voltage;
    float battery_voltage;
    orientation_t solar_panel_orientation;
    orientation_t motors_rotation;
    quaternion_t platform_rotation;
} system_state_t;

static const float angular_speed = 30.f;
static const float latitude = 10.75f;
static const float longitude = 106.75f;

static control_config_t control_config;
static system_state_t system_state = {
    .platform_rotation = QUATERNION_IDENTITY,
};
static double platform_rotation_updated_at;

static void initialize_sntp();
static void initialize_timezone();
static void notify_sntp_sync(struct timeval *tv);
static void cloud_client_data_handler(const char *data, int length);
static void motors_timer_callback(TimerHandle_t timer);
static void update_platform_rotation(TimerHandle_t timer);
static void upload_system_state(TimerHandle_t timer);
static orientation_t compensate_platform_rotation(orientation_t orientation);
static float delta_rotation(const float current, const float target, const float delta_time);
static double gettimeofday_combined();

void app_main(void)
{
    initialize_timezone();

    ESP_ERROR_CHECK(nvs_flash_init());

    bool is_connected = false;
    initialise_wifi(&is_connected);

    while (!is_connected)
        vTaskDelay(pdMS_TO_TICKS(100));

    initialize_sntp();

    cloud_client_init(cloud_client_data_handler);
    motors_init(
        GPIO_NUM_13, GPIO_NUM_4,
        GPIO_NUM_17, GPIO_NUM_16);

    TimerHandle_t motors_timer = xTimerCreate("Motors", pdMS_TO_TICKS(100), pdTRUE, NULL, motors_timer_callback);
    xTimerStart(motors_timer, pdMS_TO_TICKS(1000));
    ESP_LOGI("Motors", "Will start in 1 second.");

    TimerHandle_t update_platform_rotation_handle = xTimerCreate("Update platform rotation", pdMS_TO_TICKS(40), pdTRUE, NULL, update_platform_rotation);
    xTimerStart(update_platform_rotation_handle, pdMS_TO_TICKS(1000));
    ESP_LOGI("Platform rotation", "Will be updated in 1 second.");

    TimerHandle_t upload_system_state_handle = xTimerCreate("Upload system state", pdMS_TO_TICKS(100), pdTRUE, NULL, upload_system_state);
    xTimerStart(upload_system_state_handle, pdMS_TO_TICKS(1050));
    ESP_LOGI("System state", "Will be uploaded in 1.05 second.");

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

static void motors_timer_callback(TimerHandle_t timer)
{
    if (control_config.control_mode == AUTOMATIC)
    {
        time_t t;
        time(&t);
        system_state.solar_panel_orientation = get_sun_orientation(t, latitude, longitude);
    }
    else
    {
        system_state.solar_panel_orientation = control_config.manual_orientation;
    }

    orientation_t desired_motors_rotation = compensate_platform_rotation(system_state.solar_panel_orientation);
    float dt = pdTICKS_TO_MS(xTimerGetPeriod(timer)) / 1000.f;

    system_state.motors_rotation.azimuth += delta_rotation(system_state.motors_rotation.azimuth, desired_motors_rotation.azimuth, dt);
    system_state.motors_rotation.inclination += delta_rotation(system_state.motors_rotation.inclination, desired_motors_rotation.inclination, dt);

    motors_rotate(system_state.motors_rotation);
}

static void update_platform_rotation(TimerHandle_t timer)
{
    // TODO: Read sensor data
    vector3_t accel = {0.f, 0.f, 1.f};
    vector3_t gyro = {0.f, 0.f, 0.f};
    vector3_t magnet = {1.f, 0.f, 0.f};

    double current_time = gettimeofday_combined();
    float dt = platform_rotation_updated_at == 0.f ? 0.f : current_time - platform_rotation_updated_at;

    quaternion_mahony_update(&system_state.platform_rotation, NULL, accel, gyro, magnet, dt);

    platform_rotation_updated_at = current_time;
}

static void upload_system_state(TimerHandle_t timer)
{
    char buffer[512];
    int length = sprintf(
        buffer,
        "{"
        "\"event\":\"UPDATE_STATE\","
        "\"payload\":{"
        "\"timestamp\":%lld,"
        "\"solarPanelCurrent\":%.6f,"
        "\"solarPanelVoltage\":%.6f,"
        "\"batteryVoltage\":%.6f,"
        "\"solarPanelOrientation\":{\"azimuth\":%.6f,\"inclination\":%.6f},"
        "\"motorsRotation\":{\"azimuth\":%.6f,\"inclination\":%.6f},"
        "\"platformRotation\":{\"w\":%.6f,\"x\":%.6f,\"y\":%.6f,\"z\":%.6f}"
        "}}",
        (long long)(gettimeofday_combined() * 1000.L),
        system_state.solar_panel_current,
        system_state.solar_panel_voltage,
        system_state.battery_voltage,
        system_state.solar_panel_orientation.azimuth, system_state.solar_panel_orientation.inclination,
        system_state.motors_rotation.azimuth, system_state.motors_rotation.inclination,
        system_state.platform_rotation.w, system_state.platform_rotation.x, system_state.platform_rotation.y, system_state.platform_rotation.z);

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
    if (fabs(local_direction.x) < .01f && fabs(local_direction.y) < .01f)
    {
        return (orientation_t){
            .azimuth = system_state.motors_rotation.azimuth,
            .inclination = 0.f,
        };
    }

    return (orientation_t){
        .azimuth = -atan2(local_direction.y, local_direction.x) / rad + (local_direction.y > 0.f ? 360.f : 0.f),
        .inclination = acos(local_direction.z) / rad,
    };
}

static float delta_rotation(const float current, const float target, const float delta_time)
{
    float delta = target - current;
    float absolute_rotation = fmin(fabs(delta), angular_speed * delta_time);
    return copysign(absolute_rotation, delta);
}

static double gettimeofday_combined()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}
