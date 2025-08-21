/*
 * MIT License 2025
 * Author: Agustín Coitinho
 * Jelly BLE is a project developed within the course
 * "Tecnologías para la Internet de las Cosas"
 * Facultad de Ingeniería - Universidad de la República, Uruguay
 */
#include <zephyr/bluetooth/conn.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include "connection_manager.h"

LOG_MODULE_REGISTER(connection_manager, LOG_LEVEL_INF);

static struct bt_conn *parent_conn = NULL;
static struct bt_conn *child_conn = NULL;

/* Get parent connection */
struct bt_conn *get_parent_conn(void)
{
    return parent_conn;
}

/* Get child connection */
struct bt_conn *get_child_conn(void)
{
    return child_conn;
}

/* Set parent connection */
void set_parent_conn(struct bt_conn *conn)
{
    if (conn == parent_conn) {
        LOG_DBG("set_parent_conn: same pointer, no change (%p)", (void *)conn);
        return;
    }

    if (parent_conn) {
        bt_conn_unref(parent_conn);
        parent_conn = NULL;
    }

    parent_conn = conn ? bt_conn_ref(conn) : NULL;

    LOG_INF("Parent connection set to %p", (void *)parent_conn);
}

/* Set child connection */
void set_child_conn(struct bt_conn *conn)
{
    if (conn == child_conn) {
        LOG_DBG("set_child_conn: same pointer, no change (%p)", (void *)conn);
        return;
    }

    if (child_conn) {
        bt_conn_unref(child_conn);
        child_conn = NULL;
    }

    child_conn = conn ? bt_conn_ref(conn) : NULL;

    LOG_INF("Child connection set to %p", (void *)child_conn);
}
