/*
 * MIT License 2025
 * Author: Agustín Coitinho
 * Jelly BLE is a project developed within the course
 * "Tecnologías para la Internet de las Cosas"
 * Facultad de Ingeniería - Universidad de la República, Uruguay
 */

#include "power_debug.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(power_debug, LOG_LEVEL_INF);

static const struct device *gpio_dev;
static bool initialized = false;

int power_debug_init(void)
{
	int ret;

	gpio_dev = DEVICE_DT_GET(DEBUG_PORT);
	if (!device_is_ready(gpio_dev)) {
		LOG_ERR("GPIO device not ready");
		return -ENODEV;
	}

	/* Configure all debug pins as outputs in low state */
	for (int i = 0; i < 4; i++) {
		ret = gpio_pin_configure(gpio_dev, DEBUG_PIN_0 + i, GPIO_OUTPUT_INACTIVE);
		if (ret < 0) {
			LOG_ERR("Failed to configure debug pin %d: %d", i, ret);
			return ret;
		}
	}

	initialized = true;
	LOG_INF("Power debug GPIO initialized (P1.00-P1.03)");
	return 0;
}

void power_debug_set(uint8_t pin)
{
	if (!initialized || pin > 3) {
		return;
	}
	gpio_pin_set(gpio_dev, DEBUG_PIN_0 + pin, 1);
}

void power_debug_clear(uint8_t pin)
{
	if (!initialized || pin > 3) {
		return;
	}
	gpio_pin_set(gpio_dev, DEBUG_PIN_0 + pin, 0);
}

void power_debug_toggle(uint8_t pin)
{
	if (!initialized || pin > 3) {
		return;
	}
	gpio_pin_toggle(gpio_dev, DEBUG_PIN_0 + pin);
}

void power_debug_set_pattern(uint8_t mask)
{
	if (!initialized) {
		return;
	}

	for (int i = 0; i < 4; i++) {
		if (mask & (1 << i)) {
			gpio_pin_set(gpio_dev, DEBUG_PIN_0 + i, 1);
		} else {
			gpio_pin_set(gpio_dev, DEBUG_PIN_0 + i, 0);
		}
	}
}

void power_debug_start(debug_state_t state)
{
	if (!initialized) {
		return;
	}

	switch (state) {
	case DBG_SCANNING:
		power_debug_set(0);  /* P1.00 = HIGH during scanning */
		break;
	case DBG_CONNECTING:
		power_debug_set(0);  /* P1.00 = HIGH during connecting */
		break;
	case DBG_CONNECTED:
		power_debug_set(1);  /* P1.01 = HIGH when connected */
		break;
	case DBG_ADVERTISING:
		power_debug_set(2);  /* P1.02 = HIGH during advertising */
		break;
	case DBG_MESSAGE_TX:
	case DBG_MESSAGE_RX:
		power_debug_set(1);  /* P1.01 = Pulse during messages */
		break;
	case DBG_FORWARD:
		power_debug_set(3);  /* P1.03 = HIGH during forward */
		break;
	case DBG_IDLE:
	default:
		/* No action for idle */
		break;
	}
}

void power_debug_end(debug_state_t state)
{
	if (!initialized) {
		return;
	}

	switch (state) {
	case DBG_SCANNING:
	case DBG_CONNECTING:
		power_debug_clear(0);
		break;
	case DBG_CONNECTED:
		power_debug_clear(1);
		break;
	case DBG_ADVERTISING:
		power_debug_clear(2);
		break;
	case DBG_MESSAGE_TX:
	case DBG_MESSAGE_RX:
		power_debug_clear(1);
		break;
	case DBG_FORWARD:
		power_debug_clear(3);
		break;
	case DBG_IDLE:
	default:
		break;
	}
}

void power_debug_pulse(uint8_t pin)
{
	if (!initialized || pin > 3) {
		return;
	}

	/* Short pulse: set -> small delay -> clear */
	gpio_pin_set(gpio_dev, DEBUG_PIN_0 + pin, 1);
	k_busy_wait(10);  /* 10 microseconds */
	gpio_pin_set(gpio_dev, DEBUG_PIN_0 + pin, 0);
}
