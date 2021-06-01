idf_component_register(
    SRCS "cloud_client.c" "sun_calculator.c" "main.c"
    INCLUDE_DIRS ""
    REQUIRES esp_websocket_client json rtc sun_calc wifi_connector
)
