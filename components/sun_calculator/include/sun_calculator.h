#ifndef __SUN_CALCULATOR__
#define __SUN_CALCULATOR__

// #include <types.h>
#include "orientation.h"
#include "math.h"
#include "time.h"
#include "ds1307.h"
#include "esp_log.h"

// for ds1307
#define CONFIG_SCL_GPIO		15
#define CONFIG_SDA_GPIO		16

#define PI 3.14159265358979323846
#define rad PI/180
//date/time constants and conversions
#define daySeconds  60 * 60 * 24
#define J1970   2440588
#define J2000   2451545
//general calculations for position
#define e  rad * 23.4397 // obliquity of the Earth
// struct for coords

typedef struct coords{
    float dec;
    float ra;
}coords_t;


i2c_dev_t init_ds1307();
time_t get_time_ds1307();
orientation_t get_sun_orientation(time_t time, float latitude, float longitude);
float toDays(time_t date);
float toJulian (time_t date);
coords_t sunCoords(float d);
float solarMeanAnomaly(float d);
float eclipticLongitude(float M);
float declination(float l ,float b);
float rightAscension(float l ,float b);
float siderealTime(float d, float lw);
float azimuth(float H, float phi,float c_dec);
float altitude(float H, float phi ,float c_dec);
orientation_t getPosition(time_t date , float lat , float lng);
#endif
