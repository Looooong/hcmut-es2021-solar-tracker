#ifndef SUN_CALCULATOR_H
#define SUN_CALCULATOR_H

#include "orientation.h"

orientation_t get_sun_orientation(time_t time, float latitude, float longitude);

#endif // SUN_CALCULATOR_H
