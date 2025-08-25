/*
 * MIT License 2025
 * Author: Agustín Coitinho
 * Jelly BLE is a project developed within the course
 * "Tecnologías para la Internet de las Cosas"
 * Facultad de Ingeniería - Universidad de la República, Uruguay
 */
#pragma once
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/bluetooth.h>

#define COORDINATOR_RETRY_TIMES 15

void register_central_connection_callbacks(void);

/* Initialize and start scanning for the coordinator */
int start_scanning(void);

/* Connect to the detected device */
void connect_to_device(const bt_addr_le_t *addr, const struct bt_le_conn_param *conn_param);
