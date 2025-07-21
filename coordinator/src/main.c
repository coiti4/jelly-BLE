#include "ble_common.h"
#include "ble_callbacks.h"
#include "button.h"
#include "led.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(jelly_coordinator_main, LOG_LEVEL_INF);

int main(void)
{
	LOG_INF("Starting Jelly BLE Coordinator");

	if (led_init()) {
        LOG_ERR("Failed to initiate the LED module.");
        return -1;
    }

    if (init_button()) {
        LOG_ERR("Failed to initiate the BUTTON module.");
        return -1;
    }

    register_connection_callbacks();

	//ble_init_and_start_advertising();
    ble_init();
    ble_advertising_init();
    ble_advertising_start();

	led_blink_loop(1000);

    return 0;
}
