#ifndef __RTC__
#define __RTC__

#include "ds3231.h"
#include "stdlib.h"
#include "esp_sntp.h"
#include "time.h"
#include "i2cdev.h"

i2c_dev_t dev;

void init_rtc();
void get_time_rtc(struct tm *time);

#endif
