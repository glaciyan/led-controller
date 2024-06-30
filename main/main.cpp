#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <vector>
#include <cmath>
#include <thread>

#include "nvs_init.h"
#include "led/led_driver.h"
#include "led/led_pixel.h"

#include "ble/ble.h"
#include "ble_config.h"

const char *TAG = "main_user";

// LED
constexpr gpio_num_t RGB_LED_GPIO_NUM = GPIO_NUM_20;
constexpr int32_t PIXEL_COUNT = 1;

#define LED_CONTROLLER_LED_PWR 1
constexpr gpio_num_t LED_CONTROLLER_LED_PWR_PIN = GPIO_NUM_19;

typedef ws2812::Driver<RGB_LED_GPIO_NUM> LEDDriver;

// Main
extern "C" [[noreturn]] void app_main()
{
    ESP_LOGI(TAG, "Initializing LED Driver");
    LEDDriver ledDriver{};

    nvs::init_nvs();
    ble::init_bluetooth(gatt_services);

#if LED_CONTROLLER_LED_PWR
    ESP_LOGI(TAG, "Configuring GPIO Pin");
    gpio_config_t io_conf{};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << LED_CONTROLLER_LED_PWR_PIN);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_LOGI(TAG, "Setting GPIO to high");
    ESP_ERROR_CHECK(gpio_set_level(LED_CONTROLLER_LED_PWR_PIN, 1));
#endif

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

    ws2812::Pixel white{255, 255, 255};
    ledDriver.transmitData(&white, sizeof(white));
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    while (true)
    {
        TickType_t time = xTaskGetTickCount();
        double hue = (std::sin(time / 200.0) * 180) + 180;
        ws2812::Pixel pixel = ws2812::Pixel::pixelFromHsv(hue, 100, 100);

        std::vector<ws2812::Pixel> data{PIXEL_COUNT, pixel};

        ledDriver.transmitData(data.data(), sizeof(pixel) * data.size());
        ledDriver.joinAll();
        if (notifyReady && notify_attr_handle == color_characteristic_attr_handle)
        {
            gatt_color += 1;
            ble_gatts_notify(notify_conn_handle, notify_attr_handle);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}
