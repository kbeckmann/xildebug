#include "drivers/max14662.h"
#include "drivers/i2c.h"

#define MAX14662_ADDRESS_0_0	0x4C
#define MAX14662_ADDRESS_0_1	0x4D
#define MAX14662_ADDRESS_1_0	0x4E
#define MAX14662_ADDRESS_1_1	0x4F
#define MAX14662_REG_ADDR		0x00

#define I2C_TIMEOUT				100

#define SELF_TEST_VALUE_1		0xAA
#define SELF_TEST_VALUE_2		0x55
#define DEFAULT_VALUE			0x00

static uint8_t state[NUM_OF_MAX14662_ADDRESSES];

static uint8_t resolve_address(enum MAX14662_address address)
{
	switch (address) {
	default:
	case MAX14662_AD_0_0:
		return MAX14662_ADDRESS_0_0;
	case MAX14662_AD_0_1:
		return MAX14662_ADDRESS_0_1;
	case MAX14662_AD_1_0:
		return MAX14662_ADDRESS_1_0;
	case MAX14662_AD_1_1:
		return MAX14662_ADDRESS_1_1;
	}
}

err_t max14662_set_value(enum MAX14662_address address, uint8_t val)
{
	uint8_t i2c_address;
	err_t r;
	uint8_t data[2] = {
		MAX14662_REG_ADDR,
		val
	};

	if (state[address] == val)
		return HAL_OK;

	i2c_address = resolve_address(address);
	r = i2c_master_tx(i2c_address << 1, data, sizeof(data), I2C_TIMEOUT);
	ERR_CHECK(r);

	state[address] = val;

	return r;
}

err_t max14662_set_bit(enum MAX14662_address address, uint8_t bit, bool value)
{
	const uint8_t current_value = state[address];
	const bool masked_bit = !!(current_value & (1 << bit));
	uint8_t new_value;

	if (masked_bit == value)
		return HAL_OK;

	if (value)
		new_value = state[address] | (1 << bit);
	else
		new_value = state[address] & (~(1 << bit));
	
	return max14662_set_value(address, new_value);
}

uint8_t max14662_get_value_cached(enum MAX14662_address address)
{
	return state[address];
}

err_t max14662_get_value(enum MAX14662_address address, uint8_t *p_val)
{
	const uint8_t i2c_address = resolve_address(address);

	return i2c_master_rx(i2c_address << 1, p_val, sizeof(*p_val), I2C_TIMEOUT);
}

static err_t max14662_set_and_verify(enum MAX14662_address address, uint8_t value)
{
	uint8_t actual_value;
	err_t r;

	r = max14662_set_value(address, value);
	ERR_CHECK(r);

	r = max14662_get_value(address, &actual_value);
	ERR_CHECK(r);

	if (value != actual_value)
		return EMAX14662_INVALID_RESPONSE;

	return r;
}

err_t max14662_init(enum MAX14662_address address)
{
	err_t r;

	r = max14662_set_and_verify(address, SELF_TEST_VALUE_1);
	ERR_CHECK(r);

	r = max14662_set_and_verify(address, SELF_TEST_VALUE_2);
	ERR_CHECK(r);

	r = max14662_set_value(address, DEFAULT_VALUE);
	ERR_CHECK(r);

	return HAL_OK;
}
