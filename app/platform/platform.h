#pragma once

#include "errors.h"

#define EPLATFORM_INIT		(EPLATFORM_BASE + 0)

void platform_delay_us(uint32_t delay_us);
void platform_reset(void);
void platform_reboot_to_dfu(void);
err_t platform_init(void);
