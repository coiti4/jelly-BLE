#pragma once

#define USER_BUTTON DK_BTN1_MSK
#define RUN_STATUS_LED DK_LED1
#define CONNECTION_STATUS_LED   DK_LED2
#define RUN_LED_BLINK_INTERVAL 1000

int led_init(void);
void led_blink_loop(int interval_ms);
