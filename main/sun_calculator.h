#ifndef __SUN_CALCULATOR__
#define __SUN_CALCULATOR__

#include <types.h>
#include "orientation.h"

orientation_t get_sun_orientation(time_t time, float latitude, float longitude);

#endif
