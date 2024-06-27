#pragma once

uint16_t color_characteristic_attr_handle;
int color_characteristic_access(uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt *ctxt, void *arg)
{
    switch (ctxt->op)
    {
        case BLE_GATT_ACCESS_OP_READ_CHR:
            if (attr_handle == color_characteristic_attr_handle)
            {
                // rc = os_mbuf_append(ctxt->om, &gatt_svr_chr_val, sizeof(gatt_svr_chr_val));
                // return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
            }
            break;

        case BLE_GATT_ACCESS_OP_WRITE_CHR:
            if (attr_handle == color_characteristic_attr_handle)
            {
                //     rc = gatt_svr_write(ctxt->om, sizeof(gatt_svr_chr_val), sizeof(gatt_svr_chr_val),
                //                         &gatt_svr_chr_val, nullptr);
                //     ble_gatts_chr_updated(attr_handle);
                //     MODLOG_DFLT(INFO, "Notification/Indication scheduled for all subscribed peers.\n");
                //     return rc;
            }
            break;

        default:
            break;
    }

    assert(0);
    return BLE_ATT_ERR_UNLIKELY;
}
