#include <cJSON.h>
#include <esp_log.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include <math.h>
#include "cloud_client.h"
#include "motors_controller.h"
#include "sun_calculator.h"

typedef enum control_mode_t
{
    AUTOMATIC,
    MANUAL
} control_mode_t;

typedef struct config_t
{
    control_mode_t control_mode;
    orientation_t manual_orientation;
} config_t;

const float angular_speed = 30.f;

config_t config;
orientation_t current_orientation;

static void cloud_client_data_handler(const char *data, int length);
static void motors_timer_callback(TimerHandle_t _timer);
static orientation_t compensate_platform_orientation(orientation_t orientation);
static float delta_rotation(const float current, const float target, const float delta_time);

void app_main(void)
{
    cloud_client_init(cloud_client_data_handler);
    motors_init(4, 13);

    xTimerCreate("Motors", pdMS_TO_TICKS(50), pdTRUE, NULL, motors_timer_callback);

    vTaskStartScheduler();
}

static void cloud_client_data_handler(const char *data, int length)
{
    ESP_LOGI("Cloud Client", "Received:\n%.*s", length, data);

    char *json_data = (char *)malloc(length + 1);

    strncpy(json_data, data, length);
    json_data[length] = '\0';

    cJSON *root = cJSON_Parse(json_data);
    cJSON *event = cJSON_GetObjectItem(root, "event");

    if (strcmp(event->valuestring, "UPDATE_CONFIG") == 0)
    {
        cJSON *payload = cJSON_GetObjectItem(root, "payload");

        cJSON *control_mode = cJSON_GetObjectItem(payload, "controlMode");
        config.control_mode = strcmp(control_mode->valuestring, "MANUAL") ? MANUAL : AUTOMATIC;

        cJSON *manual_orientation = cJSON_GetObjectItem(payload, "manualOrientation");
        config.manual_orientation.azimuth = cJSON_GetObjectItem(manual_orientation, "azimuth")->valuedouble;
        config.manual_orientation.inclination = cJSON_GetObjectItem(manual_orientation, "inclination")->valuedouble;
    }

    cJSON_Delete(root);
    free(json_data);
}

static void motors_timer_callback(TimerHandle_t _timer)
{
    orientation_t motors_orientation = config.manual_orientation;

    if (config.control_mode == AUTOMATIC)
    {
        // TODO: change parameters
        motors_orientation = get_sun_orientation(0, 0, 0);
    }

    motors_orientation = compensate_platform_orientation(motors_orientation);

    current_orientation.azimuth += delta_rotation(current_orientation.azimuth, motors_orientation.azimuth, .05f);
    current_orientation.inclination += delta_rotation(current_orientation.inclination, motors_orientation.inclination, .05f);

    motors_rotate(current_orientation);
}

static orientation_t compensate_platform_orientation(orientation_t orientation)
{
    // TODO: Implement
    return orientation;
}

static float delta_rotation(const float current, const float target, const float delta_time)
{
    float delta = target - current;
    float absolute_rotation = fmin(fabs(delta), angular_speed * delta_time);
    return copysign(delta, absolute_rotation);
}
