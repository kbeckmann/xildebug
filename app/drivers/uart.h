#pragma once

#include <FreeRTOS.h>
#include <queue.h>
#include <stdbool.h>

#include "errors.h"
#include "stm32l4xx_hal.h"

#define EUART_HAL_INIT					(EUART_BASE + 0)
#define EUART_TX_SEMPH					(EUART_BASE + 1)
#define EUART_RX_SEMPH					(EUART_BASE + 2)
#define EUART_TX_TIMEOUT				(EUART_BASE + 3)
#define EUART_RX_TIMEOUT				(EUART_BASE + 4)
#define EUART_TX						(EUART_BASE + 5)
#define EUART_RX						(EUART_BASE + 6)
#define EUART_FLUSH_RX					(EUART_BASE + 7)
#define EUART_INVALID_ARG				(EUART_BASE + 8)

enum uart_stopbits_t {
	UART_STOPBITS_CONFIG_1		= 0,
	UART_STOPBITS_CONFIG_1_5	= 1,
	UART_STOPBITS_CONFIG_2		= 2,
};

enum uart_parity_t {
	UART_PARITY_CONFIG_NONE		= 0,
	UART_PARITY_CONFIG_ODD		= 1,
	UART_PARITY_CONFIG_EVEN		= 2,
	UART_PARITY_CONFIG_MARK		= 3,
	UART_PARITY_CONFIG_SPACE	= 4,
};

struct uart_line_coding_t {
	uint32_t				baudrate_bps;
	enum uart_stopbits_t	stopbits : 8;
	enum uart_parity_t		parity : 8;
	uint8_t					databits;
} __packed;

err_t uart_tx(const uint8_t *p_buf, uint32_t size, uint32_t timeout_ticks, bool blocking);
err_t uart_start_rx(QueueHandle_t queue);
err_t uart_flush_rx(void);
err_t uart_config_set(struct uart_line_coding_t *p_config);
err_t uart_init(void);
