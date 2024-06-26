#include <array>

#include "esp_log.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "console/console.h"
#include "services/gap/ble_svc_gap.h"
#include "bleprph.h"
#include "esp_bt.h"
#include "ble_gatt_svr.h"

#include "ble_config.h"
#include "ble_advertising.h"

extern "C" void ble_store_config_init(void);

namespace ble
{
    const char *tag = "NimBLE_BLE_PRPH";
    uint8_t own_addr_type = BLE_OWN_ADDR_RPA_RANDOM_DEFAULT;

    void bleprph_on_reset(int reason)
    {
        MODLOG_DFLT(ERROR, "Resetting state; reason=%d\n", reason);
    }

    const char *storage_namespace = "ble_device";
    const char *static_address_key = "static_address";

    void bleprph_on_sync()
    {
        int rc;
        esp_err_t err;

        nvs_handle_t handle;
        err = nvs_open(storage_namespace, NVS_READWRITE, &handle);
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
        rc = ble_hs_id_copy_addr(BLE_ADDR_RANDOM, addr_val, nullptr);
        assert(rc == 0);

        MODLOG_DFLT(INFO, "Device Address: ");
        print_addr(addr_val);
        MODLOG_DFLT(INFO, "\n");

        /* Begin advertising. */
        ext_bleprph_advertise();
    }

    void bleprph_host_task([[maybe_unused]] void *param)
    {
        ESP_LOGI(tag, "BLE Host Task Started");
        /* This function will return only when nimble_port_stop() is executed */
        nimble_port_run();

        nimble_port_freertos_deinit();
    }

    void init_bluetooth(const ble_gatt_svc_def *gatt_services)
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

        rc = ble::gatt_svr_init(gatt_services);
        assert(rc == 0);

        /* Set the default device name. */
        rc = ble_svc_gap_device_name_set(DEVICE_NAME);
        assert(rc == 0);

        /* XXX Need to have template for store */
        ble_store_config_init();

        nimble_port_freertos_init(bleprph_host_task);
    }
}