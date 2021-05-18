#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "sun_calculator.h"

void app_main(void)
{
    
    i2c_dev_t dev = init_ds1307();

    while (1){
        time_t time = get_time_ds1307(&dev);
        orientation_t orient = getPosition(time ,10.816572, 106.674488) ;
        printf("Time by Seconds : %ld\n", time );
        printf("azimuth = %f \naltitude = %f\n", orient.azimuth,orient.altitude);
        fflush(stdout);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
