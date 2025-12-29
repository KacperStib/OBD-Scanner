set(srcs "main.c")

idf_component_register(SRCS ${srcs}
                       PRIV_REQUIRES esp_driver_spi esp_driver_gpio mcp2515 obd ble
                       INCLUDE_DIRS ".")