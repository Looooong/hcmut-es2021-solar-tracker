#include <esp_log.h>
#include <math.h>
#include "sensor.h"

calibration_t calibration = {
    // Used to get uncalibrated data
    // .accel_offset = {.x = 0.0, .y = 0.0, .z = 0.0},
    // .accel_scale_lo = {.x = -1.0, .y = -1.0, .z = -1.0},
    // .accel_scale_hi = {.x = 1.0, .y = 1.0, .z = 1.0},
    // .gyro_bias_offset = {.x = 0.0, .y = 0.0, .z = 0.0},
    // .mag_offset = {.x = 0.0, .y = 0.0, .z = 0.0},
    // .mag_scale = {.x = 1.0, .y = 1.0, .z = 1.0},

    .accel_offset = {.x = 0.58f, .y = -0.33f, .z = 0.4f},
    .accel_scale_lo = {.x = -1.0f, .y = -1.0f, .z = -1.0f},
    .accel_scale_hi = {.x = 1.0f, .y = 1.0f, .z = 1.0f},
    .gyro_bias_offset = {.x = 0.162521f, .y = -1.242311f, .z = -4.505748f},
    .mag_offset = {.x = 30.0f, .y = 90.0f, .z = -65.0f},
    .mag_scale = {.x = 0.015385f, .y = 0.0166667f, .z = -0.015385f},
};

void sensor_init()
{
    ESP_LOGI("Sensor", "Initializing...");
    ESP_ERROR_CHECK(i2c_mpu9250_init(&calibration));
    ESP_LOGI("Sensor", "Initialized.");
}

void sensor_read(vector3_t *accel, vector3_t *gyro, vector3_t *magnet)
{
    vector_t va, vg, vm;
    ESP_ERROR_CHECK(get_accel_gyro_mag(&va, &vg, &vm));
    // ESP_LOGI(
    //     "Sensor", "Accel: (%6.2f, %6.2f, %6.2f). Gyro: (%6.2f, %6.2f, %6.2f). Magnet: (%6.2f, %6.2f, %6.2f)",
    //     va.x, va.y, va.z,
    //     vg.x, vg.y, vg.z,
    //     vm.x, vm.y, vm.z);

    accel->x = va.x;
    accel->y = va.y;
    accel->z = va.z;
    // Convert from degrees to radians
    gyro->x = vg.x * M_PI / 180.0f;
    gyro->y = vg.y * M_PI / 180.0f;
    gyro->z = vg.z * M_PI / 180.0f;
    // Swap X and Y components because of alignment issue
    magnet->x = vm.y;
    magnet->y = vm.x;
    magnet->z = vm.z;
}
