idf_component_register(
    SRCS "led_rgb.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES esp_driver_gpio esp_driver_ledc estadisticas app_config
)