#include <esp_log.h>
#include <math.h>
#include "sensor.h"

bool initialized = false;
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
    .mag_offset = {.x = 50.0f, .y = 30.0f, .z = -35.0f},
    .mag_scale = {.x = 0.015385f, .y = 0.014286f, .z = -0.015385f},
};

void sensor_init()
{
    ESP_LOGI("Sensor", "Initializing...");

    if (i2c_mpu9250_init(&calibration) == ESP_OK) {
        initialized = true;
        ESP_LOGI("Sensor", "Initialized.");
    } else {
        ESP_LOGE("Sensor", "Error initializing.");
    }
}

void sensor_read(vector3_t *accel, vector3_t *gyro, vector3_t *magnet)
{
    if (!initialized) return;

    vector_t va, vg, vm;
    
    if (get_accel_gyro_mag(&va, &vg, &vm) != ESP_OK) return;

    // ESP_LOGI(
    //     "Sensor", "Accel: (%6.2f, %6.2f, %6.2f). Gyro: (%6.2f, %6.2f, %6.2f). Magnet: (%6.2f, %6.2f, %6.2f)",
    //     va.x, va.y, va.z,
    //     vg.x, vg.y, vg.z,
    //     vm.x, vm.y, vm.z);

    accel->x = va.x;
    accel->y = va.y;
    accel->z = va.z;
    gyro->x = vg.x;
    gyro->y = vg.y;
    gyro->z = vg.z;
    // Swap X and Y components because of alignment issue
    magnet->x = vm.y;
    magnet->y = vm.x;
    magnet->z = vm.z;
}
