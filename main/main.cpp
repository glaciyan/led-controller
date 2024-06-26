#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <vector>
#include <array>
#include <cmath>
#include <thread>

#include "nvs_init.h"
#include "ble/ble.h"
#include "ble_uuids.h"
#include "led/led_driver.h"
#include "led/led_pixel.h"

const char *TAG = "main_user";

// LED
constexpr gpio_num_t RGB_LED_GPIO_NUM = GPIO_NUM_20;
constexpr int32_t PIXEL_COUNT = 1;

#define LED_CONTROLLER_LED_PWR 1
constexpr gpio_num_t LED_CONTROLLER_LED_PWR_PIN = GPIO_NUM_19;

typedef ws2812::Driver<RGB_LED_GPIO_NUM> LEDDriver;

// Bluetooth
uint16_t color_characteristic_attr_handle;
int color_characteristic_access(uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt *ctxt, void *arg)
{
    switch (ctxt->op)
    {
    case BLE_GATT_ACCESS_OP_READ_CHR:
        if (attr_handle == color_characteristic_attr_handle)
        {
            // rc = os_mbuf_append(ctxt->om, &gatt_svr_chr_val, sizeof(gatt_svr_chr_val));
            // return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        break;

    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        if (attr_handle == color_characteristic_attr_handle)
        {
        //     rc = gatt_svr_write(ctxt->om, sizeof(gatt_svr_chr_val), sizeof(gatt_svr_chr_val),
        //                         &gatt_svr_chr_val, nullptr);
        //     ble_gatts_chr_updated(attr_handle);
        //     MODLOG_DFLT(INFO, "Notification/Indication scheduled for all subscribed peers.\n");
        //     return rc;
        }
        break;

    default:
        break;
    }

    assert(0);
    return BLE_ATT_ERR_UNLIKELY;
}

constexpr ble_gatt_svc_def gatt_services[] = SERVICE_LIST(
    SERVICE(
        UUID(color_service_uuid),
        CHARACTERISTIC(
            UUID(rgb_characteristic_uuid),
            color_characteristic_access,
            ble::perm::EREAD | ble::perm::EWRITE,
            &color_characteristic_attr_handle)));

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
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}
