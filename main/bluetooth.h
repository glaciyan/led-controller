#pragma once

#include <stdint.h>
#include <array>

#include "esp_log.h"
#include "esp_err.h"

#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_gap.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"

#include "gatt_service.h"

extern "C" void ble_store_config_init(void);

namespace bluetooth
{
    static const char *TAG = "bluetooth";

    // Forward Declarations
    void start_advertising();

    // See ext_adv_raw_data_generator.py
    constexpr std::array<uint8_t, 22> ext_adv_raw_data = {
        0x02, 0x01, 0x06,                                                                        //
        0x03, 0x19, 0x92, 0x05,                                                                  //
        0x0E, 0x09, 0x4D, 0x59, 0x5F, 0x42, 0x4C, 0x45, 0x5F, 0x44, 0x45, 0x56, 0x49, 0x43, 0x45 //
    };

    uint8_t own_addr_type = BLE_OWN_ADDR_RANDOM;

    void bleprph_on_reset(int reason)
    {
        MODLOG_DFLT(ERROR, "Resetting state; reason=%d\n", reason);
    }

    void ble_app_set_addr(void)
    {
        ble_addr_t addr;
        int rc;

        /* generate new non-resolvable private address */
        rc = ble_hs_id_gen_rnd(0, &addr);
        assert(rc == 0);

        /* set generated address */
        rc = ble_hs_id_set_rnd(addr.val);

        assert(rc == 0);
    }

    int my_gap_event(struct ble_gap_event *event, void *arg)
    {
        ble_gap_conn_desc desc;
        int rc;

        switch (event->type)
        {
        case BLE_GAP_EVENT_CONNECT:
            MODLOG_DFLT(INFO, "BLE_GAP_EVENT_CONNECT");

            // start advertising again if the connection failed
            if (event->connect.status != 0)
            {
                start_advertising();
            }

            return 0;

        case BLE_GAP_EVENT_DISCONNECT:
            MODLOG_DFLT(INFO, "disconnect; reason=%d", event->disconnect.reason);
            start_advertising();
            return 0;

        case BLE_GAP_EVENT_CONN_UPDATE:
            MODLOG_DFLT(INFO, "connection updated; status=%d",
                        event->conn_update.status);
            return 0;

        case BLE_GAP_EVENT_ADV_COMPLETE:
            MODLOG_DFLT(INFO, "advertise complete; reason=%d",
                        event->adv_complete.reason);
            return 0;
        case BLE_GAP_EVENT_ENC_CHANGE:
            /* Encryption has been enabled or disabled for this connection. */
            MODLOG_DFLT(INFO, "encryption change event; status=%d",
                        event->enc_change.status);
            return 0;

        case BLE_GAP_EVENT_NOTIFY_TX:
            MODLOG_DFLT(INFO, "notify_tx event; conn_handle=%d attr_handle=%d "
                              "status=%d is_indication=%d",
                        event->notify_tx.conn_handle,
                        event->notify_tx.attr_handle,
                        event->notify_tx.status,
                        event->notify_tx.indication);
            return 0;

        case BLE_GAP_EVENT_SUBSCRIBE:
            MODLOG_DFLT(INFO, "subscribe event; conn_handle=%d attr_handle=%d "
                              "reason=%d prevn=%d curn=%d previ=%d curi=%d",
                        event->subscribe.conn_handle,
                        event->subscribe.attr_handle,
                        event->subscribe.reason,
                        event->subscribe.prev_notify,
                        event->subscribe.cur_notify,
                        event->subscribe.prev_indicate,
                        event->subscribe.cur_indicate);
            return 0;

        case BLE_GAP_EVENT_MTU:
            MODLOG_DFLT(INFO, "mtu update event; conn_handle=%d cid=%d mtu=%d",
                        event->mtu.conn_handle,
                        event->mtu.channel_id,
                        event->mtu.value);
            return 0;

        case BLE_GAP_EVENT_REPEAT_PAIRING:
            /* We already have a bond with the peer, but it is attempting to
             * establish a new secure link.  This app sacrifices security for
             * convenience: just throw away the old bond and accept the new link.
             */

            /* Delete the old bond. */
            rc = ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc);
            assert(rc == 0);
            ble_store_util_delete_peer(&desc.peer_id_addr);

            /* Return BLE_GAP_REPEAT_PAIRING_RETRY to indicate that the host should
             * continue with the pairing operation.
             */
            return BLE_GAP_REPEAT_PAIRING_RETRY;

        case BLE_GAP_EVENT_PASSKEY_ACTION:
            ESP_LOGI(TAG, "PASSKEY_ACTION_EVENT started");
            return 0;

        case BLE_GAP_EVENT_AUTHORIZE:
            MODLOG_DFLT(INFO, "authorize event: conn_handle=%d attr_handle=%d is_read=%d",
                        event->authorize.conn_handle,
                        event->authorize.attr_handle,
                        event->authorize.is_read);

            /* The default behaviour for the event is to reject authorize request */
            event->authorize.out_response = BLE_GAP_AUTHORIZE_REJECT;
            return 0;
        }

        return 0;
    }

    void start_advertising()
    {
        ble_gap_ext_adv_params params{};
        os_mbuf *data;
        uint8_t instance = 0;

        if (ble_gap_ext_adv_active(instance))
        {
            return;
        }

        memset(&params, 0, sizeof(params));

        params.connectable = 1;

        params.own_addr_type = BLE_OWN_ADDR_PUBLIC;

        params.primary_phy = BLE_HCI_LE_PHY_1M;
        params.secondary_phy = BLE_HCI_LE_PHY_CODED;
        params.sid = 1;

        params.itvl_min = BLE_GAP_ADV_FAST_INTERVAL1_MIN;
        params.itvl_max = BLE_GAP_ADV_FAST_INTERVAL1_MIN;

        int rc = ble_gap_ext_adv_configure(instance, &params, NULL, my_gap_event, NULL);
        assert(rc == 0);

        data = os_msys_get_pkthdr(sizeof(ext_adv_raw_data), 0);
        assert(data);

        rc = os_mbuf_append(data, ext_adv_raw_data.data(), ext_adv_raw_data.size());
        assert(rc == 0);

        rc = ble_gap_ext_adv_set_data(instance, data);
        assert(rc == 0);

        ESP_LOGI(TAG, "Start Extended Advertising");
        rc = ble_gap_ext_adv_start(instance, 0, 0);
        assert(rc == 0);
    }

    void bleprph_on_sync(void)
    {
        // ble_app_set_addr();

        // int rc;
        // rc = ble_hs_util_ensure_addr(true);
        // assert(rc == 0);

        // rc = ble_hs_id_infer_auto(false, &own_addr_type);
        // if (rc != 0)
        // {
        //     MODLOG_DFLT(ERROR, "error determining address type; rc=%d\n", rc);
        //     return;
        // }

        start_advertising();
    }

    void my_host_task(void *param)
    {
        ESP_LOGI(TAG, "BLE Host Task Started");
        /* This function will return only when nimble_port_stop() is executed */
        nimble_port_run();

        nimble_port_freertos_deinit();
    }

    constexpr void init_ble()
    {
        ESP_LOGI(TAG, "Initializing BLE");

        esp_err_t ret;

        ret = nimble_port_init();
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to init nimble %d", ret);
            return;
        }

        ble_hs_cfg.reset_cb = bleprph_on_reset;
        ble_hs_cfg.sync_cb = bleprph_on_sync;

        int rc = init_gatt();
        assert(rc == 0);

        ble_store_config_init();

        nimble_port_freertos_init(my_host_task);
    }
}