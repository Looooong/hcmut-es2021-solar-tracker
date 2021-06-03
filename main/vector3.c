#include <math.h>
#include "types.h"

float vector3_dot(vector3_t a, vector3_t b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float vector3_magnitude(vector3_t v)
{
    return sqrt(vector3_dot(v, v));
}

vector3_t vector3_normalize(vector3_t v)
{
    float magnitude = vector3_magnitude(v);

    if (fabs(magnitude) < .000001f)
        return v;

    return (vector3_t){
        v.x / magnitude,
        v.y / magnitude,
        v.z / magnitude,
    };
}
