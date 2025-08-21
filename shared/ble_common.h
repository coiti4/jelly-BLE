/*
 * MIT License 2025
 * Author: Agustín Coitinho
 * Jelly BLE is a project developed within the course
 * "Tecnologías para la Internet de las Cosas"
 * Facultad de Ingeniería - Universidad de la República, Uruguay
 */
#pragma once

#include <zephyr/bluetooth/bluetooth.h>

void ble_init(void);
void ble_advertising_init(void);
void ble_advertising_start(void);
