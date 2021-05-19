#ifndef CLOUD_CLIENT_H
#define CLOUD_CLIENT_H

#include <esp_websocket_client.h>
#include <freertos/FreeRTOS.h>

typedef void (*cloud_client_data_handler_t)(const char *data, int length);

void cloud_client_init(cloud_client_data_handler_t data_handler);
void cloud_client_send(const char *data, int length, TickType_t timeout);

#endif // CLOUD_CLIENT_H
