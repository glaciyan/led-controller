#pragma once

#include "esp_log.h"
#include "esp_err.h"

#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "services/ans/ble_svc_ans.h"

namespace bluetooth
{
    static const char *TAG = "bluetooth";

    constexpr void init_ble()
    {
        ESP_LOGI(TAG, "Initializing NimBLE");
    }
}