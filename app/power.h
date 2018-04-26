#pragma once

#include <stdbool.h>
#include "errors.h"

#define EPOWER_NO_INIT			(EPOWER_BASE + 0)
#define EPOWER_INVALID_ARG		(EPOWER_BASE + 1)
#define EPOWER_TASK_CREATE		(EPOWER_BASE + 2)
#define EPOWER_QUEUE_CREATE		(EPOWER_BASE + 3)

void power_dut_set_enabled(bool enabled);
err_t power_dut_get_enabled(bool *p_enabled);
err_t power_dut_ldo_set(uint32_t millivolt);
err_t power_dut_ldo_get(uint32_t *p_millivolt);
err_t power_init(void);
