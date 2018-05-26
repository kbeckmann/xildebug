#pragma once

#include <stdbool.h>
#include <stdint.h>

#define RGB_RED		(1 << 0)
#define RGB_GREEN 	(1 << 1)
#define RGB_BLUE	(1 << 2)

void led_rgb_set(uint8_t color);
void led_tx_set(bool enabled);
void led_rx_set(bool enabled);
void led_swd_set(bool enabled);
void led_init(void);
