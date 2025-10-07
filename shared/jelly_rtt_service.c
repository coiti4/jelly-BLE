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
#include "connection_manager.h"

LOG_MODULE_REGISTER(jrs, LOG_LEVEL_INF);

/* Application callback */
static jrs_rx_cb_t app_rx_cb;

/* RTT storage */
#define K_MAX_PENDING 8
static struct {
    uint64_t timestamp; /* Local timestamp for RTT */
} pending_rtt[K_MAX_PENDING];
static struct k_mutex rtt_mutex;

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

/* Notify handler (downward) */
static void notify_pkt(struct bt_conn *conn, uint16_t handle, const void *data, uint16_t len)
{
    if (len != sizeof(jrs_pkt_t)) return;

    jrs_pkt_t pkt;
    memcpy(&pkt, data, len);
    LOG_INF("Notify received: counter=%d", pkt.counter);

    if (pkt.counter == 0) {
        // Origin: Compute RTT
        k_mutex_lock(&rtt_mutex, K_FOREVER);
        for (int i = 0; i < K_MAX_PENDING; i++) {
            if (pending_rtt[i].timestamp > 0) {
                uint64_t rtt = k_uptime_get() - pending_rtt[i].timestamp;
                LOG_INF("RTT calculated: %llu ms", rtt);
                pending_rtt[i].timestamp = 0; // Limpia slot
                break;
            }
        }
        k_mutex_unlock(&rtt_mutex);
    } else {
        // Forward downward
        pkt.counter--;
        struct bt_conn *child_conn = get_child_conn();
        if (child_conn) {
            /* int err = bt_gatt_notify(child_conn, &jrs_svc.attrs[1], &pkt, sizeof(pkt));
            if (err) {
                LOG_ERR("Failed to notify downward (err %d)", err);
            } else {
                LOG_INF("Forwarded downward: counter=%d", pkt.counter);
            } */
        }
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
    BT_GATT_CCC(notify_pkt, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
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
    return bt_gatt_notify(conn, &jrs_svc.attrs[1], pkt, sizeof(jrs_pkt_t));
}

int jrs_store_timestamp(void)
{
    k_mutex_lock(&rtt_mutex, K_FOREVER);
    for (int i = 0; i < K_MAX_PENDING; i++) {
        if (pending_rtt[i].timestamp == 0) {
            pending_rtt[i].timestamp = k_uptime_get();
            k_mutex_unlock(&rtt_mutex);
            return 0;
        }
    }
    k_mutex_unlock(&rtt_mutex);
    LOG_WRN("No free RTT slots");
    return -ENOMEM;
} 

uint16_t get_jrs_pkt_handle(void)
{
    return jrs_svc.attrs[2].handle;
}
