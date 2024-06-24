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

extern "C" void ble_ans_init();


namespace ble
{
    int gatt_svr_init(const ble_gatt_svc_def *gatt_services)
    {
        int rc;

        ble_svc_gap_init();
        ble_svc_gatt_init();
        ble_ans_init();

        rc = ble_gatts_count_cfg(gatt_services);
        if (rc != 0)
        {
            return rc;
        }

        rc = ble_gatts_add_svcs(gatt_services);
        if (rc != 0)
        {
            return rc;
        }

        return 0;
    }
}
