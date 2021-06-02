#ifndef __MOTORS_CONTROLLER_H__
#define __MOTORS_CONTROLLER_H__

#include <driver/gpio.h>
#include "orientation.h"

void motors_init(
    gpio_num_t azimuth_enable_gpio, gpio_num_t inclination_enable_gpio,
    gpio_num_t azimuth_pwm_gpio, gpio_num_t inclination_pwm_gpio);
void motors_rotate(orientation_t orientation);

#endif // __MOTORS_CONTROLLER_H__
