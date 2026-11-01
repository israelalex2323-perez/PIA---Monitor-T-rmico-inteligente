idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES app_config sensor_bmp280 pantalla_oled boton potenciometro dip_switch led_rgb buzzer estadisticas)