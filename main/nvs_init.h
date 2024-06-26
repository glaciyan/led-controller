#pragma once

#include "nvs_flash.h"
#include "esp_err.h"
#include "esp_log.h"

namespace nvs
{
    constexpr const char *TAG = "nvs_init";

    void init_nvs()
    {
        ESP_LOGI(TAG, "Initializing NVS");

        esp_err_t err = nvs_flash_init();
        if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            ESP_LOGW(TAG, "No free pages or new version found, erasing nvs");
            ESP_ERROR_CHECK(nvs_flash_erase());
            err = nvs_flash_init();
        }
        ESP_ERROR_CHECK(err);
    }
}