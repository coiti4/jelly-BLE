#pragma once

#include <stdint.h>

#define USER_BUTTON DK_BTN1_MSK

int init_button(void);
void button_changed(uint32_t button_state, uint32_t has_changed);
