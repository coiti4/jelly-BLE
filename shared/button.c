/*
 * MIT License 2025
 * Author: Agustín Coitinho
 * Jelly BLE is a project developed within the course
 * "Tecnologías para la Internet de las Cosas"
 * Facultad de Ingeniería - Universidad de la República, Uruguay
 */
#include "button.h"
#include <zephyr/logging/log.h>
#include <dk_buttons_and_leds.h>

LOG_MODULE_REGISTER(button, LOG_LEVEL_INF);

int init_button(button_handler_t button_handler)
{
	int err;

	err = dk_buttons_init(button_handler);
	if (err) {
		LOG_ERR("Cannot init buttons (err: %d)", err);
	}

	return err;
}
