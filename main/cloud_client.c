#include <esp_log.h>
#include <esp_system.h>
#include "cloud_client.h"

const esp_websocket_client_config_t config = {
    .uri = "ws://localhost:8080/ws"};
esp_websocket_client_handle_t client;
cloud_client_data_handler_t external_data_handler;

static void cloud_client_data_handler_internal(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *websocket_event_data = (esp_websocket_event_data_t *)event_data;
    external_data_handler(websocket_event_data->data_ptr, websocket_event_data->data_len);
}

void cloud_client_init(cloud_client_data_handler_t data_handler)
{
    external_data_handler = data_handler;
    client = esp_websocket_client_init(&config);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_DATA, cloud_client_data_handler_internal, NULL);

    if (esp_websocket_client_start(client) != ESP_OK)
    {
        ESP_LOGE("Cloud Client", "Cannot connect start WebSocket client!");
    }
}

void cloud_client_send(const char *data, int length, TickType_t timeout)
{
    if (esp_websocket_client_send_text(client, data, length, timeout) != ESP_OK)
    {
        ESP_LOGE("Cloud Client", "Cannot send text data!");
    }
}
