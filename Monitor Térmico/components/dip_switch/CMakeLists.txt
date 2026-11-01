idf_component_register(SRCS "dip_switch.c"
                    INCLUDE_DIRS "."
                    PRIV_INCLUDE_DIRS "."
                    REQUIRES esp_driver_gpio app_config)