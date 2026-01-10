idf_component_register(SRCS "obd.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES esp_driver_gpio mcp2515)

