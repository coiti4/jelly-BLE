#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/logging/log.h>
#include "ble_common.h"
#include "scanning.h"
#include "led.h"

LOG_MODULE_REGISTER(node_main, LOG_LEVEL_INF);

int main(void)
{
    LOG_INF("Starting Jelly BLE Node");

    if (led_init()) {
        LOG_ERR("Failed to initiate the LED module.");
        return -1;
    }

    ble_init();

    LOG_INF("Starting node scanning...");
    start_scanning();

    led_blink_loop(1000);

    return 0;
}
