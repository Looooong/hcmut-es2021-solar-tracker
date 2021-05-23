#include <ds1307.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_websocket_client.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include "cloud_client.h"
#include "sun_calculator.h"
#include "wifi_connector.h"
#include "driver/adc.h"

i2c_dev_t init_ds1307();
time_t get_time_ds1307(i2c_dev_t *dev);
static void cloud_client_data_handler(const char *data, int length);

void app_main(void)
{
    ESP_ERROR_CHECK( nvs_flash_init() );
    initialise_wifi();
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
    ESP_LOGI("Cloud Client", "Received: %.*s", length, data);
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
    return mktime(&time);
}
void get_adc_value( ){
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_DB_0);
    int val = adc1_get_raw(ADC1_CHANNEL_0);
}