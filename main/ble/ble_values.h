#pragma once

#include "services/gatt/ble_svc_gatt.h"

namespace ble
{
    namespace perm
    {
        constexpr ble_gatt_chr_flags READ = (BLE_GATT_CHR_F_READ);
        constexpr ble_gatt_chr_flags WRITE = (BLE_GATT_CHR_F_WRITE);
        constexpr ble_gatt_chr_flags EREAD = (READ | BLE_GATT_CHR_F_READ_ENC);
        constexpr ble_gatt_chr_flags EWRITE = (WRITE | BLE_GATT_CHR_F_WRITE_ENC);
        constexpr ble_gatt_chr_flags NOTIFY = BLE_GATT_CHR_F_NOTIFY;
        constexpr ble_gatt_chr_flags INDICATE = BLE_GATT_CHR_F_INDICATE;
    }

    namespace flags
    {
        constexpr int LELimitedDiscovery = 1 << 0;
        constexpr int LEGeneralDiscoverable = 1 << 1;
        constexpr int BR_EDRNotSupported = 1 << 2;
        constexpr int LE_And_BR_EDR_Capable_Controller = 1 << 3;
        constexpr int Previously_Used = 1 << 4;
        constexpr std::array CabinetLight{0x92, 0x05};
        
    }
}