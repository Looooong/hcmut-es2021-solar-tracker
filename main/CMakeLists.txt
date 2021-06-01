idf_component_register(
    SRCS "motors_controller.c" "servo_motor.c" "cloud_client.c" "sun_calculator.c" "main.c"
    INCLUDE_DIRS ""
    REQUIRES esp_websocket_client json rtc sun_calc wifi_connector
)
