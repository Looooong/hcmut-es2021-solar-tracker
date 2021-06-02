#include <driver/ledc.h>
#include <esp_log.h>
#include "motors_controller.h"

const float azimuth_0_degrees_duty_cycle = .05f;
const float azimuth_360_degrees_duty_cycle = .1f;
const float inclination_0_degrees_duty_cycle = .05f;
const float inclination_180_degrees_duty_cycle = .1f;

orientation_t current_orientation;

static unsigned int target_duty_azimuth(float azimuth);
static unsigned int target_duty_inclination(float inclination);

void motors_init(int azimuth_gpio, int inclination_gpio)
{
    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 50,
    };
    ledc_channel_config_t azimuth_channel_config = {
        .gpio_num = azimuth_gpio,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .hpoint = 0,
    };
    ledc_channel_config_t inclination_channel_config = {
        .gpio_num = inclination_gpio,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LEDC_CHANNEL_1,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .hpoint = 0,
    };

    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));
    ESP_ERROR_CHECK(ledc_channel_config(&azimuth_channel_config));
    ESP_ERROR_CHECK(ledc_channel_config(&inclination_channel_config));
    ESP_LOGI("Motors", "Initialized.");
}

void motors_rotate(orientation_t orientation)
{
    if (orientation.azimuth != current_orientation.azimuth)
    {
        ledc_set_duty(
            LEDC_HIGH_SPEED_MODE,
            LEDC_CHANNEL_0,
            target_duty_azimuth(orientation.azimuth));
        ledc_update_duty(
            LEDC_HIGH_SPEED_MODE,
            LEDC_CHANNEL_0);

        current_orientation.azimuth = orientation.azimuth;
        ESP_LOGI("Motors", "New azimuth: %f", current_orientation.azimuth);
    }

    if (orientation.inclination != current_orientation.inclination)
    {
        ledc_set_duty(
            LEDC_HIGH_SPEED_MODE,
            LEDC_CHANNEL_1,
            target_duty_inclination(orientation.inclination));
        ledc_update_duty(
            LEDC_HIGH_SPEED_MODE,
            LEDC_CHANNEL_1);

        current_orientation.inclination = orientation.inclination;
        ESP_LOGI("Motors", "New inclination: %f", current_orientation.inclination);
    }
}

static unsigned int target_duty_azimuth(float azimuth)
{
    return 1024.f * (azimuth_0_degrees_duty_cycle + (azimuth / 360.f) * (azimuth_360_degrees_duty_cycle - azimuth_0_degrees_duty_cycle));
}

static unsigned int target_duty_inclination(float inclination)
{
    return 1024.f * (inclination_0_degrees_duty_cycle + (inclination / 180.f) * (inclination_180_degrees_duty_cycle - inclination_0_degrees_duty_cycle));
}
