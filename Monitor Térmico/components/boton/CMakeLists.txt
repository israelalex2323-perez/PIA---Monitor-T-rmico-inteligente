
idf_component_register(
    SRCS "boton.c"
    INCLUDE_DIRS "."
    REQUIRES esp_driver_gpio app_config estadisticas potenciometro
)
