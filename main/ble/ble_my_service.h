#pragma once

#include "services/gatt/ble_svc_gatt.h"

namespace my_service
{
    uint8_t gatt_svr_chr_val;
    uint16_t gatt_svr_chr_val_handle;

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
}