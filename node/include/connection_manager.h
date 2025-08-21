/*
 * MIT License 2025
 * Author: Agustín Coitinho
 * Jelly BLE is a project developed within the course
 * "Tecnologías para la Internet de las Cosas"
 * Facultad de Ingeniería - Universidad de la República, Uruguay
 */
#pragma once

#include <zephyr/bluetooth/bluetooth.h>

/* Get parent connection */
struct bt_conn *get_parent_conn(void);

/* Get child connection */
struct bt_conn *get_child_conn(void);

/* Set parent connection */
void set_parent_conn(struct bt_conn *conn);

/* Set child connection */
void set_child_conn(struct bt_conn *conn);
