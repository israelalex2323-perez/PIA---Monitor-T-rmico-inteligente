
idf_component_register(
    SRCS "pantalla_oled.c"
    INCLUDE_DIRS "."
    REQUIRES esp_driver_i2c sensor_bmp280 estadisticas
)
