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

/**
 * @brief Set the handle of the Jelly RTT Service declaration.
 *
 * @param handle GATT handle of the service declaration.
 */
void set_jrs_service_handle(uint16_t handle);

/**
 * @brief Get the handle of the Jelly RTT Service declaration.
 *
 * @return uint16_t GATT handle of the service declaration.
 */
uint16_t get_jrs_service_handle(void);

/**
 * @brief Set the handle of the JRS characteristic declaration.
 *
 * @param handle GATT handle of the characteristic declaration.
 */
void set_jrs_char_decl_handle(uint16_t handle);

/**
 * @brief Get the handle of the JRS characteristic declaration.
 *
 * @return uint16_t GATT handle of the characteristic declaration.
 */
uint16_t get_jrs_char_decl_handle(void);

/**
 * @brief Set the handle of the JRS characteristic value.
 *
 * @param handle GATT handle of the characteristic value.
 */
void set_jrs_value_handle(uint16_t handle);

/**
 * @brief Get the handle of the JRS characteristic value.
 *
 * @return uint16_t GATT handle of the characteristic value.
 */
uint16_t get_jrs_value_handle(void);

/**
 * @brief Set the handle of the JRS Client Characteristic Configuration (CCC).
 *
 * @param handle GATT handle of the CCC descriptor.
 */
void set_jrs_ccc_handle(uint16_t handle);

/**
 * @brief Get the handle of the JRS Client Characteristic Configuration (CCC).
 *
 * @return uint16_t GATT handle of the CCC descriptor.
 */
uint16_t get_jrs_ccc_handle(void);

/**
 * @brief Print a formatted summary of all JRS handles.
 *
 * This is mainly for debugging and verification after discovery or service registration.
 */
void jrs_print_handle_summary(void);


int jrs_notify(struct bt_conn *conn, const jrs_pkt_t *pkt);
