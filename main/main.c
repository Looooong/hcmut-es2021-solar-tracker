#include <cJSON.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_sntp.h>
#include <esp_system.h>
#include <esp_wifi.h>
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

static const float angular_speed = 30.f;
static const float latitude = 10.75f;
static const float longitude = 106.75f;

static control_config_t *control_config;
static orientation_t current_orientation;
static quaternion_t platform_rotation = QUATERNION_IDENTITY;
static vector3_t platform_rotation_error;
static double platform_rotation_updated_at;

static void initialize_sntp();
static void notify_sntp_sync(struct timeval *tv);
static void cloud_client_data_handler(const char *data, int length);
static void motors_timer_callback(TimerHandle_t _timer);
static void update_platform_rotation(TimerHandle_t _timer);
static orientation_t compensate_platform_rotation(orientation_t orientation);
static float delta_rotation(const float current, const float target, const float delta_time);

void app_main(void)
{
    setenv("TZ", "GMT+7", 1);
    tzset();

    control_config = (control_config_t *)malloc(sizeof(control_config_t));

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
        control_config->control_mode = strcmp(control_mode->valuestring, "MANUAL") == 0 ? MANUAL : AUTOMATIC;

        cJSON *manual_orientation = cJSON_GetObjectItem(payload, "manualOrientation");
        control_config->manual_orientation.azimuth = cJSON_GetObjectItem(manual_orientation, "azimuth")->valuedouble;
        control_config->manual_orientation.inclination = cJSON_GetObjectItem(manual_orientation, "inclination")->valuedouble;
    }

    cJSON_Delete(root);
}

static void motors_timer_callback(TimerHandle_t _timer)
{
    orientation_t motors_orientation = control_config->manual_orientation;

    if (control_config->control_mode == AUTOMATIC)
    {
        time_t t;
        time(&t);
        motors_orientation = get_sun_orientation(t, latitude, longitude);
    }

    motors_orientation = compensate_platform_rotation(motors_orientation);

    current_orientation.azimuth += delta_rotation(current_orientation.azimuth, motors_orientation.azimuth, .1f);
    current_orientation.inclination += delta_rotation(current_orientation.inclination, motors_orientation.inclination, .1f);

    motors_rotate(current_orientation);
}

static void update_platform_rotation(TimerHandle_t _timer)
{
    // TODO: Read sensor data
    vector3_t accel = {0.f, 0.f, 1.f};
    vector3_t gyro = {0.f, 0.f, 0.f};
    vector3_t magnet = {1.f, 0.f, 0.f};

    struct timeval tv;
    gettimeofday(&tv, NULL);
    double current_time = tv.tv_sec + tv.tv_usec * 1e-6;
    float dt = platform_rotation_updated_at == 0.f ? 0.f : current_time - platform_rotation_updated_at;

    quaternion_mahony_update(&platform_rotation, &platform_rotation_error, accel, gyro, magnet, dt);

    platform_rotation_updated_at = current_time;
}

static orientation_t compensate_platform_rotation(orientation_t orientation)
{
    vector3_t world_direction = quaternion_rotate(
        quaternion_from_orientation(orientation),
        VECTOR3_UP);
    vector3_t local_direction = quaternion_rotate(
        quaternion_inverse(platform_rotation),
        world_direction);

    local_direction = vector3_normalize(local_direction);

    // Nearly align with the platform normal
    if (fabs(local_direction.x) < .01f && fabs(local_direction.y) < .01f)
    {
        return (orientation_t){
            .azimuth = current_orientation.azimuth,
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
