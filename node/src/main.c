/*
 * MIT License 2025
 * Author: Agustín Coitinho
 * Jelly BLE is a project developed within the course
 * "Tecnologías para la Internet de las Cosas"
 * Facultad de Ingeniería - Universidad de la República, Uruguay
 */
#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include "ble_common.h"
#include "scanning.h"
#include "led.h"
#include "ble_callbacks.h"
#include "jelly_rtt_service.h"
#include "button.h"
#include "connection_manager.h"
#include "rtt_manager.h"
#include "power_debug.h"

LOG_MODULE_REGISTER(node_main, LOG_LEVEL_INF);

/* Write app callback */
static void node_rx_cb(struct bt_conn *conn, const jrs_pkt_t *pkt)
{
    /* MEASUREMENT: Forward message upward (received from child) */
    power_debug_start(DBG_FORWARD);

    struct bt_conn *parent_conn = get_parent_conn();
    if (parent_conn) {
        // Intermediate node: Increment and forward upward
        jrs_pkt_t new_pkt = { .counter = pkt->counter + 1 };
        int err = bt_gatt_write_without_response(parent_conn, get_jrs_value_handle(), &new_pkt, sizeof(new_pkt), false);
        if (err) {
            LOG_ERR("Failed to forward upward (err %d)", err);
        } else {
            LOG_INF("Forwarded upward: counter=%d", new_pkt.counter);
        }
    }

    power_debug_end(DBG_FORWARD);
}

/* Button handler: called by the node that starts packet transfer */
static void button_handler(uint32_t button_state, uint32_t has_changed)
{
    if (has_changed & USER_BUTTON && !(button_state & USER_BUTTON)) {
        /* MEASUREMENT: Send a message (initiated by button) */
        power_debug_start(DBG_MESSAGE_TX);

        // Store timestamp
        if (rtt_store_timestamp()) {
            LOG_ERR("Failed to store timestamp");
            power_debug_end(DBG_MESSAGE_TX);
            return;
        }

        // Sends counter=0
        jrs_pkt_t pkt = { .counter = 0 };
        struct bt_conn *parent_conn = get_parent_conn();
        if (parent_conn) {
            int err = bt_gatt_write_without_response(parent_conn, get_jrs_value_handle(), &pkt, sizeof(pkt), false);
            if (err) {
                LOG_ERR("Failed to send upward (err %d)", err);
            } else {
                LOG_INF("Sent upward from button: counter=%d", pkt.counter);
            }
        }

        power_debug_end(DBG_MESSAGE_TX);
    }
}

int main(void)
{
    LOG_INF("Starting Jelly BLE Node");

    if (led_init()) {
        LOG_ERR("Failed to initiate the LED module.");
        return -1;
    }

    /* Initialize debug module for current measurements */
    if (power_debug_init()) {
        LOG_ERR("Failed to initiate Power Debug module.");
        return -1;
    }

    register_peripheral_connection_callbacks();

    rtt_manager_init();

    ble_init();
    bt_jrs_init(node_rx_cb);

    register_central_connection_callbacks();
    LOG_INF("Starting node scanning...");
    /* MEASUREMENT: Start scanning - P1.00 is activated in start_scanning() */
    start_scanning();

    LOG_INF("Starting advertising...");
    ble_advertising_init();
    /* MEASUREMENT: Start advertising - P1.02 HIGH when advertising starts */
    power_debug_start(DBG_ADVERTISING);
    ble_advertising_start();

    if (init_button(button_handler)) {
        LOG_ERR("Failed to initiate the button module.");
        return -1;
    }

    //led_blink_loop(1000);

    return 0;
}
