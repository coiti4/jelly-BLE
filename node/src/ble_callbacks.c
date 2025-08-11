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
    }
    /* Stop advertising once connected */
}

static void on_disconnected(struct bt_conn *conn, uint8_t reason) {
    LOG_INF("Disconnected. Reason %d", reason);
    if (conn == get_parent_conn()) {
        set_parent_conn(NULL);
        LOG_INF("Parent disconnected");
        /* Restart scanning */
        start_scanning();
    } else if (conn == get_child_conn()) {
        set_child_conn(NULL);
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

void register_connection_callbacks(void) {
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
