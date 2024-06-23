#pragma once

#include "hex_util.h"

#define CHARACTERISTIC(__uid, __callback, __flags, __vh) {.uuid = __uid, .access_cb = __callback, .flags = __flags, .val_handle = __vh}

#define SERVICE(__uid, __chrs...)               \
    {                                           \
        .type = BLE_GATT_SVC_TYPE_PRIMARY,      \
        .uuid = __uid,                          \
        .includes = nullptr,                    \
        .characteristics = (ble_gatt_chr_def[]) \
        {                                       \
            __chrs,                             \
            {                                   \
                0                               \
            }                                   \
        }                                       \
    }

#define SERVICE_LIST(__services...) \
    {                               \
        __services,                 \
        {                           \
            0                       \
        }                           \
    }

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
