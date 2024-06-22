idf_component_register(SRCS "main.cpp" "led_strip_encoder.c" "ble_ans_init.c"
                       INCLUDE_DIRS "."
                       REQUIRES bt
                       REQUIRES nvs_flash
                       REQUIRES driver)
