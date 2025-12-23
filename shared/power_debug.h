/*
 * MIT License 2025
 * Author: Agustín Coitinho
 * Jelly BLE is a project developed within the course
 * "Tecnologías para la Internet de las Cosas"
 * Facultad de Ingeniería - Universidad de la República, Uruguay
 *
 * Power Debug GPIO Module
 * This module provides functions to toggle GPIO pins during power
 * consumption measurements with the Nordic PPK2.
 *
 * Pin scheme for nRF5340-DK:
 * - P1.00 (DEBUG_PIN_0): Scanning/Connecting activity
 * - P1.01 (DEBUG_PIN_1): Connected state / Message activity
 * - P1.02 (DEBUG_PIN_2): Advertising state
 * - P1.03 (DEBUG_PIN_3): Forward/Special events
 *
 * Pins can be connected to a logic analyzer alongside the PPK2
 * to correlate current consumption with specific activities.
 */

#ifndef POWER_DEBUG_H
#define POWER_DEBUG_H

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/* Pin definitions - nRF5340 DK Port 1 */
#define DEBUG_PORT DT_NODELABEL(gpio1)
#define DEBUG_PIN_0 0  /* P1.00 - Scanning/Connecting */
#define DEBUG_PIN_1 1  /* P1.01 - Connected/Message */
#define DEBUG_PIN_2 2  /* P1.02 - Advertising */
#define DEBUG_PIN_3 3  /* P1.03 - Forward/Events */

/* Debug states */
typedef enum {
    DBG_IDLE = 0,
    DBG_SCANNING,
    DBG_CONNECTING,
    DBG_CONNECTED,
    DBG_ADVERTISING,
    DBG_MESSAGE_TX,
    DBG_MESSAGE_RX,
    DBG_FORWARD
} debug_state_t;

/**
 * @brief Initialize GPIO pins for power consumption debugging
 * @return 0 on success, negative value on error
 */
int power_debug_init(void);

/**
 * @brief Set a specific pin to high level
 * @param pin Pin number (0-3)
 */
void power_debug_set(uint8_t pin);

/**
 * @brief Clear a specific pin to low level
 * @param pin Pin number (0-3)
 */
void power_debug_clear(uint8_t pin);

/**
 * @brief Toggle a specific pin
 * @param pin Pin number (0-3)
 */
void power_debug_toggle(uint8_t pin);

/**
 * @brief Set multiple pins according to a bit mask
 * @param mask 4-bit mask (bit 0 = pin 0, etc)
 */
void power_debug_set_pattern(uint8_t mask);

/**
 * @brief Mark the start of an operation (set pin)
 * @param state Debug state
 */
void power_debug_start(debug_state_t state);

/**
 * @brief Mark the end of an operation (clear pin)
 * @param state Debug state
 */
void power_debug_end(debug_state_t state);

/**
 * @brief Generate a short pulse on a pin (useful for instant events)
 * @param pin Pin number (0-3)
 */
void power_debug_pulse(uint8_t pin);

#endif /* POWER_DEBUG_H */
