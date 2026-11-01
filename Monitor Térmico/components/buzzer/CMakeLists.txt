idf_component_register(
    SRCS "buzzer.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES esp_driver_gpio esp_driver_ledc estadisticas app_config
)