idf_component_register( SRCS "dir_sensor.c"
                        INCLUDE_DIRS "."
                        REQUIRES mpu9250 ahrs main
                        )