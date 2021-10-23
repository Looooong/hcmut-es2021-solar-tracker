#ifndef SENSOR_H
#define SENSOR_H

#include <mpu9250.h>
#include "types.h"

void sensor_init();
void sensor_read(vector3_t* accel, vector3_t *gyro, vector3_t *magnet);

#endif // SENSOR_H
