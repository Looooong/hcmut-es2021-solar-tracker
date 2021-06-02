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
#include "wifi_connector.h"

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

const float angular_speed = 30.f;
const float latitude = 10.75f;
const float longitude = 106.75f;

control_config_t *control_config;
orientation_t current_orientation;

static void initialize_sntp();
static void notify_sntp_sync(struct timeval *tv);
static void cloud_client_data_handler(const char *data, int length);
static void motors_timer_callback(TimerHandle_t _timer);
static orientation_t compensate_platform_orientation(orientation_t orientation);
static float delta_rotation(const float current, const float target, const float delta_time);

void app_main(void)
{
    control_config = (control_config_t *)malloc(sizeof(control_config_t));

    ESP_ERROR_CHECK(nvs_flash_init());

    bool is_connected = false;
    initialise_wifi(&is_connected);

    while (!is_connected)
        vTaskDelay(pdMS_TO_TICKS(100));

    initialize_sntp();

    cloud_client_init(cloud_client_data_handler);
    motors_init(17, 16);

    // xTimerCreate("Motors", pdMS_TO_TICKS(50), pdTRUE, NULL, motors_timer_callback);

    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = ((1ULL << 4) | (1ULL << 13));
    //disable pull-down mode
    io_conf.pull_down_en = 1;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    gpio_set_level(4, 1);
    gpio_set_level(13, 1);

    ESP_LOGI("Solar tracker", "Initialized.");
}

static void initialize_sntp()
{
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_sync_interval(15000);
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

        motors_rotate(control_config->manual_orientation);
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

    motors_orientation = compensate_platform_orientation(motors_orientation);

    current_orientation.azimuth += delta_rotation(current_orientation.azimuth, motors_orientation.azimuth, .05f);
    current_orientation.inclination += delta_rotation(current_orientation.inclination, motors_orientation.inclination, .05f);

    if (current_orientation.inclination > 90.f){
        ESP_LOGI("Motors", "Inclination over 90.");
        return;
    }

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
