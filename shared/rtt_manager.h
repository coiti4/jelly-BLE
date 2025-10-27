/*
 * MIT License 2025
 * Author: Agustín Coitinho
 * Jelly BLE is a project developed within the course
 * "Tecnologías para la Internet de las Cosas"
 * Facultad de Ingeniería - Universidad de la República, Uruguay
 */

#pragma once

#include <zephyr/types.h>

void rtt_manager_init(void);
int rtt_store_timestamp(void);
int rtt_compute_time(void);
