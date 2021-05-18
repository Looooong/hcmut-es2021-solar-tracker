#include <math.h>
#include <time.h>
#include "sun_calc.h"

// for ds1307
#define CONFIG_SCL_GPIO 15
#define CONFIG_SDA_GPIO 16

#define PI 3.14159265358979323846
#define rad PI / 180
//date/time constants and conversions
#define daySeconds 60 * 60 * 24
#define J1970 2440588
#define J2000 2451545
//general calculations for position
#define e rad * 23.4397 // obliquity of the Earth
// struct for coords

typedef struct
{
    float dec;
    float ra;
} celestial_coords_t;

static float toDays(time_t date);
float toJulian(time_t date);
celestial_coords_t sunCoords(float d);
float solarMeanAnomaly(float d);
float eclipticLongitude(float M);
float declination(float l, float b);
float rightAscension(float l, float b);
float siderealTime(float d, float lw);
float azimuth(float H, float phi, float c_dec);
float altitude(float H, float phi, float c_dec);

float toDays(time_t date)
{
    return toJulian(date) - J2000;
}

float toJulian(time_t date)
{
    return (float)(date) / (float)(daySeconds)-0.5 + J1970;
}

celestial_coords_t sunCoords(float d)
{
    float M = solarMeanAnomaly(d);
    float L = eclipticLongitude(M);
    celestial_coords_t coords;
    coords.dec = declination(L, 0);
    coords.ra = rightAscension(L, 0);
    return coords;
}

float solarMeanAnomaly(float d)
{
    return rad * (357.5291 + 0.98560028 * d);
}

float eclipticLongitude(float M)
{
    float C = rad * (1.9148 * sin(M) + 0.02 * sin(2 * M) + 0.0003 * sin(3 * M));
    float P = rad * 102.9372;
    return M + C + P + PI;
}

float declination(float l, float b)
{
    return asin(sin(b) * cos(e) + cos(b) * sin(e) * sin(l));
}

float rightAscension(float l, float b)
{
    return atan2(sin(l) * cos(e) - tan(b) * sin(e), cos(l));
}

float siderealTime(float d, float lw)
{
    return rad * (280.16 + 360.9856235 * d) - lw;
}

float azimuth(float H, float phi, float dec)
{
    return atan2(sin(H), cos(H) * sin(phi) - tan(dec) * cos(phi));
}

float altitude(float H, float phi, float dec)
{
    return asin(sin(phi) * sin(dec) + cos(phi) * cos(dec) * cos(H));
}

sun_coords_t sunCalcGetPosition(time_t date, float lat, float lng)
{
    float lw = rad * lng * (-1);
    float phi = rad * lat;
    float d = toDays(date);
    celestial_coords_t c = sunCoords(d);
    float H = siderealTime(d, lw) - c.ra;
    sun_coords_t sun_angle;
    sun_angle.azimuth = azimuth(H, phi, c.dec);
    sun_angle.altitude = altitude(H, phi, c.dec);
    return sun_angle;
}
