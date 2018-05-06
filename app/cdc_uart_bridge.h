#pragma once

#include <stdbool.h>

#include "errors.h"

#define ECDC_UART_BRIDGE_NO_INIT			(ECDC_UART_BRIDGE_BASE + 0)
#define ECDC_UART_BRIDGE_INVALID_ARG		(ECDC_UART_BRIDGE_BASE + 1)
#define ECDC_UART_BRIDGE_TASK_CREATE		(ECDC_UART_BRIDGE_BASE + 2)

err_t cdc_uart_bridge_init(void);
