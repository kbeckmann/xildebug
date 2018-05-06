#pragma once

#include "stm32l4xx_hal.h"

typedef uint32_t err_t;

#define ERR_CHECK(r) \
	do { \
		if ((r) != ERR_OK) \
			for(;;); \
	} while (0)

#define HAL_ERROR_GET(x) (((x) >> 14) & 0x3)
#define HAL_ERROR_SET(hal_status, err_to_return) (err_to_return | ((hal_status) << 14))

#define HAL_ERR_CHECK(hal_status, err_to_return) \
	do { \
		if ((hal_status) != HAL_OK) \
			return HAL_ERROR_SET(err_to_return, hal_status); \
	} while (0)

#define ERR_OK					(0)
#define ERR_BASE				(0x80000000)
#define EADC_BASE				(ERR_BASE + (0x01 << 16))
#define EGPIO_BASE				(ERR_BASE + (0x02 << 16))
#define EI2C_BASE				(ERR_BASE + (0x03 << 16))
#define ELED_BASE				(ERR_BASE + (0x04 << 16))
#define EMAX14662_BASE			(ERR_BASE + (0x05 << 16))
#define EMCP4018T_BASE			(ERR_BASE + (0x06 << 16))
#define EUART_BASE				(ERR_BASE + (0x07 << 16))
#define EPOWER_BASE				(ERR_BASE + (0x08 << 16))
#define EUSB_BASE				(ERR_BASE + (0x09 << 16))
#define EUSB_CDC_BASE			(ERR_BASE + (0x0A << 16))
#define EUSB_HID_BASE			(ERR_BASE + (0x0B << 16))
#define ECDC_UART_BRIDGE_BASE	(ERR_BASE + (0x0C << 16))
