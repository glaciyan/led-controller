idf_component_register(SRCS "main.cpp" "led_strip_encoder.c" "ble_gatt_svr.c"
                       INCLUDE_DIRS "."
                       REQUIRES bt
                       REQUIRES nvs_flash
                       REQUIRES driver)
