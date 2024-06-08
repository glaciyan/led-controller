#pragma once

#include <stdint.h>
#include <array>

#include "esp_log.h"
#include "esp_err.h"

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"

namespace bluetooth
{
    static const char *TAG = "bluetooth";

    constexpr int32_t PROFILES_COUNT = 1;
    constexpr int32_t HEART_PROFILE_IDX = 0;

    constexpr int32_t EXT_ADV_HANDLE = 0;
    constexpr int32_t NUM_EXT_ADV_SET = 1;
    constexpr int32_t EXT_ADV_DURATION = 0;
    constexpr int32_t EXT_ADV_MAX_EVENTS = 0;

    // See ext_adv_raw_data_generator.py
    constexpr std::array<uint8_t, 28> ext_adv_raw_data = {
        0x02, 0x01, 0x06,                                                                                          //
        0x02, 0x0A, 0xEB,                                                                                          //
        0x03, 0x03, 0xAB, 0xCD,                                                                                    //
        0x11, 0x09, 0x45, 0x53, 0x50, 0x5F, 0x42, 0x4C, 0x45, 0x35, 0x30, 0x5F, 0x53, 0x45, 0x52, 0x56, 0x45, 0x52 //
    };

    constexpr esp_ble_gap_ext_adv_t ext_adv_enable_params{EXT_ADV_HANDLE, EXT_ADV_DURATION, EXT_ADV_MAX_EVENTS};

    esp_ble_gap_ext_adv_params_t ext_adv_params_coded = {
        .type = ESP_BLE_GAP_SET_EXT_ADV_PROP_SCANNABLE,
        .interval_min = 0x50,
        .interval_max = 0x50,
        .channel_map = ADV_CHNL_ALL,
        .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .peer_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .peer_addr = 0,
        .filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
        .tx_power = EXT_ADV_TX_PWR_NO_PREFERENCE,
        .primary_phy = ESP_BLE_GAP_PHY_1M,
        .max_skip = 0,
        .secondary_phy = ESP_BLE_GAP_PHY_CODED,
        .sid = 3,
        .scan_req_notif = false,
    };

