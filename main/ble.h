#include <array>

#include "esp_log.h"
/* BLE */
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "console/console.h"
#include "services/gap/ble_svc_gap.h"
#include "bleprph.h"
#include "esp_bt.h"
#include "ble_gatt_svr.h"

constexpr std::array<uint8_t, 27> ext_adv_raw_data = {
    0x02, 0x01, 0x06, //
    0x03, 0x19, 0x92, 0x05, //
    0x13, 0x09, 0x4C, 0x45, 0x44, 0x20, 0x53, 0x68, 0x6F, 0x77, 0x63, 0x61, 0x73, 0x65, 0x20, 0x4C, 0x69, 0x67, 0x68, 0x74 //
};

const char *tag = "NimBLE_BLE_PRPH";
uint8_t own_addr_type = BLE_OWN_ADDR_RPA_RANDOM_DEFAULT;

int bleprph_gap_event(struct ble_gap_event *event, void *arg);
extern "C" void ble_store_config_init(void);

/**
 * Logs information about a connection to the console.
 */
void bleprph_print_conn_desc(ble_gap_conn_desc *desc)
{
    MODLOG_DFLT(INFO, "handle=%d our_ota_addr_type=%d rpa=%d our_ota_addr=",
                desc->conn_handle, desc->our_ota_addr.type, BLE_ADDR_IS_RPA(&desc->our_ota_addr));
    print_addr(desc->our_ota_addr.val);
    MODLOG_DFLT(INFO, " our_id_addr_type=%d rpa=%d our_id_addr=",
                desc->our_id_addr.type, BLE_ADDR_IS_RPA(&desc->our_id_addr));
    print_addr(desc->our_id_addr.val);
    MODLOG_DFLT(INFO, " peer_ota_addr_type=%d rpa=%d  peer_ota_addr=",
                desc->peer_ota_addr.type, BLE_ADDR_IS_RPA(&desc->peer_ota_addr));
    print_addr(desc->peer_ota_addr.val);
    MODLOG_DFLT(INFO, " peer_id_addr_type=%d rpa=%d peer_id_addr=",
                desc->peer_id_addr.type, BLE_ADDR_IS_RPA(&desc->peer_id_addr));
    print_addr(desc->peer_id_addr.val);
    MODLOG_DFLT(INFO, " conn_itvl=%d conn_latency=%d supervision_timeout=%d "
                      "encrypted=%d authenticated=%d bonded=%d\n",
                desc->conn_itvl, desc->conn_latency,
                desc->supervision_timeout,
                desc->sec_state.encrypted,
                desc->sec_state.authenticated,
                desc->sec_state.bonded);
}

/**
 * Enables advertising with the following parameters:
 *     o General discoverable mode.
 *     o Undirected connectable mode.
 */
constexpr void ext_bleprph_advertise(void)
{
    ble_gap_ext_adv_params params;
    os_mbuf *data;
    uint8_t instance = 0;
    int rc;

    /* First check if any instance is already active */
    if (ble_gap_ext_adv_active(instance))
    {
        return;
    }

    /* use defaults for non-set params */
    memset(&params, 0, sizeof(params));

    /* enable connectable advertising */
    params.connectable = 1;

    /* advertise using random addr */
    params.own_addr_type = BLE_OWN_ADDR_RPA_RANDOM_DEFAULT;

    params.primary_phy = BLE_HCI_LE_PHY_1M;
    params.secondary_phy = BLE_HCI_LE_PHY_2M;
    // params.tx_power = 127;
    params.sid = 1;

    params.itvl_min = BLE_GAP_ADV_FAST_INTERVAL2_MIN;
    params.itvl_max = BLE_GAP_ADV_FAST_INTERVAL2_MAX;

    /* configure instance 0 */
    rc = ble_gap_ext_adv_configure(instance, &params, NULL,
                                   bleprph_gap_event, NULL);
    assert(rc == 0);

    /* in this case only scan response is allowed */

    /* get mbuf for scan rsp data */
    data = os_msys_get_pkthdr(ext_adv_raw_data.size(), 0);
    assert(data);

    /* fill mbuf with scan rsp data */
    rc = os_mbuf_append(data, ext_adv_raw_data.data(), ext_adv_raw_data.size());
    assert(rc == 0);

    rc = ble_gap_ext_adv_set_data(instance, data);
    assert(rc == 0);

    /* start advertising */
    rc = ble_gap_ext_adv_start(instance, 0, 0);
    // assert (rc == 0);
    if (rc != 0)
    {
        MODLOG_DFLT(ERROR, "failed to start extended advertisement %d", rc);
    }
}

