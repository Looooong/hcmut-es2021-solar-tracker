#include <time.h>
#include <sun_calc.h>
#include "sun_calculator.h"

#define PI 3.14159265358979323846
#define rad (PI / 180)

orientation_t get_sun_orientation(time_t time, float latitude, float longitude)
{
    sun_coords_t sun_coords = sunCalcGetPosition(time, latitude, longitude);
    orientation_t orientation;
    orientation.azimuth = sun_coords.azimuth / rad;
    orientation.inclination = 90.f - sun_coords.altitude / rad;
    return orientation;
}
