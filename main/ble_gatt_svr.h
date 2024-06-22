/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <cassert>
#include <cstdio>
#include <cstring>
#include <array>
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "bleprph.h"
#include "services/ans/ble_svc_ans.h"

#include "ble_gatt_util.h"

extern "C" void ble_ans_init();

namespace ble
{
    constexpr int MAX_NOTIFY = 5;

    constexpr ble_uuid128_t gatt_svr_svc_uuid = UUID128("88feb8853fa6c610010abf2eee3b7de3");

    uint8_t gatt_svr_chr_val;
    uint16_t gatt_svr_chr_val_handle;
    const ble_uuid128_t gatt_svr_chr_uuid = UUID128("945683fd41b67f3c69d929fdc6dcef52");

    int gatt_svc_access(uint16_t conn_handle, uint16_t attr_handle,
                        ble_gatt_access_ctxt *ctxt, void *arg);

    const ble_gatt_svc_def gatt_svr_svcs[] = {
        {
            /*** Service ***/
            .type = BLE_GATT_SVC_TYPE_PRIMARY,
            .uuid = &gatt_svr_svc_uuid.u,
            .characteristics = (ble_gatt_chr_def[]){
                {
                    .uuid = &gatt_svr_chr_uuid.u,
                    .access_cb = gatt_svc_access,
                    .flags = EREAD | EWRITE,
                    .val_handle = &gatt_svr_chr_val_handle,
                },
                {
                    0,
                }},
        },

        {
            0,
        },
    };

    int gatt_svr_write(os_mbuf *om, uint16_t min_len, uint16_t max_len,
                       void *dst, uint16_t *len)
    {
        uint16_t om_len = OS_MBUF_PKTLEN(om);
        if (om_len < min_len || om_len > max_len)
        {
            return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        }

        int rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);
        if (rc != 0)
        {
            return BLE_ATT_ERR_UNLIKELY;
        }

        return 0;
    }

    /**
     * Access callback whenever a characteristic/descriptor is read or written to.
     * Here reads and writes need to be handled.
     * ctxt->op tells weather the operation is read or write and
     * weather it is on a characteristic or descriptor,
     * ctxt->dsc->uuid tells which characteristic/descriptor is accessed.
     * attr_handle give the value handle of the attribute being accessed.
     * Accordingly do:
     *     Append the value to ctxt->om if the operation is READ
     *     Write ctxt->om to the value if the operation is WRITE
     **/
    int gatt_svc_access(uint16_t conn_handle, uint16_t attr_handle,
                        ble_gatt_access_ctxt *ctxt, void *arg)
    {
        int rc;

        switch (ctxt->op)
        {
        case BLE_GATT_ACCESS_OP_READ_CHR:
            if (conn_handle != BLE_HS_CONN_HANDLE_NONE)
            {
                MODLOG_DFLT(INFO, "Characteristic read; conn_handle=%d attr_handle=%d\n",
                            conn_handle, attr_handle);
            }
            else
            {
                MODLOG_DFLT(INFO, "Characteristic read by NimBLE stack; attr_handle=%d\n",
                            attr_handle);
            }
            if (attr_handle == gatt_svr_chr_val_handle)
            {
                rc = os_mbuf_append(ctxt->om, &gatt_svr_chr_val, sizeof(gatt_svr_chr_val));
                return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
            }
            break;

        case BLE_GATT_ACCESS_OP_WRITE_CHR:
            if (conn_handle != BLE_HS_CONN_HANDLE_NONE)
            {
                MODLOG_DFLT(INFO, "Characteristic write; conn_handle=%d attr_handle=%d",
                            conn_handle, attr_handle);
            }
            else
            {
                MODLOG_DFLT(INFO, "Characteristic write by NimBLE stack; attr_handle=%d",
                            attr_handle);
            }
            if (attr_handle == gatt_svr_chr_val_handle)
            {
                rc = gatt_svr_write(ctxt->om, sizeof(gatt_svr_chr_val), sizeof(gatt_svr_chr_val),
                                    &gatt_svr_chr_val, nullptr);
                ble_gatts_chr_updated(attr_handle);
                MODLOG_DFLT(INFO, "Notification/Indication scheduled for all subscribed peers.\n");
                return rc;
            }
            break;

        default:
            break;
        }

        assert(0);
        return BLE_ATT_ERR_UNLIKELY;
    }

    void gatt_svr_register_cb(ble_gatt_register_ctxt *ctxt, void *arg)
    {
        char buf[BLE_UUID_STR_LEN];

        switch (ctxt->op)
        {
        case BLE_GATT_REGISTER_OP_SVC:
            MODLOG_DFLT(DEBUG, "registered service %s with handle=%d\n",
                        ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                        ctxt->svc.handle);
            break;

        case BLE_GATT_REGISTER_OP_CHR:
            MODLOG_DFLT(DEBUG, "registering characteristic %s with def_handle=%d val_handle=%d\n",
                        ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                        ctxt->chr.def_handle, ctxt->chr.val_handle);
            break;

        case BLE_GATT_REGISTER_OP_DSC:
            MODLOG_DFLT(DEBUG, "registering descriptor %s with handle=%d\n",
                        ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                        ctxt->dsc.handle);
            break;

        default:
            assert(0);
            break;
        }
    }

    int gatt_svr_init()
    {
        int rc;

        ble_svc_gap_init();
        ble_svc_gatt_init();
        ble_ans_init();

        rc = ble_gatts_count_cfg(gatt_svr_svcs);
        if (rc != 0)
        {
            return rc;
        }

        rc = ble_gatts_add_svcs(gatt_svr_svcs);
        if (rc != 0)
        {
            return rc;
        }

        return 0;
    }
}
