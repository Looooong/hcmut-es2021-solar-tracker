#include <cJSON.h>
#include <ds1307.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_websocket_client.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include "cloud_client.h"
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
}
config_t;

config_t config;

i2c_dev_t init_ds1307();
time_t get_time_ds1307(i2c_dev_t *dev);
static void cloud_client_data_handler(const char *data, int length);

void app_main(void)
{
    i2c_dev_t dev = init_ds1307();
    cloud_client_init(cloud_client_data_handler);

    while (1)
    {
        time_t time = get_time_ds1307(&dev);
        orientation_t orient = get_sun_orientation(time, 10.816572, 106.674488);
        printf("Time by Seconds : %ld\n", time);
        printf("azimuth = %f \ninclination = %f\n", orient.azimuth, orient.inclination);
        fflush(stdout);
        cloud_client_send("Hello from client", 18, 10000);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
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
        config.control_mode = control_mode->valueint > 0 ? MANUAL : AUTOMATIC;

        cJSON *manual_orientation = cJSON_GetObjectItem(payload, "manualOrientation");
        config.manual_orientation.azimuth = cJSON_GetObjectItem(manual_orientation, "azimuth")->valuedouble;
        config.manual_orientation.inclination = cJSON_GetObjectItem(manual_orientation, "inclination")->valuedouble;
    }

    cJSON_Delete(root);
    free(json_data);
}

i2c_dev_t init_ds1307()
{
    i2c_dev_t dev;
    while (ds1307_init_desc(&dev, I2C_NUM_0, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO) != ESP_OK)
    {
        ESP_LOGE(pcTaskGetTaskName(0), "Could not init device descriptor.");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    return dev;
}

time_t get_time_ds1307(i2c_dev_t *dev)
{
    struct tm time;

    while (ds1307_get_time(dev, &time) != ESP_OK)
    {
        ESP_LOGE(pcTaskGetTaskName(0), "Could not get time.");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    time.tm_year = time.tm_year - 1900;
    time.tm_hour = time.tm_hour - TIME_ZONE_DS1307;
    // printf( "time.tm_sec=%d",time.tm_sec);
    // printf( "time.tm_min=%d",time.tm_min);
    // printf( "time.tm_hour=%d",time.tm_hour);
    // printf( "time.tm_wday=%d",time.tm_wday);
    // printf( "time.tm_mday=%d",time.tm_mday);
    // printf( "time.tm_mon=%d",time.tm_mon);
    // printf( "time.tm_year=%d",time.tm_year);
    return mktime(&time);
}
