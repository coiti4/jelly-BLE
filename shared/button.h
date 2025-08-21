/*
 * MIT License 2025
 * Author: Agustín Coitinho
 * Jelly BLE is a project developed within the course
 * "Tecnologías para la Internet de las Cosas"
 * Facultad de Ingeniería - Universidad de la República, Uruguay
 */
#pragma once

#include <stdint.h>

#define USER_BUTTON DK_BTN1_MSK

int init_button(void);
void button_changed(uint32_t button_state, uint32_t has_changed);
