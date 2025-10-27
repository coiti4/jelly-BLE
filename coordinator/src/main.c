/*
 * MIT License 2025
 * Author: Agustín Coitinho
 * Jelly BLE is a project developed within the course
 * "Tecnologías para la Internet de las Cosas"
 * Facultad de Ingeniería - Universidad de la República, Uruguay
 */
#include "ble_common.h"
#include "ble_callbacks.h"
#include "button.h"
#include "led.h"
#include "jelly_rtt_service.h"
#include <bluetooth/services/lbs.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(jelly_coordinator_main, LOG_LEVEL_INF);

static void button_changed(uint32_t button_state, uint32_t has_changed)
{
	int err;
	bool user_button_changed = (has_changed & USER_BUTTON) ? true : false;
	bool user_button_pressed = (button_state & USER_BUTTON) ? true : false;
	if (user_button_changed) {
		LOG_INF("Button %s", (user_button_pressed ? "pressed" : "released"));

		err = bt_lbs_send_button_state(user_button_pressed);
		if (err) {
			LOG_ERR("Couldn't send notification. (err: %d)", err);
		}
	}
}

/* RTT callback */
static void coordinator_rx_cb(struct bt_conn *conn, const jrs_pkt_t *pkt)
{
    int err = bt_jrs_send(conn, pkt);
    if (err) {
        LOG_ERR("Coordinator failed to echo: counter=%d, err=%d", pkt->counter, err);
    } else {
        LOG_INF("Coordinator echoed: counter=%d to conn %p", pkt->counter, (void *)conn);
    }
}

int main(void)
{
	LOG_INF("Starting Jelly BLE Coordinator");

	if (led_init()) {
        LOG_ERR("Failed to initiate the LED module.");
        return -1;
    }

    if (init_button(button_changed)) {
        LOG_ERR("Failed to initiate the BUTTON module.");
        return -1;
    }

    register_connection_callbacks();

	//ble_init_and_start_advertising();
    ble_init();
	bt_jrs_init(coordinator_rx_cb);
    ble_advertising_init();
    ble_advertising_start();

	led_blink_loop(1000);

    return 0;
}
