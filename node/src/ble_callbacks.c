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
#include "power_debug.h"

static struct bt_uuid_128 discover_uuid = BT_UUID_INIT_128(0);
static struct bt_gatt_discover_params discover_params;
struct bt_gatt_subscribe_params subscribe_params;

LOG_MODULE_REGISTER(ble_callbacks, LOG_LEVEL_INF);


/* Notify handler (downward) */
static uint8_t notify_func(struct bt_conn *conn,
			   struct bt_gatt_subscribe_params *params,
			   const void *data, uint16_t length)
{
	if (length != sizeof(jrs_pkt_t)) {
        return BT_GATT_ITER_STOP;
    }

    /* MEASUREMENT: Receive message from parent */
    power_debug_pulse(1);  /* Short pulse on P1.01 for RX */

    jrs_pkt_t pkt;
    memcpy(&pkt, data, length);
    LOG_INF("Notify received: counter=%d", pkt.counter);

    if (pkt.counter == 0) {
        // This node has started the communication, compute RTT
        rtt_compute_time();
    } else {
        /* MEASUREMENT: Forward message downward (to child) */
        power_debug_start(DBG_FORWARD);

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

        power_debug_end(DBG_FORWARD);
    }

    return BT_GATT_ITER_CONTINUE;
}

static uint8_t discover_func(struct bt_conn *conn,
			     const struct bt_gatt_attr *attr,
			     struct bt_gatt_discover_params *params)
{
	int err;

	if (!attr) {
		LOG_INF("Discover complete");
		(void)memset(params, 0, sizeof(*params));
		return BT_GATT_ITER_STOP;
	}

	LOG_INF("[ATTRIBUTE] handle %u", attr->handle);

	if (!bt_uuid_cmp(discover_params.uuid, BT_UUID_JRS)) {
        LOG_INF("Found JRS service");
        set_jrs_service_handle(attr->handle);
		memcpy(&discover_uuid, BT_UUID_JRS_PKT, sizeof(discover_uuid));

		discover_params.uuid = &discover_uuid.uuid;
		discover_params.start_handle = attr->handle + 1;
		discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;

		err = bt_gatt_discover(conn, &discover_params);
		if (err) {
			LOG_INF("Discover failed (err %d)", err);
		}
	} else if (!bt_uuid_cmp(discover_params.uuid,
				BT_UUID_JRS_PKT)) {
        struct bt_gatt_chrc *chrc = (struct bt_gatt_chrc *)attr->user_data;
        set_jrs_char_decl_handle(attr->handle);
        set_jrs_value_handle(chrc->value_handle);
		discover_params.uuid = BT_UUID_GATT_CCC;
		discover_params.start_handle = attr->handle + 2;
		discover_params.type = BT_GATT_DISCOVER_DESCRIPTOR;

		subscribe_params.value_handle = bt_gatt_attr_value_handle(attr);

		err = bt_gatt_discover(conn, &discover_params);
		if (err) {
			LOG_INF("Discover failed (err %d)", err);
		}
	} else {
        /* CCC found, now subscribe */
        set_jrs_ccc_handle(attr->handle);
		subscribe_params.notify = notify_func;
		subscribe_params.value = BT_GATT_CCC_NOTIFY;
		subscribe_params.ccc_handle = attr->handle;

		err = bt_gatt_subscribe(conn, &subscribe_params);
		if (err && err != -EALREADY) {
			LOG_INF("Subscribe failed (err %d)", err);
		} else {
			LOG_INF("[SUBSCRIBED]");
		}

		return BT_GATT_ITER_STOP;
	}

	return BT_GATT_ITER_STOP;
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
        /* MEASUREMENT: Connecting to child - End of connection process */
        power_debug_pulse(0);  /* Pulse on P1.00 to mark child connection event */

        set_child_conn(conn);
        dk_set_led(CONNECTED_LED, 1);

        /* MEASUREMENT: Idle without advertising - P1.02 LOW when child connected */
        power_debug_clear(2);

    } else {
        /* MEASUREMENT: Connected to parent - End of connection process */
        power_debug_end(DBG_CONNECTING);
        power_debug_set(1);  /* P1.01 HIGH = connected to parent */

        // parent
        memcpy(&discover_uuid, BT_UUID_JRS, sizeof(discover_uuid));
		discover_params.uuid = &discover_uuid.uuid;
		discover_params.func = discover_func;
		discover_params.start_handle = BT_ATT_FIRST_ATTRIBUTE_HANDLE;
		discover_params.end_handle = BT_ATT_LAST_ATTRIBUTE_HANDLE;
		discover_params.type = BT_GATT_DISCOVER_PRIMARY;

		err = bt_gatt_discover(get_parent_conn(), &discover_params);
		if (err) {
			LOG_INF("Discover failed(err %d)", err);
			return;
		}
    }
}

static void on_disconnected(struct bt_conn *conn, uint8_t reason) {
    int err;
    LOG_INF("Disconnected. Reason %d", reason);
    if (conn == get_parent_conn()) {
        LOG_INF("Parent disconnected");
        set_parent_conn(NULL);

        /* MEASUREMENT: Disconnected from parent, return to scanning */
        power_debug_clear(1);  /* P1.01 LOW = not connected */

        /* Restart scanning */
        err = start_scanning();
        if (err) {
            LOG_ERR("Failed to restart scanning (err %d)", err);
        }
    } else if (conn == get_child_conn()) {
        set_child_conn(NULL);
        dk_set_led(CONNECTED_LED, 0);
        LOG_INF("Child disconnected");

        /* MEASUREMENT: Disconnected from child, return to advertising */
        power_debug_set(2);  /* P1.02 HIGH = advertising active */

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
