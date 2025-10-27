/*
 * MIT License 2025
 * Author: Agustín Coitinho
 * Jelly BLE is a project developed within the course
 * "Tecnologías para la Internet de las Cosas"
 * Facultad de Ingeniería - Universidad de la República, Uruguay
 */

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/logging/log.h>
#include "jelly_rtt_service.h"

LOG_MODULE_REGISTER(jrs, LOG_LEVEL_INF);

/* Application callback */
static jrs_rx_cb_t app_rx_cb;

/* ----------------------------------------------------------------------
 * Characteristic write handler (upward)
 * --------------------------------------------------------------------*/
static ssize_t write_pkt(struct bt_conn *conn,
                         const struct bt_gatt_attr *attr,
                         const void *buf, uint16_t len,
                         uint16_t offset, uint8_t flags)
{
    LOG_DBG("Attribute write, handle: %u, conn: %p", attr->handle, (void *)conn);
    if (len != sizeof(jrs_pkt_t)) {
        LOG_WRN("Invalid packet size %d (expected %d)", len, sizeof(jrs_pkt_t));
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    }

    jrs_pkt_t pkt;
    memcpy(&pkt, buf, len);
    LOG_INF("Packet received: counter=%d", pkt.counter);

    if (app_rx_cb) {
        app_rx_cb(conn, &pkt);
    }

    return len;
}

/* ----------------------------------------------------------------------
 * CCC subscription handler
 * --------------------------------------------------------------------*/
static void jrs_subscription(const struct bt_gatt_attr *attr, uint16_t value)
{
    if (value == BT_GATT_CCC_NOTIFY) {
        LOG_INF("Client subscribed\n");
    } else if (value == 0) {
        LOG_INF("Client unsubscribed\n");
    }
}

/* ----------------------------------------------------------------------
 * JRS service definition
 * --------------------------------------------------------------------*/
BT_GATT_SERVICE_DEFINE(jrs_svc,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_JRS),
    BT_GATT_CHARACTERISTIC(BT_UUID_JRS_PKT,
                           BT_GATT_CHRC_WRITE_WITHOUT_RESP | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_WRITE,
                           NULL, write_pkt, NULL),
    BT_GATT_CCC(jrs_subscription, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);

/* ----------------------------------------------------------------------
 * API implementation
 * --------------------------------------------------------------------*/
int bt_jrs_init(jrs_rx_cb_t cb)
{
    app_rx_cb = cb;
    LOG_INF("Jelly RTT Service initialized");
    return 0;
}

int bt_jrs_send(struct bt_conn *conn, const jrs_pkt_t *pkt)
{
    int err = bt_gatt_notify(conn, &jrs_svc.attrs[2], pkt, sizeof(jrs_pkt_t));
    if (err) {
        LOG_ERR("Failed to send notification (err %d)", err);
    } else {
        LOG_INF("Coordinator sent notification: counter=%d to conn %p", pkt->counter, (void *)conn);
    }
    return err;
}

uint16_t get_jrs_value_handle(void)
{
    return jrs_svc.attrs[2].handle;
}

uint16_t get_jrs_ccc_handle(void)
{
    return jrs_svc.attrs[3].handle;
}

int jrs_notify(struct bt_conn *conn, const jrs_pkt_t *pkt)
{
    return bt_gatt_notify(conn, &jrs_svc.attrs[2], pkt, sizeof(jrs_pkt_t));
}

