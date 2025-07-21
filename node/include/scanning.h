#pragma once

#include <zephyr/bluetooth/bluetooth.h>

/* Initialize and satart scanning for the coordinator */
int start_scanning(void);

/* Connect to the detected device */
void connect_to_device(const bt_addr_le_t *addr, const struct bt_le_conn_param *conn_param);
