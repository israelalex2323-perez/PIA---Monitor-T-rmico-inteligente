idf_component_register(
    SRCS "potenciometro.c"
    INCLUDE_DIRS "."
    REQUIRES app_config
    PRIV_REQUIRES esp_adc
)