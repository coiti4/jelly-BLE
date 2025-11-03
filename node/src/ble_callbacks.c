/*
 * MIT License 2025
 * Author: Agustín Coitinho
 * Jelly BLE is a project developed within the course
 * "Tecnologías para la Internet de las Cosas"
 * Facultad de Ingeniería - Universidad de la República, Uruguay
 */
#include <zephyr/bluetooth/conn.h>
#include <zephyr/logging/log.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/bluetooth/gatt.h>
#include "led.h"
#include "ble_common.h"
#include "scanning.h"
#include "connection_manager.h"
#include "ble_callbacks.h"
#include "jelly_rtt_service.h"
#include "rtt_manager.h"

struct bt_gatt_subscribe_params subscribe_params;

LOG_MODULE_REGISTER(ble_callbacks, LOG_LEVEL_INF);


/* Notify handler (downward) */
static uint8_t notify_func(struct bt_conn *conn,
			   struct bt_gatt_subscribe_params *params,
			   const void *data, uint16_t length)
{
	if (length != sizeof(jrs_pkt_t)) return;

    jrs_pkt_t pkt;
    memcpy(&pkt, data, length);
    LOG_INF("Notify received: counter=%d", pkt.counter);

    if (pkt.counter == 0) {
        // This node has started the communication, compute RTT
        rtt_compute_time();
    } else {
        // Forward downward
        pkt.counter--;
        struct bt_conn *child_conn = get_child_conn();
        if (child_conn) {
            int err = jrs_notify(child_conn, &pkt);
            if (err) {
                LOG_ERR("Failed to notify downward (err %d)", err);
            } else {
                LOG_INF("Forwarded downward: counter=%d", pkt.counter);
            }
        }
    }
}

static void on_connected(struct bt_conn *conn, uint8_t err) {
    if (err) { // revisar
		LOG_ERR("Connection error %d", err);
        if (conn == get_parent_conn()) {
            set_parent_conn(NULL);
        } else if (conn == get_child_conn()) {
            set_child_conn(NULL);
        }
		return;
	}
    if (conn != get_parent_conn()) {
        set_child_conn(conn);
        dk_set_led(CONNECTED_LED, 1);

    } else {
        // parent
        /* Subscribe to RTT characteristic */
        subscribe_params.notify = notify_func;
        subscribe_params.value = BT_GATT_CCC_NOTIFY;
        subscribe_params.value_handle = get_jrs_value_handle();
        subscribe_params.ccc_handle = get_jrs_ccc_handle();

        int err = bt_gatt_subscribe(get_parent_conn(), &subscribe_params);
        if (err && err != -EALREADY) {
            LOG_INF("Subscribe failed (err %d)\n", err);
        } else {
            LOG_INF("[SUBSCRIBED]\n");
        }
    }
    /* Stop advertising once connected */
}

static void on_disconnected(struct bt_conn *conn, uint8_t reason) {
    int err;
    LOG_INF("Disconnected. Reason %d", reason);
    if (conn == get_parent_conn()) {
        LOG_INF("Parent disconnected");
        set_parent_conn(NULL);
        /* Restart scanning */
        err = start_scanning();
        if (err) {
            LOG_ERR("Failed to restart scanning (err %d)", err);
        }
    } else if (conn == get_child_conn()) {
        set_child_conn(NULL);
        dk_set_led(CONNECTED_LED, 0);
        LOG_INF("Child disconnected");
        /* Restart advertising */
        ble_advertising_start();
    } else {
        LOG_ERR("Disconnected unknown connection");
    }
}

/* static void on_le_param_updated(struct bt_conn *conn, uint16_t interval,
                                uint16_t latency, uint16_t timeout) {
    double connection_interval = interval*1.25; // in ms
	uint16_t supervision_timeout = timeout*10; // in ms
	LOG_INF("Connection parameters updated: interval %.2f ms, latency %d intervals, timeout %d ms", connection_interval, latency, supervision_timeout);
}*/

void on_recycled(void)
{
	//ble_advertising_start();
}

void register_peripheral_connection_callbacks(void)
{
    static struct bt_conn_cb connection_callbacks = {
        .connected = on_connected,
        .disconnected = on_disconnected,
        .recycled = on_recycled,
    };

    int err = bt_conn_cb_register(&connection_callbacks);
    if (err) {
        LOG_ERR("Connection callback register failed (err %d)", err);
    }
}
