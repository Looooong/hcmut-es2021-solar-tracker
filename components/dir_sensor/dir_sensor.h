#ifndef _DIR_SENSOR_H
#define _DIR_SENSOR_H
#include "../../main/types.h"
#include "driver/gpio.h"


// do calibration() before init_i2c_dir_sensor() and copy cali config to cal value in dir_sensor.c
// then run program again without calibration()
void calibration(); 
void init_i2c_dir_sensor(gpio_num_t SCL_GPIO, gpio_num_t SDA_GPIO);
void get_vector3(vector3_t *v_accel,vector3_t *v_gyro,vector3_t *v_mag);

#endif


