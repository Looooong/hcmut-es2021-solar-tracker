idf_component_register(SRCS "rtc.c"
                    INCLUDE_DIRS "include"
                    REQUIRES ds3231  wifi_connector)
