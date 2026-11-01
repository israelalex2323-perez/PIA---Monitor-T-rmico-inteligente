idf_component_register(
    SRCS "estadisticas.c"
    INCLUDE_DIRS "."
    REQUIRES sensor_bmp280 freertos
)