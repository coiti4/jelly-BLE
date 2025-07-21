#include <zephyr/bluetooth/conn.h>
#include <zephyr/logging/log.h>
#include <dk_buttons_and_leds.h>
#include "led.h"
#include "ble_common.h"
#include "scanning.h"
#include "connection_manager.h"
#include "ble_callbacks.h"

LOG_MODULE_REGISTER(ble_callbacks, LOG_LEVEL_INF);

static void on_connected(struct bt_conn *conn, uint8_t err) {
    if (err) {
		LOG_ERR("Connection error %d", err);
        if (conn == get_parent_conn()) {
            set_parent_conn(NULL);
        }
        /* Restart scanning */
        start_scanning();
		return;
	}
    if (conn == get_parent_conn()) {
        set_parent_conn(conn);
        LOG_INF("Connected to parent");
    } else {
        LOG_ERR("New unknown connection");
    }

    /* struct bt_conn_info info;
	err = bt_conn_get_info(conn, &info);
	if (err) {
		LOG_ERR("bt_conn_get_info() returned %d", err);
		return;
	} */
/*  double connection_interval = info.le.interval*1.25; // in ms
	uint16_t supervision_timeout = info.le.timeout*10; // in ms
	LOG_INF("Connection parameters: interval %.2f ms, latency %d intervals, timeout %d ms", connection_interval, info.le.latency, supervision_timeout); */
}

static void on_disconnected(struct bt_conn *conn, uint8_t reason) {
    LOG_INF("Disconnected. Reason %d", reason);
    if (conn == get_parent_conn()) {
        set_parent_conn(NULL);
        LOG_INF("Parent disconnected");
        /* Restart scanning */
        start_scanning();
    } else {
        LOG_ERR("Disconnected unknown connection");
    }
}

/* static void on_le_param_updated(struct bt_conn *conn, uint16_t interval,
                                uint16_t latency, uint16_t timeout) {
    double connection_interval = interval*1.25; // in ms
	uint16_t supervision_timeout = timeout*10; // in ms
	LOG_INF("Connection parameters updated: interval %.2f ms, latency %d intervals, timeout %d ms", connection_interval, latency, supervision_timeout);
}

void on_recycled(void)
{
	advertising_start();
} */

void register_connection_callbacks(void) {
    static struct bt_conn_cb connection_callbacks = {
        .connected = on_connected,
        .disconnected = on_disconnected,
    };

    int err = bt_conn_cb_register(&connection_callbacks);
    if (err) {
        LOG_ERR("Connection callback register failed (err %d)", err);
    }
}
