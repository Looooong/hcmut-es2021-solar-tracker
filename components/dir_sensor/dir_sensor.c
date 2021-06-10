#include "dir_sensor.h"
#include "MadgwickAHRS.h"
#include "mpu9250.h"
#include "calibrate.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_task_wdt.h"
#include "common.h"

#define SAMPLE_FREQ_Hz 200
static const char *TAG = "get_vector";
#define I2C_MASTER_NUM I2C_NUM_0

calibration_t cal = {
    .mag_offset = {.x = 49.167969, .y = 77.349609, .z = -50.875000},
    .mag_scale = {.x = 0.934578, .y = 0.927854, .z = 1.173374},
    
    .accel_offset = {.x = -1.149350, .y = 0.650544, .z = -0.795528},
    .accel_scale_lo = {.x = 0.424583, .y = 1.329111, .z = 0.625761},
    .accel_scale_hi = {.x = -1.578237, .y = -0.676312, .z = -1.391017},

    .gyro_bias_offset = {.x = 0.162139, .y = -1.131729, .z = -4.508235}};

void transfer_to_vector3(vector3_t *v1,vector_t *v2){
    v1->x = -v2->x;
    v1->y = -v2->y;
    v1->z = -v2->z;
}
void init_i2c_dir_sensor(gpio_num_t SCL_GPIO, gpio_num_t SDA_GPIO){
    choose_SCL_SDA_GPIO(SCL_GPIO,SDA_GPIO);
    i2c_mpu9250_init(&cal);
    MadgwickAHRSinit(SAMPLE_FREQ_Hz, 0.8);
}

void get_vector3(vector3_t *v_accel,vector3_t *v_gyro,vector3_t *v_mag){
    vector_t va, vg, vm;

    // uint64_t i = 0;

    
    // Get the Accelerometer, Gyroscope and Magnetometer values.
    ESP_ERROR_CHECK(get_accel_gyro_mag(&va, &vg, &vm));

    // Apply the AHRS algorithm
    MadgwickAHRSupdate(DEG2RAD(vg.x), DEG2RAD(vg.y), DEG2RAD(vg.z),
                        va.x, va.y, va.z,
                        vm.x, vm.y, vm.z);
    //ESP_LOGI(TAG, "Accel->x: %2.3f, ->y: %2.3f, ->z: %2.3f ", va.x, va.y, va.z);
    transfer_to_vector3(v_accel, &va);
    transfer_to_vector3(v_gyro, &vg);
    transfer_to_vector3(v_mag, &vm);
    esp_task_wdt_reset();
}

void calibration(){
    calibrate_gyro();
    calibrate_accel();
    calibrate_mag();
}