/**
 * The nimble host executes this callback when a GAP event occurs.  The
 * application associates a GAP event callback with each connection that forms.
 * bleprph uses the same callback for all connections.
 *
 * @param event                 The type of event being signalled.
 * @param ctxt                  Various information pertaining to the event.
 * @param arg                   Application-specified argument; unused by
 *                                  bleprph.
 *
 * @return                      0 if the application successfully handled the
 *                                  event; nonzero on failure.  The semantics
 *                                  of the return code is specific to the
 *                                  particular GAP event being signalled.
 */
int bleprph_gap_event(ble_gap_event *event, void *arg)
{
    ble_gap_conn_desc desc;
    int rc;

    switch (event->type)
    {
    case BLE_GAP_EVENT_CONNECT:
        /* A new connection was established or a connection attempt failed. */
        MODLOG_DFLT(INFO, "connection %s; status=%d ",
                    event->connect.status == 0 ? "established" : "failed",
                    event->connect.status);
        if (event->connect.status == 0)
        {
            rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
            assert(rc == 0);
            bleprph_print_conn_desc(&desc);
        }

        if (event->connect.status != 0)
        {
            /* Connection failed; resume advertising. */
            ext_bleprph_advertise();
        }

        return 0;

    case BLE_GAP_EVENT_DISCONNECT:
        MODLOG_DFLT(INFO, "disconnect; reason=%d ", event->disconnect.reason);
        bleprph_print_conn_desc(&event->disconnect.conn);

        /* Connection terminated; resume advertising. */
        ext_bleprph_advertise();
        return 0;

    case BLE_GAP_EVENT_CONN_UPDATE:
        /* The central has updated the connection parameters. */
        MODLOG_DFLT(INFO, "connection updated; status=%d ",
                    event->conn_update.status);
        rc = ble_gap_conn_find(event->conn_update.conn_handle, &desc);
        assert(rc == 0);
        bleprph_print_conn_desc(&desc);
        return 0;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        MODLOG_DFLT(INFO, "advertise complete; reason=%d",
                    event->adv_complete.reason);
        // TODO: why are we advertising again
        // ext_bleprph_advertise();
        return 0;

    case BLE_GAP_EVENT_ENC_CHANGE:
        /* Encryption has been enabled or disabled for this connection. */
        MODLOG_DFLT(INFO, "encryption change event; status=%d ",
                    event->enc_change.status);
        rc = ble_gap_conn_find(event->enc_change.conn_handle, &desc);
        assert(rc == 0);
        bleprph_print_conn_desc(&desc);
        return 0;

    case BLE_GAP_EVENT_IDENTITY_RESOLVED:
        MODLOG_DFLT(INFO, "identity resolved");
        rc = ble_gap_conn_find(event->identity_resolved.conn_handle, &desc);
        assert(rc == 0);
        bleprph_print_conn_desc(&desc);
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
                          "reason=%d prevn=%d curn=%d previ=%d curi=%d\n",
                    event->subscribe.conn_handle,
                    event->subscribe.attr_handle,
                    event->subscribe.reason,
                    event->subscribe.prev_notify,
                    event->subscribe.cur_notify,
                    event->subscribe.prev_indicate,
                    event->subscribe.cur_indicate);
        return 0;

    case BLE_GAP_EVENT_MTU:
        MODLOG_DFLT(INFO, "mtu update event; conn_handle=%d cid=%d mtu=%d\n",
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

void bleprph_on_reset(int reason)
{
    MODLOG_DFLT(ERROR, "Resetting state; reason=%d\n", reason);
}

#define STORAGE_NAMESPACE "ble_kevin"
static const char *static_address_key = "static_address";

void bleprph_on_sync(void)
{
    int rc = 0;
    esp_err_t err = 0;

    nvs_handle_t handle;
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    ESP_ERROR_CHECK(err);

    ble_addr_t addr{BLE_ADDR_RANDOM, {0, 0, 0, 0, 0, 0}};

    size_t blob_size = 6;
    ESP_LOGI(tag, "Trying to read static address from nvs");
    err = nvs_get_blob(handle, static_address_key, &addr.val, &blob_size);
    if (err == ESP_OK)
    {
        ESP_LOGI(tag, "Found static address in nvs");
    }
    else if (err == ESP_ERR_NVS_NOT_FOUND || blob_size == 0)
    {
        ESP_LOGI(tag, "Static address was not found. Generating a new one and persisting it. err: %d, size %d", err, blob_size);
        rc = ble_hs_id_gen_rnd(0, &addr);
        if (rc != 0)
        {
            MODLOG_DFLT(ERROR, "Failed to generate random address");
            assert(false);
        }

        err = nvs_set_blob(handle, static_address_key, &addr.val, sizeof(addr.val));
        ESP_ERROR_CHECK(err);
    }

    nvs_close(handle);

    addr.val[5] |= 0xc0;

    // rc = ble_hs_id_gen_rnd(0, &addr);
    // if (rc != 0)
    // {
    //     MODLOG_DFLT(ERROR, "Failed to generate random address");
    //     assert(false);
    // }

    // rc = ble_hs_id_copy_addr(BLE_ADDR_RANDOM, NULL, NULL);
    // if (rc == 0)
    // {
    //     MODLOG_DFLT(INFO, "Already got a random address");
    // }
    // else
    // {
    //     MODLOG_DFLT(INFO, "No random address already set, searching for one");
    //     rc = esp_ble_hw_get_static_addr((esp_ble_addr_t *)&addr);
    //     if (rc != 0)
    //     {
    //         MODLOG_DFLT(INFO, "No static random address found, generating one");
    //         /* If it didn't work, generate random address */
    //         rc = ble_hs_id_gen_rnd(0, &addr);
    //         if (rc != 0)
    //         {
    //             MODLOG_DFLT(ERROR, "Failed to generate random address");
    //             assert(false);
    //         }
    //     }

    //     /* Configure nimble to use the random address. */
    //     rc = ble_hs_id_set_rnd(addr.val);
    //     if (rc != 0)
    //     {
    //         MODLOG_DFLT(ERROR, "Failed to set random address");
    //         assert(false);
    //     }
    // }

    /* Configure nimble to use the random address. */
    rc = ble_hs_id_set_rnd(addr.val);
    if (rc != 0)
    {
        MODLOG_DFLT(ERROR, "Failed to set random address");
        assert(false);
    }

    rc = ble_hs_id_infer_auto(1, &own_addr_type);
    if (rc != 0)
    {
        MODLOG_DFLT(ERROR, "error determining address type; rc=%d\n", rc);
        return;
    }
    MODLOG_DFLT(INFO, "Inferred address type %d", own_addr_type);

    /* Printing ADDR */
    uint8_t addr_val[6] = {0};
    rc = ble_hs_id_copy_addr(BLE_ADDR_RANDOM, addr_val, NULL);

    MODLOG_DFLT(INFO, "Device Address: ");
    print_addr(addr_val);
    MODLOG_DFLT(INFO, "\n");

    /* Begin advertising. */
    ext_bleprph_advertise();
}

void bleprph_host_task(void *param)
{
    ESP_LOGI(tag, "BLE Host Task Started");
    /* This function will return only when nimble_port_stop() is executed */
    nimble_port_run();

    nimble_port_freertos_deinit();
}

constexpr void init_bluetooth(void)
{
    int rc;

    esp_err_t ret = nimble_port_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(tag, "Failed to init nimble %d ", ret);
        return;
    }
    /* Initialize the NimBLE host configuration. */
    ble_hs_cfg.reset_cb = bleprph_on_reset;
    ble_hs_cfg.sync_cb = bleprph_on_sync;
    ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    ble_hs_cfg.sm_io_cap = BLE_SM_IO_CAP_NO_IO;
    ble_hs_cfg.sm_bonding = 1;
    ble_hs_cfg.sm_mitm = 1;
    ble_hs_cfg.sm_sc = 1;

    /* Enable the appropriate bit masks to make sure the keys
     * that are needed are exchanged
     */
    ble_hs_cfg.sm_our_key_dist |= BLE_SM_PAIR_KEY_DIST_ENC;
    ble_hs_cfg.sm_their_key_dist |= BLE_SM_PAIR_KEY_DIST_ENC;

    /* Stores the IRK */
    ble_hs_cfg.sm_our_key_dist |= BLE_SM_PAIR_KEY_DIST_ID;
    ble_hs_cfg.sm_their_key_dist |= BLE_SM_PAIR_KEY_DIST_ID;

    /* Signing Keys */
    ble_hs_cfg.sm_our_key_dist |= BLE_SM_PAIR_KEY_DIST_SIGN;
    ble_hs_cfg.sm_their_key_dist |= BLE_SM_PAIR_KEY_DIST_SIGN;

    rc = ble::gatt_svr_init();
    assert(rc == 0);

    /* Set the default device name. */
    rc = ble_svc_gap_device_name_set("led-showcase-light");
    assert(rc == 0);

    /* XXX Need to have template for store */
    ble_store_config_init();

    nimble_port_freertos_init(bleprph_host_task);
}
