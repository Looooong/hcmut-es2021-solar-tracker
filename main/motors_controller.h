#ifndef MOTORS_CONTROLLER_H
#define MOTORS_CONTROLLER_H

#include "orientation.h"

void motors_init(int azimuth_gpio, int inclination_gpio);
void motors_rotate(orientation_t orientation);

#endif // MOTORS_CONTROLLER_H
