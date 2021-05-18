#include <ds1307.h>
#include <esp_log.h>
#include <esp_spi_flash.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include "sun_calculator.h"

i2c_dev_t init_ds1307();
time_t get_time_ds1307(i2c_dev_t *dev);

void app_main(void)
{
    i2c_dev_t dev = init_ds1307();

    while (1)
    {
        time_t time = get_time_ds1307(&dev);
        orientation_t orient = get_sun_orientation(time, 10.816572, 106.674488);
        printf("Time by Seconds : %ld\n", time);
        printf("azimuth = %f \ninclination = %f\n", orient.azimuth, orient.inclination);
        fflush(stdout);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
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
