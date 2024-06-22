#pragma once

#include "hex_util.h"

namespace ble
{
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