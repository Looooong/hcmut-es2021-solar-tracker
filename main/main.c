// #include <cJSON.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_websocket_client.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include "driver/adc.h"

#include "cloud_client.h"
#include "sun_calculator.h"
#include "wifi_connector.h"
#include "rtc.h"


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

// static void cloud_client_data_handler(const char *data, int length);
void app_main(void)
{
    ESP_ERROR_CHECK( nvs_flash_init() );
    // initialise_wifi(&is_connected);
    init_rtc();
    // cloud_client_init(cloud_client_data_handler);

    while(true)
    {
        struct tm time_tm;
        time_t time ;
        get_time_rtc(&time_tm);
        time = mktime(&time_tm);
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

void get_adc_value( ){
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_DB_0);
    int val = adc1_get_raw(ADC1_CHANNEL_0);
}