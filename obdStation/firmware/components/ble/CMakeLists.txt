idf_component_register(SRCS "ble.c"
                    INCLUDE_DIRS "include"
                    REQUIRES bt driver esp_event nvs_flash)

