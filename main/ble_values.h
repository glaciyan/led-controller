#pragma once

#include "services/gatt/ble_svc_gatt.h"

namespace ble
{
    namespace perm
    {
        const ble_gatt_chr_flags READ = (BLE_GATT_CHR_F_READ);
        const ble_gatt_chr_flags WRITE = (BLE_GATT_CHR_F_WRITE);
        const ble_gatt_chr_flags EREAD = (READ | BLE_GATT_CHR_F_READ_ENC);
        const ble_gatt_chr_flags EWRITE = (WRITE | BLE_GATT_CHR_F_WRITE_ENC);
    }

    namespace flags
    {
        const int LELimitedDiscovery = 1 << 0;
        const int LEGeneralDiscoverable = 1 << 1;
        const int BR_EDRNotSupported = 1 << 2;
        const int LE_And_BR_EDR_Capable_Controller = 1 << 3;
        const int Previously_Used = 1 << 4;
    }
}