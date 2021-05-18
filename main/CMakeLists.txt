idf_component_register(
    SRCS "sun_calculator.c" "main.c"
    INCLUDE_DIRS ""
    REQUIRES sun_calc
    REQUIRES spi_flash
)
