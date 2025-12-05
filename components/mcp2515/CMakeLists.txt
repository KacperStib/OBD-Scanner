idf_component_register(SRCS "mcp2515.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES esp_driver_gpio
                    REQUIRES esp_driver_spi)

