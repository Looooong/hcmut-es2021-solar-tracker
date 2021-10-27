#include <driver/ledc.h>
#include <esp_log.h>
#include <math.h>
#include "motors_controller.h"

static const float azimuth_0_degrees_duty_cycle = .02f;
static const float azimuth_360_degrees_duty_cycle = .12f;
static const float inclination_0_degrees_duty_cycle = .02f;
static const float inclination_180_degrees_duty_cycle = .12f;

static orientation_t current_orientation = {
    .azimuth = 1000.f,
    .inclination = 1000.f,
};

static unsigned int target_duty_azimuth(float azimuth);
static unsigned int target_duty_inclination(float inclination);

void motors_init(
    gpio_num_t azimuth_enable_gpio, gpio_num_t inclination_enable_gpio,
    gpio_num_t azimuth_pwm_gpio, gpio_num_t inclination_pwm_gpio)
{
    ESP_LOGI("Motors", "Initializing...");
    gpio_config_t enable_gpio_config = {
        .pin_bit_mask = BIT(azimuth_enable_gpio) | BIT(inclination_enable_gpio),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 50,
    };
    ledc_channel_config_t azimuth_channel_config = {
        .gpio_num = azimuth_pwm_gpio,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .hpoint = 0,
    };
    ledc_channel_config_t inclination_channel_config = {
        .gpio_num = inclination_pwm_gpio,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LEDC_CHANNEL_1,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .hpoint = 0,
    };

    ESP_ERROR_CHECK(gpio_config(&enable_gpio_config));
    ESP_ERROR_CHECK(gpio_set_level(azimuth_enable_gpio, 1));
    ESP_ERROR_CHECK(gpio_set_level(inclination_enable_gpio, 1));
    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));
    ESP_ERROR_CHECK(ledc_channel_config(&azimuth_channel_config));
    ESP_ERROR_CHECK(ledc_channel_config(&inclination_channel_config));
    ESP_LOGI("Motors", "Initialized.");
}

void motors_rotate(orientation_t orientation)
{
    bool is_dirty = false;

    if (fabs(orientation.azimuth - current_orientation.azimuth) > .01f)
    {
        ledc_set_duty(
            LEDC_HIGH_SPEED_MODE,
            LEDC_CHANNEL_0,
            target_duty_azimuth(orientation.azimuth));
        ledc_update_duty(
            LEDC_HIGH_SPEED_MODE,
            LEDC_CHANNEL_0);

        is_dirty = true;
    }

    if (fabs(orientation.inclination - current_orientation.inclination) > .01f)
    {
        ledc_set_duty(
            LEDC_HIGH_SPEED_MODE,
            LEDC_CHANNEL_1,
            target_duty_inclination(orientation.inclination));
        ledc_update_duty(
            LEDC_HIGH_SPEED_MODE,
            LEDC_CHANNEL_1);

        is_dirty = true;
    }

    if (is_dirty)
    {
        current_orientation = orientation;
        // ESP_LOGI("Motors", "New orientation: (%6.3f, %6.3f)", orientation.azimuth, orientation.inclination);
    }
}

static unsigned int target_duty_azimuth(float azimuth)
{
    return 1024.f * (azimuth_360_degrees_duty_cycle - (azimuth / 180.f) * (azimuth_360_degrees_duty_cycle - azimuth_0_degrees_duty_cycle));
}

static unsigned int target_duty_inclination(float inclination)
{
    return 1024.f * (inclination_0_degrees_duty_cycle + (inclination / 180.f + .5f) * (inclination_180_degrees_duty_cycle - inclination_0_degrees_duty_cycle));
}