    void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
    {
        switch (event)
        {
        case ESP_GAP_BLE_EXT_ADV_SET_PARAMS_COMPLETE_EVT:
            ESP_LOGI(TAG, "ESP_GAP_BLE_EXT_ADV_SET_PARAMS_COMPLETE_EVT status %d", param->ext_adv_set_params.status);
            esp_ble_gap_config_ext_adv_data_raw(EXT_ADV_HANDLE, ext_adv_raw_data.size(), ext_adv_raw_data.data());
            break;
        case ESP_GAP_BLE_EXT_ADV_DATA_SET_COMPLETE_EVT:
            ESP_LOGI(TAG, "ESP_GAP_BLE_EXT_ADV_DATA_SET_COMPLETE_EVT status %d", param->ext_adv_data_set.status);
            esp_ble_gap_ext_adv_start(NUM_EXT_ADV_SET, &ext_adv_enable_params);
            break;
        case ESP_GAP_BLE_EXT_ADV_START_COMPLETE_EVT:
            ESP_LOGI(TAG, "ESP_GAP_BLE_EXT_ADV_START_COMPLETE_EVT, status = %d", param->ext_adv_data_set.status);
            break;
        case ESP_GAP_BLE_ADV_TERMINATED_EVT:
            ESP_LOGI(TAG, "ESP_GAP_BLE_ADV_TERMINATED_EVT, status = %d", param->adv_terminate.status);
            if (param->adv_terminate.status == 0x00)
            {
                ESP_LOGI(TAG, "ADV successfully ended with a connection being created");
            }
            break;
        case ESP_GAP_BLE_PASSKEY_REQ_EVT: /* passkey request event */
            /* Call the following function to input the passkey which is displayed on the remote device */
            // esp_ble_passkey_reply(heart_rate_profile_tab[HEART_PROFILE_APP_IDX].remote_bda, true, 0x00);
            break;
        case ESP_GAP_BLE_OOB_REQ_EVT:
        {
            ESP_LOGI(TAG, "ESP_GAP_BLE_OOB_REQ_EVT");
            uint8_t tk[16] = {1}; // If you paired with OOB, both devices need to use the same tk
            esp_ble_oob_req_reply(param->ble_security.ble_req.bd_addr, tk, sizeof(tk));
            break;
        }
        case ESP_GAP_BLE_LOCAL_IR_EVT: /* BLE local IR event */
            ESP_LOGI(TAG, "ESP_GAP_BLE_LOCAL_IR_EVT");
            break;
        case ESP_GAP_BLE_LOCAL_ER_EVT: /* BLE local ER event */
            ESP_LOGI(TAG, "ESP_GAP_BLE_LOCAL_ER_EVT");
            break;
        case ESP_GAP_BLE_NC_REQ_EVT:
            /* The app will receive this evt when the IO has DisplayYesNO capability and the peer device IO also has DisplayYesNo capability.
            show the passkey number to the user to confirm it with the number displayed by peer device. */
            esp_ble_confirm_reply(param->ble_security.ble_req.bd_addr, true);
            ESP_LOGI(TAG, "ESP_GAP_BLE_NC_REQ_EVT, the passkey Notify number:%" PRIu32, param->ble_security.key_notif.passkey);
            break;
        case ESP_GAP_BLE_SEC_REQ_EVT:
            /* send the positive(true) security response to the peer device to accept the security request.
            If not accept the security request, should send the security response with negative(false) accept value*/
            esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
            break;
        case ESP_GAP_BLE_PASSKEY_NOTIF_EVT: /// the app will receive this evt when the IO  has Output capability and the peer device IO has Input capability.
            /// show the passkey number to the user to input it in the peer device.
            ESP_LOGI(TAG, "The passkey Notify number:%06" PRIu32, param->ble_security.key_notif.passkey);
            break;
        case ESP_GAP_BLE_KEY_EVT:
            // shows the ble key info share with peer device to the user.
            ESP_LOGI(TAG, "key type = %d", param->ble_security.ble_key.key_type);
            break;
        case ESP_GAP_BLE_AUTH_CMPL_EVT:
        {
            ESP_LOGI(TAG, "pair status = %s", param->ble_security.auth_cmpl.success ? "success" : "fail");
            if (!param->ble_security.auth_cmpl.success)
            {
                ESP_LOGI(TAG, "fail reason = 0x%x", param->ble_security.auth_cmpl.fail_reason);
            }
            else
            {
                ESP_LOGI(TAG, "auth mode = %d", param->ble_security.auth_cmpl.auth_mode);
            }
            // show_bonded_devices();
            break;
        }
        case ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT:
        {
            ESP_LOGD(TAG, "ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT status = %d", param->remove_bond_dev_cmpl.status);
            ESP_LOGI(TAG, "ESP_GAP_BLE_REMOVE_BOND_DEV");
            ESP_LOGI(TAG, "-----ESP_GAP_BLE_REMOVE_BOND_DEV----");
            esp_log_buffer_hex(TAG, (void *)param->remove_bond_dev_cmpl.bd_addr, sizeof(esp_bd_addr_t));
            ESP_LOGI(TAG, "------------------------------------");
            break;
        }
        case ESP_GAP_BLE_SET_LOCAL_PRIVACY_COMPLETE_EVT:
            ESP_LOGI(TAG, "ESP_GAP_BLE_SET_LOCAL_PRIVACY_COMPLETE_EVT, tatus = %x", param->local_privacy_cmpl.status);
            esp_ble_gap_ext_adv_set_params(EXT_ADV_HANDLE, &ext_adv_params_coded);
            break;
        case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
            ESP_LOGI(TAG, "update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
                     param->update_conn_params.status,
                     param->update_conn_params.min_int,
                     param->update_conn_params.max_int,
                     param->update_conn_params.conn_int,
                     param->update_conn_params.latency,
                     param->update_conn_params.timeout);
            break;
        default:
            break;
        }
    }

    void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
    {
    }

    constexpr void init_ble()
    {
        ESP_LOGI(TAG, "Initializing BLE");

        ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

        esp_err_t ret;
        esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
        ret = esp_bt_controller_init(&bt_cfg);
        if (ret)
        {
            ESP_LOGE(TAG, "%s initialize controller failed: %s", __func__, esp_err_to_name(ret));
            return;
        }

        ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
        if (ret)
        {
            ESP_LOGE(TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
            return;
        }

        esp_bluedroid_config_t bd_cfg = BT_BLUEDROID_INIT_CONFIG_DEFAULT();
        ret = esp_bluedroid_init_with_cfg(&bd_cfg);
        if (ret)
        {
            ESP_LOGE(TAG, "%s init bluetooth failed: %s", __func__, esp_err_to_name(ret));
            return;
        }

        ret = esp_bluedroid_enable();
        if (ret)
        {
            ESP_LOGE(TAG, "%s enable bluetooth failed: %s", __func__, esp_err_to_name(ret));
            return;
        }

        // ret = esp_ble_gatts_register_callback(gatts_event_handler);
        // if (ret)
        // {
        //     ESP_LOGE(TAG, "gatts register error, error code = %x", ret);
        //     return;
        // }

        ret = esp_ble_gap_register_callback(gap_event_handler);
        if (ret)
        {
            ESP_LOGE(TAG, "gap register error, error code = %x", ret);
            return;
        }
    }
}