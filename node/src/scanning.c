/*
 * MIT License 2025
 * Author: Agustín Coitinho
 * Jelly BLE is a project developed within the course
 * "Tecnologías para la Internet de las Cosas"
 * Facultad de Ingeniería - Universidad de la República, Uruguay
 */
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/logging/log.h>
#include <bluetooth/scan.h>
#include "scanning.h"
#include "connection_manager.h"

LOG_MODULE_REGISTER(scanning, LOG_LEVEL_INF);

static uint8_t matchs_nb = 0;

/* Callback for filter match */
static void scan_filter_match(struct bt_scan_device_info *device_info,
                            struct bt_scan_filter_match *filter_match,
                            bool connectable)
{
    if (filter_match->name.match) {
        char addr_str[BT_ADDR_LE_STR_LEN];
        bt_addr_le_to_str(device_info->recv_info->addr, addr_str, sizeof(addr_str));

        /* Connection is established automatically if connect_if_match is enabled */
        if (strcmp(filter_match->name.name, "Jelly BLE Coordinator") == 0) {
            matchs_nb = 0;
            LOG_INF("Coordinator found: %s (name: %s)", addr_str, filter_match->name.name);
            connect_to_device(device_info->recv_info->addr, device_info->conn_param); 
        } else if (strcmp(filter_match->name.name, "Jelly BLE Node") == 0) {
            matchs_nb++;
            LOG_INF("Node found: %s (name: %s)", addr_str, filter_match->name.name);
            if (matchs_nb == COORDINATOR_RETRY_TIMES) {
                // connect to device
                matchs_nb = 0;
                connect_to_device(device_info->recv_info->addr, device_info->conn_param); 
            }
        } else {
            LOG_WRN("Unknown device found: %s (name: %s)", addr_str, filter_match->name.name);
        }
    }
}

/* Callback for filter no-match */
static void scan_filter_no_match(struct bt_scan_device_info *device_info,
                                bool connectable)
{

}

/* Callback for connection errors */
static void scan_connecting_error(struct bt_scan_device_info *device_info)
{
    LOG_ERR("Connection failed");
    /* Restart scanning */
    bt_scan_start(BT_SCAN_TYPE_SCAN_PASSIVE);
}

/* Callback for ongoing connection */
static void scan_connecting(struct bt_scan_device_info *device_info, struct bt_conn *conn)
{
    set_parent_conn(conn); 
}

/* Scanning callback structure */
BT_SCAN_CB_INIT(scan_cb, scan_filter_match, scan_filter_no_match, 
    scan_connecting_error, scan_connecting);

/* Initialize and start scanning for the parent */
int start_scanning(void)
{
    int err;

    /* Initialize scanning module */
    struct bt_scan_init_param scan_init = {
        .scan_param = NULL, /* Use default parameters */
        .connect_if_match = false, /* Do not connect automatically on filter match */
        .conn_param = BT_LE_CONN_PARAM_DEFAULT,
    };
    bt_scan_init(&scan_init);

    /* Register scanning callbacks */
    bt_scan_cb_register(&scan_cb);

    bt_scan_filter_remove_all();

    /* Add filter for parent name */
    err = bt_scan_filter_add(BT_SCAN_FILTER_TYPE_NAME, "Jelly BLE Coordinator");
    if (err) {
        LOG_ERR("Failed to add name filter (err %d)", err);
        return err;
    }

    /* Add filter for node name */
    err = bt_scan_filter_add(BT_SCAN_FILTER_TYPE_NAME, "Jelly BLE Node");
    if (err) {
        LOG_ERR("Failed to add name filter (err %d)", err);
        return err;
    }

    /* Enable name filter */
    err = bt_scan_filter_enable(BT_SCAN_NAME_FILTER, false);
    if (err) {
        LOG_ERR("Failed to enable filter (err %d)", err);
        return err;
    }

    /* Start scanning */
    err = bt_scan_start(BT_SCAN_TYPE_SCAN_PASSIVE);
    if (err) {
        LOG_ERR("Failed to start scanning (err %d)", err);
        return err;
    }

    LOG_INF("Coordinator scanning started");
    return 0;
}

void connect_to_device(const bt_addr_le_t *addr, const struct bt_le_conn_param *conn_param)
{
    int err;
    struct bt_conn *conn = NULL;

    err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN, conn_param, &conn);
    if (err) {
        LOG_ERR("Failed to create connection (err %d)", err);
        /* Restart scanning on failure */
        bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE);
    } else {
        LOG_INF("Connection initiated");
        /* Update parent connection */
        set_parent_conn(conn);
        bt_conn_unref(conn);
    }
}
