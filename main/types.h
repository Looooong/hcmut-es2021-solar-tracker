#ifndef __TYPES_H__
#define __TYPES_H__

#define QUATERNION_IDENTITY \
    (quaternion_t) { 1.f, 0.f, 0.f, 0.f }
#define VECTOR3_UP \
    (vector3_t) { 0.f, 0.f, 1.f }

typedef struct orientation_t
{
    float azimuth;
    float inclination;
} orientation_t;

typedef struct quaternion_t
{
    float w;
    float x;
    float y;
    float z;
} quaternion_t;

typedef struct vector3_t
{
    float x;
    float y;
    float z;
} vector3_t;

quaternion_t quaternion_from_orientation(orientation_t o);
quaternion_t quaternion_inverse(quaternion_t q);
quaternion_t quaternion_multiply(quaternion_t q1, quaternion_t q2);
vector3_t quaternion_rotate(quaternion_t q, vector3_t v);
void quaternion_mahony_update(quaternion_t *q, vector3_t *error, vector3_t accel, vector3_t gyro, vector3_t magnet, float dt);

float vector3_dot(vector3_t a, vector3_t b);
float vector3_magnitude(vector3_t v);
vector3_t vector3_normalize(vector3_t v);

#endif // __TYPES_H__
