#include <math.h>
#include <time.h>
#include "sun_calc.h"

#define PI 3.14159265358979323846
#define rad (PI / 180)
//date/time constants and conversions
#define daySeconds (60 * 60 * 24)
#define J1970 2440588
#define J2000 2451545
//general calculations for position
#define e (rad * 23.4397) // obliquity of the Earth
// struct for coords

typedef struct
{
    double dec;
    double ra;
} celestial_coords_t;

static double toDays(time_t date);
double toJulian(time_t date);
celestial_coords_t sunCoords(double d);
double solarMeanAnomaly(double d);
double eclipticLongitude(double M);
double declination(double l, double b);
double rightAscension(double l, double b);
double siderealTime(double d, double lw);
double azimuth(double H, double phi, double c_dec);
double altitude(double H, double phi, double c_dec);

double toDays(time_t date)
{
    return toJulian(date) - J2000;
}

double toJulian(time_t date)
{
    return (double)(date) / (double)(daySeconds)-0.5 + J1970;
}

celestial_coords_t sunCoords(double d)
{
    double M = solarMeanAnomaly(d);
    double L = eclipticLongitude(M);
    celestial_coords_t coords;
    coords.dec = declination(L, 0);
    coords.ra = rightAscension(L, 0);
    return coords;
}

double solarMeanAnomaly(double d)
{
    return rad * (357.5291 + 0.98560028 * d);
}

double eclipticLongitude(double M)
{
    double C = rad * (1.9148 * sin(M) + 0.02 * sin(2 * M) + 0.0003 * sin(3 * M));
    double P = rad * 102.9372;
    return M + C + P + PI;
}

double declination(double l, double b)
{
    return asin(sin(b) * cos(e) + cos(b) * sin(e) * sin(l));
}

double rightAscension(double l, double b)
{
    return atan2(sin(l) * cos(e) - tan(b) * sin(e), cos(l));
}

double siderealTime(double d, double lw)
{
    return rad * (280.16 + 360.9856235 * d) - lw;
}

double azimuth(double H, double phi, double dec)
{
    return PI + atan2(sin(H), cos(H) * sin(phi) - tan(dec) * cos(phi));
}

double altitude(double H, double phi, double dec)
{
    return asin(sin(phi) * sin(dec) + cos(phi) * cos(dec) * cos(H));
}

sun_coords_t sunCalcGetPosition(time_t date, double lat, double lng)
{
    double lw = rad * -lng;
    double phi = rad * lat;
    double d = toDays(date);
    celestial_coords_t c = sunCoords(d);
    double H = siderealTime(d, lw) - c.ra;
    sun_coords_t sun_angle;
    sun_angle.azimuth = azimuth(H, phi, c.dec);
    sun_angle.altitude = altitude(H, phi, c.dec);
    return sun_angle;
}
