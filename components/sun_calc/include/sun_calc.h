#ifndef __SUN_CALC_H__
#define __SUN_CALC_H__

typedef struct
{
    double azimuth;
    double altitude;
} sun_coords_t;

sun_coords_t sunCalcGetPosition(time_t date, double lat, double lng);

#endif // __SUN_CALC_H__
