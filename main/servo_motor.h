/* servo motor control example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"
#include "driver/gpio.h"
//You can get these value from the datasheet of servo you use, in general pulse width varies between 1000 to 2000 mocrosecond
#define SERVO_MIN_PULSEWIDTH 500 //Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH 2600 //Maximum pulse width in microsecond
#define SERVO_MAX_DEGREE 180 //Maximum angle in degree upto which servo can rotate
#define GPIO_SERVO_0_PWM    16
#define GPIO_SERVO_1_PWM    17
#define GPIO_MOTOR_IO_0    4
#define GPIO_MOTOR_IO_1    13
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_MOTOR_IO_0) | (1ULL<<GPIO_MOTOR_IO_1))

#define SERVO_IDLE    0
#define SERVO_RUNNING    1
#define SERVO_WAIT    2

void init_gpio_motor(void);
void motor_0_on(void);
void motor_0_off(void);
void motor_1_on(void);
void motor_1_off(void);


//0x la huong bac, oy la huong dong, oz la vecto phap truyen voi trai dat
static uint32_t servo_per_degree_init(uint32_t degree_of_rotation);

void init_system(void);
void init_PWM(void);
void servo_motor_process_xoy(void);
void servo_motor_process_yoz(void);
extern int32_t angle_yoz, pulse_yoz;//goc tam pin voi phap tuyen trai dat
extern int32_t angle_current_yoz = 0;
extern int32_t angle_xoy, pulse_xoy;//goc huong cua tam pin voi huong bac
extern int32_t angle_current_xoy = 0;
