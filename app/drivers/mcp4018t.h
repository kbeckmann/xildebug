#pragma once

#include "stm32l4xx_hal.h"
#include "errors.h"

#define EMCP4018T_INVALID_RESPONSE	(EMCP4018T_BASE + 0)
#define EMCP4018T_INVALID_ARG		(EMCP4018T_BASE + 1)

err_t mcp4018t_set_value(uint8_t val);
err_t mcp4018t_get_value(uint8_t *p_val);
err_t mcp4018t_init(void);
