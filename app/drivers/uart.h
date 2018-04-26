#pragma once

#include "stm32l4xx_hal.h"
#include "errors.h"

#define EUART_HAL_INIT					(EUART_BASE + 0)

UART_HandleTypeDef *uart_get_handle(void);
err_t uart_init(void);
