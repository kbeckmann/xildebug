#pragma once

#include <stdbool.h>
#include <stdint.h>

/* This file is located in targets/$(TARGET)/gpio_pins.h */
#include "gpio_pins.h"

/* platform/$(PLATFORM)/gpio_impl.h implements:
 *
 * void gpio_write(uint32_t port, uint32_t pin, bool value);
 *
 * This is done so the code can be inlined.
 */
#include "gpio_impl.h"

void gpio_init(void);
