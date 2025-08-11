#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/logging/log.h>
#include "ble_common.h"
#include "scanning.h"
#include "led.h"
#include "ble_callbacks.h"

LOG_MODULE_REGISTER(node_main, LOG_LEVEL_INF);

int main(void)
{
    LOG_INF("Starting Jelly BLE Node");

    if (led_init()) {
        LOG_ERR("Failed to initiate the LED module.");
        return -1;
    }

    register_connection_callbacks();

    ble_init();

    LOG_INF("Starting node scanning...");
    start_scanning();

    LOG_INF("Starting advertising...");
    ble_advertising_init();
    ble_advertising_start();

    //led_blink_loop(1000);

    return 0;
}
