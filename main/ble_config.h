#pragma once

#include "ble/ble_values.h"
#include "ble/ble_gap_util.h"

#include "ble_color_characteristic.h"

#define DEVICE_NAME "ShowShow"
constexpr std::array Appearance = ble::flags::CabinetLight;

constexpr ble_uuid128_t color_service_uuid = UUID128("88feb8853fa6c610010abf2eee3b7de3");
constexpr ble_uuid128_t rgb_characteristic_uuid = UUID128("945683fd41b67f3c69d929fdc6dcef52");

constexpr auto ext_adv_raw_data = concatenate(
        tag_data(0x01, std::array{ble::flags::LEGeneralDiscoverable | ble::flags::BR_EDRNotSupported}),
        tag_data(0x19, Appearance),
        device_name(DEVICE_NAME)
);

constexpr ble_gatt_svc_def gatt_services[] = SERVICE_LIST(
        SERVICE(
                UUID(color_service_uuid),
                CHARACTERISTIC(
                        UUID(rgb_characteristic_uuid),
                        color_characteristic_access,
                        ble::perm::EREAD | ble::perm::EWRITE | ble::perm::NOTIFY,
                        &color_characteristic_attr_handle
                )
        )
);
