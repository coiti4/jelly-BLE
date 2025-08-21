/*
 * MIT License 2025
 * Author: Agustín Coitinho
 * Jelly BLE is a project developed within the course
 * "Tecnologías para la Internet de las Cosas"
 * Facultad de Ingeniería - Universidad de la República, Uruguay
 */
#include "button.h"
#include <zephyr/logging/log.h>
#include <bluetooth/services/lbs.h>
#include <dk_buttons_and_leds.h>

LOG_MODULE_REGISTER(button, LOG_LEVEL_INF);

int init_button(void)
{
	int err;

	err = dk_buttons_init(button_changed);
	if (err) {
		LOG_ERR("Cannot init buttons (err: %d)", err);
	}

	return err;
}

void button_changed(uint32_t button_state, uint32_t has_changed)
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
