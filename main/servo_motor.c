static void mcpwm_example_gpio_initialize(void)
{
    printf("initializing mcpwm servo control gpio......\n");
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, GPIO_SERVO_0_PWM);    //Set GPIO 18 as PWM0A, to which servo is connected
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, GPIO_SERVO_1_PWM); 
}

/**
 * @brief Use this function to calcute pulse width for per degree rotation
 *
 * @param  degree_of_rotation the angle in degree to which servo has to rotate
 *
 * @return
 *     - calculated pulse width
 */
static uint32_t servo_per_degree_init(uint32_t degree_of_rotation)
{
    uint32_t cal_pulsewidth = 0;
    cal_pulsewidth = (SERVO_MIN_PULSEWIDTH + (((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * (degree_of_rotation)) / (SERVO_MAX_DEGREE)));
    return cal_pulsewidth;
}

/**
 * @brief Configure MCPWM module
 */


// void app_main(void)
// {
//     init_system();
//     static uint32_t counter = 0;
//     angle_current_xoy = 0;
//     angle_xoy = 180;
//     angle_current_yoz = 0;
//     angle_yoz = 180;
//     while (1) {
//         counter = (counter + 1)% 6000;

//         if(counter == 0)
//         {
//             angle_xoy = (angle_xoy + 100 ) % 180;
//         }
//         if(counter == 0)
//         {
//             angle_yoz= (angle_yoz + 100 ) % 180;
//         }

//         servo_motor_process_xoy();
//         servo_motor_process_yoz();


//         vTaskDelay(5);  







//     //     for (count = 0; count < SERVO_MAX_DEGREE; count++) {
//     //         printf("Angle of rotation: %d\n", count);
//     //         angle = servo_per_degree_init(count);
//     //         printf("pulse width: %dus\n", angle);
//     //         mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, angle);
//     // //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation at 5V
//     //         // mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 1000);
//     //         vTaskDelay(10);  
//     //     }
//     }
// }


void servo_motor_process_xoy(void)
{
    //static uint8_t cnt = 0;
    static uint8_t status_motor = 0;
    switch (status_motor)
    {
    case SERVO_IDLE:
        motor_0_off();
        if(angle_current_xoy != angle_xoy)
        {
            status_motor = SERVO_RUNNING;
            motor_0_on();
        }
        break;
    case SERVO_RUNNING:
        if(angle_xoy - angle_current_xoy >= 1)
        {
            pulse_xoy = servo_per_degree_init(angle_current_xoy + 1);
            angle_current_xoy = angle_current_xoy + 1;
            status_motor = SERVO_WAIT;
        }
        else 
            status_motor = SERVO_IDLE;

        if(angle_current_xoy - angle_xoy >= 1)
        {
            pulse_xoy = servo_per_degree_init(angle_current_xoy - 1);
            angle_current_xoy = angle_current_xoy - 1;
            status_motor = SERVO_WAIT;
        }
        else 
            status_motor = SERVO_IDLE;

        mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, pulse_xoy);
        break;
    case SERVO_WAIT:
        status_motor = SERVO_RUNNING;
        break;
    default:
        status_motor = SERVO_IDLE;
        break;
    }


    printf("Angle of rotatio: %d\n", angle_current_xoy);          
    printf("pulse width: %dus\n", pulse_xoy);
}


void servo_motor_process_yoz(void)
{
    //static uint8_t cnt = 0;
    static uint8_t status_motor = 0;
    switch (status_motor)
    {
    case SERVO_IDLE:
        motor_1_off();
        if(angle_current_yoz != angle_yoz)
        {
            status_motor = SERVO_RUNNING;
            motor_1_on();
        }
        break;
    case SERVO_RUNNING:
        if(angle_yoz - angle_current_yoz >= 1)
        {
            pulse_yoz = servo_per_degree_init(angle_current_yoz + 1);
            angle_current_yoz = angle_current_yoz + 1;
            status_motor = SERVO_WAIT;
        }
        else 
            status_motor = SERVO_IDLE;

        if(angle_current_yoz - angle_yoz >= 1)
        {
            pulse_yoz = servo_per_degree_init(angle_current_yoz - 1);
            angle_current_yoz = angle_current_yoz - 1;
            status_motor = SERVO_WAIT;
        }
        else 
            status_motor = SERVO_IDLE;

        mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, pulse_yoz);
        break;
    case SERVO_WAIT:
        status_motor = SERVO_RUNNING;
        break;
    default:
        status_motor = SERVO_IDLE;
        break;
    }


    printf("Angle of rotation yoz: %d\n", angle_current_yoz);          
    printf("pulse width yoz: %dus\n", pulse_yoz);
}



void init_system(void)
{
    printf("Testing servo motor.......\n");
    mcpwm_example_gpio_initialize();
    init_PWM();
    init_gpio_motor();


}

void init_PWM(void)
{
    //2. initial mcpwm configuration
    printf("Configuring Initial Parameters of mcpwm......\n");
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 50;    //frequency = 50Hz, i.e. for every servo motor time period should be 20ms
    pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0;    //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings
}
void init_gpio_motor(void)
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 1;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    motor_0_off();
    motor_1_off();
}

void motor_0_on(void)
{
    gpio_set_level(GPIO_MOTOR_IO_0, 1);
}

void motor_0_off(void)
{
    gpio_set_level(GPIO_MOTOR_IO_0, 0);
}

void motor_1_on(void)
{
    gpio_set_level(GPIO_MOTOR_IO_1, 1);
}

void motor_1_off(void)
{
    gpio_set_level(GPIO_MOTOR_IO_1, 0);
}
