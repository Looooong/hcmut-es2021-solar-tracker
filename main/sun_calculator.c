#include <time.h>
#include <sun_calc.h>
#include "sun_calculator.h"

orientation_t get_sun_orientation(time_t time, float latitude, float longitude)
{
    sun_coords_t sun_coords = sunCalcGetPosition(time, latitude, longitude);
    orientation_t orientation;
    orientation.azimuth = sun_coords.azimuth;
    orientation.inclination = 90.0 - sun_coords.altitude;
    return orientation;
}
