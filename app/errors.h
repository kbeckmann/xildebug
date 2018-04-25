#pragma once

#include "stm32l4xx_hal.h"

typedef uint32_t err_t;

#define ERR_CHECK(r) \
	do { \
		if ((r) != ERR_OK) \
			for(;;); \
	} while (0)

#define HAL_ERR_CHECK(hal_status, err_to_return) \
	do { \
		if ((hal_status) != HAL_OK) \
			return (err_to_return | ((hal_status) << 14)); \
	} while (0)

#define ERR_OK			(0)
#define ERR_BASE		(0x80000000)
#define ADC_BASE		(ERR_BASE + (0x01 << 16))
#define GPIO_BASE		(ERR_BASE + (0x02 << 16))
#define I2C_BASE		(ERR_BASE + (0x03 << 16))
#define LED_BASE		(ERR_BASE + (0x04 << 16))
#define MAX14662_BASE	(ERR_BASE + (0x05 << 16))
#define MCP4018T_BASE	(ERR_BASE + (0x06 << 16))
#define UART_BASE		(ERR_BASE + (0x07 << 16))
#define POWER_BASE		(ERR_BASE + (0x08 << 16))
#define EUSB_BASE		(ERR_BASE + (0x09 << 16))
#define EUSB_CDC_BASE	(ERR_BASE + (0x0A << 16))
