#ifndef SUN_CALC_H
#define SUN_CALC_H

typedef struct
{
    float azimuth;
    float altitude;
} sun_coords_t;

sun_coords_t sunCalcGetPosition(time_t date, float lat, float lng);

#endif // SUN_CALC_H
