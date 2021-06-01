#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "protocol_examples_common.h"
#include "esp_sntp.h"
#include "stdbool.h"

#include "rtc.h"
#include "wifi_connector.h"

#define NTP_SERVER  "pool.ntp.org"
#define TAG "rtc"
#define SDA_GPIO    21
#define SCL_GPIO    22
void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

static void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    //sntp_setservername(0, "pool.ntp.org");
    ESP_LOGI(TAG, "Your NTP Server is %s", NTP_SERVER);
    sntp_setservername(0, NTP_SERVER);
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_init();
}

static bool obtain_time(void)
{
    // tcpip_adapter_init();
    // ESP_ERROR_CHECK( esp_event_loop_create_default() );

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    bool is_connected = false;
    initialise_wifi(&is_connected);
    while(!is_connected){
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    initialize_sntp();
    // wait for time to be set
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    // ESP_ERROR_CHECK(example_connect());
    if (retry == retry_count) return false;
    return true;
}
void init_rtc(){
    ESP_LOGI(pcTaskGetTaskName(0), "Connecting to WiFi and getting time over NTP.");
    if(!obtain_time()) {
        ESP_LOGE(pcTaskGetTaskName(0), "Fail to getting time over NTP.");
        while (1) { vTaskDelay(1); }
    }

    // update 'now' variable with current time
    time_t now;
    struct tm timeinfo;
    char strftime_buf[64];
    time(&now);
    now = now + (CONFIG_TIMEZONE*60*60);
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(pcTaskGetTaskName(0), "The current date/time is: %s", strftime_buf);
    memset(&dev, 0, sizeof(i2c_dev_t));
    ESP_ERROR_CHECK(ds3231_init_desc(&dev, 0, SDA_GPIO, SCL_GPIO));
     struct tm time = {
        .tm_year = timeinfo.tm_year + 1900,
        .tm_mon  = timeinfo.tm_mon,  // 0-based
        .tm_mday = timeinfo.tm_mday,
        .tm_hour = timeinfo.tm_hour,
        .tm_min  = timeinfo.tm_min,
        .tm_sec  = timeinfo.tm_sec
    };

    if (ds3231_set_time(&dev, &time) != ESP_OK) {
        ESP_LOGE(pcTaskGetTaskName(0), "Could not set time.");
        while (1) { vTaskDelay(1); }
    }
    ESP_LOGI(pcTaskGetTaskName(0), "Set initial date time done");

    // goto deep sleep
    const int deep_sleep_sec = 10;
    ESP_LOGI(pcTaskGetTaskName(0), "Entering deep sleep for %d seconds", deep_sleep_sec);
    esp_deep_sleep(1000000LL * deep_sleep_sec);

}
void get_time_rtc(struct tm *time){
    ds3231_get_time(&dev, time);
}