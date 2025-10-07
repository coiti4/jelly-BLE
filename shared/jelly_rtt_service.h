/*
 * MIT License 2025
 * Author: Agustín Coitinho
 * Jelly BLE is a project developed within the course
 * "Tecnologías para la Internet de las Cosas"
 * Facultad de Ingeniería - Universidad de la República, Uruguay
 */

#pragma once

#include <zephyr/types.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/sys/util.h>

/* ----------------------------------------------------------------------
 * UUID definitions
 * --------------------------------------------------------------------*/
/* Jelly RTT Service (JRS) */
#define BT_UUID_JRS_VAL \
    BT_UUID_128_ENCODE(0x6f6a0001, 0x8e3d, 0x4d2f, 0xa3a1, 0x1234deadbeef)
#define BT_UUID_JRS_PKT_VAL \
    BT_UUID_128_ENCODE(0x6f6a0002, 0x8e3d, 0x4d2f, 0xa3a1, 0x1234deadbeef)

#define BT_UUID_JRS       BT_UUID_DECLARE_128(BT_UUID_JRS_VAL)
#define BT_UUID_JRS_PKT   BT_UUID_DECLARE_128(BT_UUID_JRS_PKT_VAL)

/* ----------------------------------------------------------------------
 * Jelly packet format
 * --------------------------------------------------------------------*/
typedef struct __packed {
    uint8_t counter; /* Increments upward, decrements downward */
} jrs_pkt_t;

/* ----------------------------------------------------------------------
 * Callbacks
 * --------------------------------------------------------------------*/
/** Callback type for receiving a Jelly packet */
typedef void (*jrs_rx_cb_t)(struct bt_conn *conn, const jrs_pkt_t *pkt);

/* ----------------------------------------------------------------------
 * API
 * --------------------------------------------------------------------*/
/** Initialize the Jelly RTT Service */
int bt_jrs_init(jrs_rx_cb_t cb);

/** Send a Jelly packet to the connected peer */
int bt_jrs_send(struct bt_conn *conn, const jrs_pkt_t *pkt);

/** Store a local timestamp for RTT calculation */
int jrs_store_timestamp(void);

uint16_t get_jrs_pkt_handle(void);
