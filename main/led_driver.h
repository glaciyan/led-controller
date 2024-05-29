#pragma once

#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/rmt_tx.h"

#include "led_strip_encoder.h"

namespace ws2812
{
    static const char *TAG = "led_driver";

    static const uint32_t RMT_LED_STRIP_RESOLUTION_HZ = 10'000'000;

    template<gpio_num_t gpioPin>
    struct Driver
    {
        // Create RMT TX channel
        rmt_channel_handle_t led_chan = nullptr;
        rmt_tx_channel_config_t tx_chan_config{};

        // Install led strip encoder
        rmt_encoder_handle_t led_encoder = nullptr;
        led_strip_encoder_config_t encoder_config{};

        // Transmission
        rmt_transmit_config_t tx_config{};

        constexpr Driver()
        {
            ESP_LOGI(TAG, "Create RMT TX channel");
            tx_chan_config.gpio_num = gpioPin;
            tx_chan_config.clk_src = RMT_CLK_SRC_DEFAULT;
            tx_chan_config.resolution_hz = RMT_LED_STRIP_RESOLUTION_HZ;
            tx_chan_config.mem_block_symbols = 64; // symbol size = 4
            tx_chan_config.trans_queue_depth = 4;  // set the number of transactions that can be pending in the background

            ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));

            encoder_config.resolution = RMT_LED_STRIP_RESOLUTION_HZ;
            ESP_ERROR_CHECK(rmt_new_led_strip_encoder(&encoder_config, &led_encoder));

            ESP_LOGI(TAG, "Enable RMT TX channel");
            ESP_ERROR_CHECK(rmt_enable(led_chan));

            tx_config.loop_count = 0;
        }

        constexpr esp_err_t transmitData(const void *data, const size_t dataSize) const
        {
            return rmt_transmit(led_chan, led_encoder, data, dataSize, &tx_config);
        }

        constexpr esp_err_t joinAll() const
        {
            return rmt_tx_wait_all_done(led_chan, portMAX_DELAY);
        }
    };
}