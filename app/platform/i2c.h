#pragma once

#include "errors.h"

#define EI2C_HAL_INIT					(EI2C_BASE + 0)
#define EI2C_HAL_CONFIG_ANALOG_FILTER	(EI2C_BASE + 1)
#define EI2C_HAL_CONFIG_DIGITAL_FILTER	(EI2C_BASE + 2)
#define EI2C_HAL_MASTER_TRANSMIT		(EI2C_BASE + 3)
#define EI2C_HAL_MASTER_RECEIVE			(EI2C_BASE + 4)
#define EI2C_NO_INIT					(EI2C_BASE + 5)

err_t i2c_master_tx(uint16_t dev_address, uint8_t *p_data, uint16_t size, uint32_t timeout_ms);
err_t i2c_master_rx(uint16_t dev_address, uint8_t *p_data, uint16_t size, uint32_t timeout_ms);
err_t i2c_init(void);
