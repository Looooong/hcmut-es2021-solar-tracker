#include "sun_calculator.h"

float toDays(time_t date){
    return toJulian(date) -J2000;
}

float toJulian(time_t date){
    return (float)(date)/(float)(daySeconds) -0.5 + J1970;
}

coords_t sunCoords(float d ){
    float M = solarMeanAnomaly(d);
    float L = eclipticLongitude(M);
    coords_t coords;
    coords.dec = declination(L, 0);
    coords.ra  = rightAscension(L, 0);
    return coords;
}
float solarMeanAnomaly(float d){
    return rad *( 357.5291 + 0.98560028 * d );
}

float eclipticLongitude(float M){
    float C =  rad * (1.9148 * sin(M) + 0.02 * sin(2 * M) + 0.0003 * sin(3 * M));
    float P = rad * 102.9372;
    return M + C + P + PI;
}

float declination(float l , float b){
    return asin(sin(b) * cos(e) + cos(b) * sin(e) * sin(l));
}

float rightAscension(float l ,float b){
    return atan2(sin(l) * cos(e) - tan(b) * sin(e), cos(l)); 
}

float siderealTime(float d, float lw){
    return rad * (280.16 + 360.9856235 * d) - lw;
}

float azimuth(float H, float phi,float dec){
    return atan2(sin(H), cos(H) * sin(phi) - tan(dec) * cos(phi));
}

float altitude(float H, float phi ,float dec){
    return asin(sin(phi) * sin(dec) + cos(phi) * cos(dec) * cos(H));
} 

orientation_t getPosition(time_t date , float lat , float lng){
    float lw = rad * lng *(-1);
    float phi = rad * lat;
    float d   = toDays(date);
    coords_t c  = sunCoords(d);
    float H  = siderealTime(d, lw) - c.ra;
    orientation_t sun_angle;
    sun_angle.azimuth = azimuth(H, phi, c.dec);
    sun_angle.altitude = altitude(H, phi, c.dec);
    return sun_angle;
}