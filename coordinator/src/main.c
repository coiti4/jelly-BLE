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
#include "power_debug.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(jelly_coordinator_main, LOG_LEVEL_INF);

static void button_changed(uint32_t button_state, uint32_t has_changed)
{
	bool user_button_changed = (has_changed & USER_BUTTON) ? true : false;
	bool user_button_pressed = (button_state & USER_BUTTON) ? true : false;
	if (user_button_changed) {
		LOG_INF("Button %s", (user_button_pressed ? "pressed" : "released"));
	}
}

/* RTT callback */
static void coordinator_rx_cb(struct bt_conn *conn, const jrs_pkt_t *pkt)
{
    /* MEASUREMENT: Forward a message (echo) */
    power_debug_start(DBG_FORWARD);

    int err = bt_jrs_send(conn, pkt);
    if (err) {
        LOG_ERR("Coordinator failed to echo: counter=%d, err=%d", pkt->counter, err);
    } else {
        LOG_INF("Coordinator echoed: counter=%d to conn %p", pkt->counter, (void *)conn);
    }

    power_debug_end(DBG_FORWARD);
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

    /* Initialize debug module for current measurements */
    if (power_debug_init()) {
        LOG_ERR("Failed to initiate Power Debug module.");
        return -1;
    }

    register_connection_callbacks();

    ble_init();
	bt_jrs_init(coordinator_rx_cb);

    ble_advertising_init();

    /* MEASUREMENT: Start advertising (first time) */
    power_debug_start(DBG_ADVERTISING);
    ble_advertising_start();

	/* MEASUREMENT: Idle/steady state - The blink loop represents coordinator idle state */
	led_blink_loop(1000);

    return 0;
}
