#pragma once

#include "services/gatt/ble_svc_gatt.h"
#include "hex_util.h"

namespace ble
{
    const ble_gatt_chr_flags READ = (BLE_GATT_CHR_F_READ);
    const ble_gatt_chr_flags WRITE = (BLE_GATT_CHR_F_WRITE);
    const ble_gatt_chr_flags EREAD = (READ | BLE_GATT_CHR_F_READ_ENC);
    const ble_gatt_chr_flags EWRITE = (WRITE | BLE_GATT_CHR_F_WRITE_ENC);

    consteval ble_uuid128_t UUID128(const char (&hex)[33])
    {
        ble_uuid128_t uuid{BLE_UUID_TYPE_128, {}};
        for (std::size_t i = 0; i < 16; ++i)
        {
            uuid.value[i] = util::hexPairToByte(hex[2 * i], hex[2 * i + 1]);
        }
        return uuid;
    }

}