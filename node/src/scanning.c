#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/logging/log.h>
#include <bluetooth/scan.h>
#include "scanning.h"
#include "connection_manager.h"

LOG_MODULE_REGISTER(scanning, LOG_LEVEL_INF);

/* Callback for filter match */
static void scan_filter_match(struct bt_scan_device_info *device_info,
                            struct bt_scan_filter_match *filter_match,
                            bool connectable)
{
    if (filter_match->name.match) {
        char addr_str[BT_ADDR_LE_STR_LEN];
        bt_addr_le_to_str(device_info->recv_info->addr, addr_str, sizeof(addr_str));

        LOG_INF("Coordinator found: %s (name: %s)", addr_str, filter_match->name.name);
        /* Connection is established automatically if connect_if_match is enabled */
    }
}

/* Callback for filter no-match */
static void scan_filter_no_match(struct bt_scan_device_info *device_info,
                                bool connectable)
{
    /* Handle directed advertising */
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
    LOG_INF("Connecting to coordinator...");
    /* Update coordinator connection */
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
        .connect_if_match = true, /* Connect automatically on filter match */
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

    LOG_INF("Scanning started");
    return 0;
}

void connect_to_device(const bt_addr_le_t *addr, const struct bt_le_conn_param *conn_param)
{
    int err;
    struct bt_conn *conn;

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
