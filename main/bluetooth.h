#pragma once

#include "esp_log.h"
#include "esp_err.h"

namespace bluetooth
{
    static const char *TAG = "bluetooth";

    constexpr void init_ble()
    {
        ESP_LOGI(TAG, "Initializing BLE");
    }
}