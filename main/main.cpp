/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <array>
#include <cmath>

#include "led_driver.h"
#include "led_pixel.h"

#define RMT_LED_STRIP_GPIO_NUM (static_cast<gpio_num_t>(48))

#define EXAMPLE_CHASE_SPEED_MS (10)

typedef led::Driver<RMT_LED_STRIP_GPIO_NUM> LEDDriver;

extern "C" void app_main()
{
    LEDDriver ledDriver{};

    led::Pixel red{255, 0, 0};
    auto redData = red.asGRB8Bit();
    ESP_ERROR_CHECK(ledDriver.transmitData(&redData, led::Pixel::size));
    ESP_ERROR_CHECK(ledDriver.joinAll());
    vTaskDelay(pdMS_TO_TICKS(500));

    led::Pixel green{0, 255, 0};
    auto greenData = green.asGRB8Bit();
    ESP_ERROR_CHECK(ledDriver.transmitData(&greenData, led::Pixel::size));
    ESP_ERROR_CHECK(ledDriver.joinAll());
    vTaskDelay(pdMS_TO_TICKS(500));

    led::Pixel blue{0, 0, 255};
    auto blueData = blue.asGRB8Bit();
    ESP_ERROR_CHECK(ledDriver.transmitData(&blueData, led::Pixel::size));
    ESP_ERROR_CHECK(ledDriver.joinAll());
    vTaskDelay(pdMS_TO_TICKS(500));

    while (true)
    {
        TickType_t time = xTaskGetTickCount();
        double hue = (std::sin(time / 200.0) * 180) + 180;
        led::Pixel pixel = led::Pixel::pixelFromHsv(hue, 100, 100);

        auto data = pixel.asGRB8Bit();
        ESP_ERROR_CHECK(ledDriver.transmitData(&data, sizeof(uint8_t) * data.size()));
        ESP_ERROR_CHECK(ledDriver.joinAll());
        vTaskDelay(pdMS_TO_TICKS(EXAMPLE_CHASE_SPEED_MS));
    }
}
