#include <math.h>
#include "types.h"

#define PI 3.14159265358979323846
#define rad (PI / 180.f)

quaternion_t quaternion_from_orientation(orientation_t o)
{
    float yaw = -o.azimuth * rad;
    float pitch = o.inclination * rad;
    float roll = 0.f;

    // Abbreviations for the various angular functions
    float cy = cos(yaw * .5f);
    float sy = sin(yaw * .5f);
    float cp = cos(pitch * .5f);
    float sp = sin(pitch * .5f);
    float cr = cos(roll * .5f);
    float sr = sin(roll * .5f);

    return (quaternion_t){
        .w = cr * cp * cy + sr * sp * sy,
        .x = sr * cp * cy - cr * sp * sy,
        .y = cr * sp * cy + sr * cp * sy,
        .z = cr * cp * sy - sr * sp * cy,
    };
}

quaternion_t quaternion_inverse(quaternion_t q)
{
    return (quaternion_t){q.w, -q.x, -q.y, -q.z};
}

quaternion_t quaternion_multiply(quaternion_t q1, quaternion_t q2)
{
    return (quaternion_t){
        .w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z,
        .x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
        .y = q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x,
        .z = q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w,
    };
}

vector3_t quaternion_rotate(quaternion_t q, vector3_t v)
{
    quaternion_t qv = {0.f, v.x, v.y, v.z};

    qv = quaternion_multiply(
        quaternion_multiply(q, qv),
        quaternion_inverse(q));

    return (vector3_t){qv.x, qv.y, qv.z};
}

// https://github.com/sparkfun/SparkFun_MPU-9250_Breakout_Arduino_Library/blob/master/src/quaternionFilters.cpp

// These are the free parameters in the Mahony filter and fusion scheme, Kp
// for proportional feedback, Ki for integral
#define Kp 2.0f * 5.0f
#define Ki 0.0f

// Similar to Madgwick scheme but uses proportional and integral filtering on
// the error between estimated reference vectors and measured ones.
void quaternion_mahony_update(quaternion_t *q, vector3_t *error, vector3_t accel, vector3_t gyro, vector3_t magnet, float dt)
{
    // short name local variable for readability
    float q1 = q->w, q2 = q->x, q3 = q->y, q4 = q->z;
    float ax = accel.x, ay = accel.y, az = accel.z;
    float gx = gyro.x, gy = gyro.y, gz = gyro.z;
    float mx = magnet.x, my = magnet.y, mz = magnet.z;
    float norm;
    float hx, hy, bx, bz;
    float vx, vy, vz, wx, wy, wz;
    float ex, ey, ez;
    float pa, pb, pc;

    // Auxiliary variables to avoid repeated arithmetic
    float q1q1 = q1 * q1;
    float q1q2 = q1 * q2;
    float q1q3 = q1 * q3;
    float q1q4 = q1 * q4;
    float q2q2 = q2 * q2;
    float q2q3 = q2 * q3;
    float q2q4 = q2 * q4;
    float q3q3 = q3 * q3;
    float q3q4 = q3 * q4;
    float q4q4 = q4 * q4;

    // Normalise accelerometer measurement
    norm = sqrt(ax * ax + ay * ay + az * az);
    if (norm == 0.0f)
        return;         // Handle NaN
    norm = 1.0f / norm; // Use reciprocal for division
    ax *= norm;
    ay *= norm;
    az *= norm;

    // Normalise magnetometer measurement
    norm = sqrt(mx * mx + my * my + mz * mz);
    if (norm == 0.0f)
        return;         // Handle NaN
    norm = 1.0f / norm; // Use reciprocal for division
    mx *= norm;
    my *= norm;
    mz *= norm;

    // Reference direction of Earth's magnetic field
    hx = 2.0f * mx * (0.5f - q3q3 - q4q4) + 2.0f * my * (q2q3 - q1q4) + 2.0f * mz * (q2q4 + q1q3);
    hy = 2.0f * mx * (q2q3 + q1q4) + 2.0f * my * (0.5f - q2q2 - q4q4) + 2.0f * mz * (q3q4 - q1q2);
    bx = sqrt((hx * hx) + (hy * hy));
    bz = 2.0f * mx * (q2q4 - q1q3) + 2.0f * my * (q3q4 + q1q2) + 2.0f * mz * (0.5f - q2q2 - q3q3);

    // Estimated direction of gravity and magnetic field
    vx = 2.0f * (q2q4 - q1q3);
    vy = 2.0f * (q1q2 + q3q4);
    vz = q1q1 - q2q2 - q3q3 + q4q4;
    wx = 2.0f * bx * (0.5f - q3q3 - q4q4) + 2.0f * bz * (q2q4 - q1q3);
    wy = 2.0f * bx * (q2q3 - q1q4) + 2.0f * bz * (q1q2 + q3q4);
    wz = 2.0f * bx * (q1q3 + q2q4) + 2.0f * bz * (0.5f - q2q2 - q3q3);

    // Error is cross product between estimated direction and measured direction of gravity
    ex = (ay * vz - az * vy) + (my * wz - mz * wy);
    ey = (az * vx - ax * vz) + (mz * wx - mx * wz);
    ez = (ax * vy - ay * vx) + (mx * wy - my * wx);

    // Apply feedback terms
    gx = gx + Kp * ex;
    gy = gy + Kp * ey;
    gz = gz + Kp * ez;
    if (error != NULL)
    {
        // Accumulate integral error
        error->x += ex;
        error->y += ey;
        error->z += ez;

        // Integral feedback
        gx += Ki * error->x;
        gy += Ki * error->y;
        gz += Ki * error->z;
    }

    // Integrate rate of change of quaternion
    pa = q2;
    pb = q3;
    pc = q4;
    q1 = q1 + (-q2 * gx - q3 * gy - q4 * gz) * (0.5f * dt);
    q2 = pa + (q1 * gx + pb * gz - pc * gy) * (0.5f * dt);
    q3 = pb + (q1 * gy - pa * gz + pc * gx) * (0.5f * dt);
    q4 = pc + (q1 * gz + pa * gy - pb * gx) * (0.5f * dt);

    // Normalise quaternion
    norm = sqrt(q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4);
    norm = 1.0f / norm;
    q->w = q1 * norm;
    q->x = q2 * norm;
    q->y = q3 * norm;
    q->z = q4 * norm;
}
