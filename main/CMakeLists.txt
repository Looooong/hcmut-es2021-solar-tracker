idf_component_register(
    SRCS "sensor.c" "vector3.c" "quaternion.c" "motors_controller.c" "servo_motor.c" "cloud_client.c" "sun_calculator.c" "main.c"
    INCLUDE_DIRS ""
    REQUIRES esp_websocket_client json mpu9250 sun_calc wifi_connector
)
