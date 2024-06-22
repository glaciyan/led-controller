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
#include "ble_values.h"
#include "ble_my_service.h"

extern "C" void ble_ans_init();

namespace ble
{
    constexpr ble_uuid128_t gatt_svr_svc_uuid = UUID128("88feb8853fa6c610010abf2eee3b7de3");

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
                    .access_cb = my_service::gatt_svc_access,
                    .flags = perm::EREAD | perm::EWRITE,
                    .val_handle = &my_service::gatt_svr_chr_val_handle,
                },
                {
                    0,
                }},
        },

        {
            0,
        },
    };

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
