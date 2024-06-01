/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include <vector>
#include <cmath>

#include "led_driver.h"
#include "led_pixel.h"

#define RMT_LED_STRIP_GPIO_NUM (GPIO_NUM_2)
// #define LED_SWITCH_GPIO_NUM (GPIO_NUM_19)

#define EXAMPLE_CHASE_SPEED_MS (10)

// static const char *TAG = "main_user";

typedef ws2812::Driver<RMT_LED_STRIP_GPIO_NUM> LEDDriver;

extern "C" void app_main()
{
    LEDDriver ledDriver{};

    ws2812::Pixel red{255, 0, 0};
    ESP_ERROR_CHECK(ledDriver.transmitData(&red, sizeof(red)));
    vTaskDelay(pdMS_TO_TICKS(500));

    ws2812::Pixel green{0, 255, 0};
    ESP_ERROR_CHECK(ledDriver.transmitData(&green, sizeof(green)));
    vTaskDelay(pdMS_TO_TICKS(500));

    ws2812::Pixel blue{0, 0, 255};
    ESP_ERROR_CHECK(ledDriver.transmitData(&blue, sizeof(blue)));
    vTaskDelay(pdMS_TO_TICKS(500));

    while (true)
    {
        TickType_t time = xTaskGetTickCount();
        double hue = (std::sin(time / 200.0) * 180) + 180;
        ws2812::Pixel pixel = ws2812::Pixel::pixelFromHsv(hue, 100, 100);

        const int32_t pixelCount = 37;
        std::vector<ws2812::Pixel> data{pixelCount, pixel};

        ESP_ERROR_CHECK(ledDriver.transmitData(data.data(), sizeof(pixel) * data.size()));
        ESP_ERROR_CHECK(ledDriver.joinAll());
        vTaskDelay(pdMS_TO_TICKS(EXAMPLE_CHASE_SPEED_MS));
    }
}
