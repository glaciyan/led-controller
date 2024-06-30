#pragma once

bool notifyReady = false;
uint16_t notify_conn_handle;
uint16_t notify_attr_handle;

uint16_t color_characteristic_attr_handle;
int32_t gatt_color = 0;

int color_characteristic_access(uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt *ctxt, void *arg)
{
    switch (ctxt->op)
    {
        case BLE_GATT_ACCESS_OP_READ_CHR:
            MODLOG_DFLT(INFO, "Reading Color Characteristic conn_handle %d", conn_handle);
            if (attr_handle == color_characteristic_attr_handle)
            {
                int rc = os_mbuf_append(ctxt->om, &gatt_color, sizeof(gatt_color));
                return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
            }
            break;

        case BLE_GATT_ACCESS_OP_WRITE_CHR:
            if (attr_handle == color_characteristic_attr_handle)
            {
                uint16_t copiedLength = 0;
                int rc = ble_hs_mbuf_to_flat(ctxt->om, &gatt_color, sizeof(gatt_color), &copiedLength);
                return rc;
            }
            break;

        default:
            break;
    }

    assert(0);
    return BLE_ATT_ERR_UNLIKELY;
}
