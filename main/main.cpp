#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <vector>
#include <cmath>
#include <thread>

#include "nvs_init.h"
#include "bluetooth.h"
#include "led_driver.h"
#include "led_pixel.h"

static const char *TAG = "main_user";

static const gpio_num_t RGB_LED_GPIO_NUM = GPIO_NUM_48;
static const int32_t PIXEL_COUNT = 1;

typedef ws2812::Driver<RGB_LED_GPIO_NUM> LEDDriver;

extern "C" void app_main()
{
    nvs::init_nvs();
    bluetooth::init_ble();

    ESP_LOGI(TAG, "Initializing LED Driver");
    LEDDriver ledDriver{};

    ESP_LOGI(TAG, "Displaying test pixel");
    ws2812::Pixel red{255, 0, 0};
    ledDriver.transmitData(&red, sizeof(red));
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    ws2812::Pixel green{0, 255, 0};
    ledDriver.transmitData(&green, sizeof(green));
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    ws2812::Pixel blue{0, 0, 255};
    ledDriver.transmitData(&blue, sizeof(blue));
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    while (true)
    {
        TickType_t time = xTaskGetTickCount();
        double hue = (std::sin(time / 200.0) * 180) + 180;
        ws2812::Pixel pixel = ws2812::Pixel::pixelFromHsv(hue, 100, 100);

        std::vector<ws2812::Pixel> data{PIXEL_COUNT, pixel};

        ledDriver.transmitData(data.data(), sizeof(pixel) * data.size());
        ledDriver.joinAll();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}
