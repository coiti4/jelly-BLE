#include <zephyr/bluetooth/conn.h>
#include <zephyr/logging/log.h>
#include <dk_buttons_and_leds.h>
#include "led.h"
#include "ble_common.h"
#include "ble_callbacks.h"

LOG_MODULE_REGISTER(ble_callbacks, LOG_LEVEL_INF);

static uint8_t active_connections = 0; // Counter for active connections
#define MAX_CONN CONFIG_BT_MAX_CONN

static struct bt_conn *conns[MAX_CONN] = { 0 };

static void restart_advertising_if_needed(void) {
    if (active_connections < MAX_CONN) {
        LOG_INF("Advertising again (active: %d, max: %d)", active_connections, MAX_CONN);
        ble_advertising_start();
    } else {
        LOG_INF("Maximum connections reached: %d", MAX_CONN);
    }
}

static void on_connected(struct bt_conn *conn, uint8_t err) {
    if (err) {
		LOG_ERR("Connection error %d", err);
		return;
	}

	if (active_connections < MAX_CONN) {
        conns[active_connections++] = bt_conn_ref(conn);  // guardar referencia
        LOG_INF("Connected");
	    dk_set_led(active_connections, 1);
    } else {
        LOG_WRN("Too many connections already established!");
    }

	/* STEP 1.1 - Declare a structure to store the connection parameters */
	struct bt_conn_info info;
	err = bt_conn_get_info(conn, &info);
	if (err) {
		LOG_ERR("bt_conn_get_info() returned %d", err);
		return;
	}
	/* STEP 1.2 - Add the connection parameters to your log */
	double connection_interval = info.le.interval*1.25; // in ms
	uint16_t supervision_timeout = info.le.timeout*10; // in ms
	LOG_INF("Connection parameters: interval %.2f ms, latency %d intervals, timeout %d ms", connection_interval, info.le.latency, supervision_timeout);

    restart_advertising_if_needed();
	//ble_advertising_start();
	/* update_phy(my_conn);
	k_sleep(K_MSEC(1000)); // Delay added to avoid link layer collisions.
	update_data_length(my_conn);
	update_mtu(my_conn); */
}

static void on_disconnected(struct bt_conn *conn, uint8_t reason) {
    LOG_INF("Disconnected. Reason %d", reason);

	// Search and delete the connection
    for (int i = 0; i < MAX_CONN; i++) {
        if (conns[i] == conn) {
            bt_conn_unref(conns[i]);
            conns[i] = NULL;
            dk_set_led(active_connections--, 0);
            if (active_connections == MAX_CONN - 1) {
                restart_advertising_if_needed();
            }
            break;
        }
    }
}

static void on_le_param_updated(struct bt_conn *conn, uint16_t interval,
                                uint16_t latency, uint16_t timeout) {
    double connection_interval = interval*1.25; // in ms
	uint16_t supervision_timeout = timeout*10; // in ms
	LOG_INF("Connection parameters updated: interval %.2f ms, latency %d intervals, timeout %d ms", connection_interval, latency, supervision_timeout);
}

static void on_recycled(void)
{
	//restart_advertising_if_needed();
}

/* static void on_le_phy_updated(struct bt_conn *conn, struct bt_conn_le_phy_info *param) {
    LOG_INF("LE PHY updated");
} */

/* static void on_le_data_len_updated(struct bt_conn *conn,
                                   struct bt_conn_le_data_len_info *info) {
    LOG_INF("Data length updated");
} */

void register_connection_callbacks(void) {
    static struct bt_conn_cb connection_callbacks = {
        .connected = on_connected,
        .disconnected = on_disconnected,
        .recycled = on_recycled,
        .le_param_updated = on_le_param_updated,
        /* .le_phy_updated = on_le_phy_updated,
        .le_data_len_updated = on_le_data_len_updated, */
    };

    int err = bt_conn_cb_register(&connection_callbacks); // REVISAR cambiar a macro
    if (err) {
        LOG_ERR("Connection callback register failed (err %d)", err);
    }
}
