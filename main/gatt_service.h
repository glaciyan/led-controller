#pragma once

#include <array>
#include <assert.h>
#include <stdlib.h>

#include "host/ble_gatt.h"
#include "host/ble_hs_mbuf.h"

namespace bluetooth
{
    /* 59462f12-9543-9999-12c8-58b459a2712d */
    constexpr ble_uuid128_t gatt_svr_svc_sec_test_uuid =
        BLE_UUID128_INIT(0x2d, 0x71, 0xa2, 0x59, 0xb4, 0x58, 0xc8, 0x12, 0x99, 0x99,
                         0x43, 0x95, 0x12, 0x2f, 0x46, 0x59);

    /* 5c3a659e-897e-45e1-b016-007107c96df6 */
    constexpr ble_uuid128_t gatt_svr_chr_sec_test_rand_uuid =
        BLE_UUID128_INIT(0xf6, 0x6d, 0xc9, 0x07, 0x71, 0x00, 0x16, 0xb0, 0xe1, 0x45,
                         0x7e, 0x89, 0x9e, 0x65, 0x3a, 0x5c);

    /* 5c3a659e-897e-45e1-b016-007107c96df7 */
    constexpr ble_uuid128_t gatt_svr_chr_sec_test_static_uuid =
        BLE_UUID128_INIT(0xf7, 0x6d, 0xc9, 0x07, 0x71, 0x00, 0x16, 0xb0, 0xe1, 0x45,
                         0x7e, 0x89, 0x9e, 0x65, 0x3a, 0x5c);

    uint8_t gatt_svr_sec_test_static_val;

    int gatt_svr_chr_write(struct os_mbuf *om, uint16_t min_len, uint16_t max_len,
                           void *dst, uint16_t *len)
    {
        uint16_t om_len{};
        int rc;

        om_len = OS_MBUF_PKTLEN(om);
        if (om_len < min_len || om_len > max_len)
        {
            return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        }

        rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);
        if (rc != 0)
        {
            return BLE_ATT_ERR_UNLIKELY;
        }

        return 0;
    }

    int gatt_svr_chr_access_sec_test(uint16_t conn_handle, uint16_t attr_handle,
                                     struct ble_gatt_access_ctxt *ctxt,
                                     void *arg)
    {
        const ble_uuid_t *uuid;
        int rand_num;
        int rc;

        uuid = ctxt->chr->uuid;

        /* Determine which characteristic is being accessed by examining its
         * 128-bit UUID.
         */

        if (ble_uuid_cmp(uuid, &gatt_svr_chr_sec_test_rand_uuid.u) == 0)
        {
            assert(ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR);

            /* Respond with a 32-bit random number. */
            rand_num = rand();
            rc = os_mbuf_append(ctxt->om, &rand_num, sizeof rand_num);
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        }

        if (ble_uuid_cmp(uuid, &gatt_svr_chr_sec_test_static_uuid.u) == 0)
        {
            switch (ctxt->op)
            {
            case BLE_GATT_ACCESS_OP_READ_CHR:
                rc = os_mbuf_append(ctxt->om, &gatt_svr_sec_test_static_val,
                                    sizeof gatt_svr_sec_test_static_val);
                return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

            case BLE_GATT_ACCESS_OP_WRITE_CHR:
                rc = gatt_svr_chr_write(ctxt->om,
                                        sizeof gatt_svr_sec_test_static_val,
                                        sizeof gatt_svr_sec_test_static_val,
                                        &gatt_svr_sec_test_static_val, NULL);
                return rc;

            default:
                assert(0);
                return BLE_ATT_ERR_UNLIKELY;
            }
        }

        /* Unknown characteristic; the nimble stack should not have called this
         * function.
         */
        assert(0);
        return BLE_ATT_ERR_UNLIKELY;
    }

    constexpr std::array<ble_gatt_chr_def, 3>
        service_security_test_characteristics = {
            ble_gatt_chr_def{
                .uuid = &gatt_svr_chr_sec_test_rand_uuid.u,
                .access_cb = gatt_svr_chr_access_sec_test,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_READ_ENC,
                },

            ble_gatt_chr_def{
                .uuid = &gatt_svr_chr_sec_test_static_uuid.u,
                .access_cb = gatt_svr_chr_access_sec_test,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_ENC},

            0 // terminate
    };

    constexpr std::array<ble_gatt_svc_def, 2> gatt_services = {
        ble_gatt_svc_def{
            .type = BLE_GATT_SVC_TYPE_PRIMARY,
            .uuid = &gatt_svr_svc_sec_test_uuid.u,
            .characteristics = service_security_test_characteristics.data()},

        0 // terminate
    };

    constexpr int init_gatt() {
        int rc = ble_gatts_count_cfg(gatt_services.data());
        if (rc != 0) {
            MODLOG_DFLT(INFO, "Failed to count services %d", rc);
            return rc;
        }

        rc = ble_gatts_add_svcs(gatt_services.data());
        if (rc != 0) {
            MODLOG_DFLT(INFO, "Failed to add services %d", rc);
            return rc;
        }

        return 0;
    }
}