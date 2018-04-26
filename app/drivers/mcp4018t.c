#include "drivers/mcp4018t.h"
#include "drivers/i2c.h"

#include <stdbool.h>

#define MCP4018T_ADDRESS	0x2F
#define I2C_TIMEOUT_MS		100
#define SELF_TEST_VALUE		0x2A
#define DEFAULT_VALUE		0x01
#define MAX_VALUE			0x7f

static struct {
	bool initialized;
	uint8_t value;
} self;

err_t mcp4018t_set_value(uint8_t val)
{
	if (!self.initialized)
		return EMCP4018T_NO_INIT;

	if (val > MAX_VALUE)
		return EMCP4018T_INVALID_ARG;

	if (self.value == val)
		return ERR_OK;

	return i2c_master_tx(MCP4018T_ADDRESS << 1, &val, sizeof(val), I2C_TIMEOUT_MS);
}

err_t mcp4018t_get_value(uint8_t *p_val)
{
	if (!self.initialized)
		return EMCP4018T_NO_INIT;

	if (!p_val)
		return EMCP4018T_INVALID_ARG;

	return i2c_master_rx(MCP4018T_ADDRESS << 1, p_val, sizeof(*p_val), I2C_TIMEOUT_MS);
}

err_t mcp4018t_init(void)
{
	uint8_t value;
	err_t r;

	if (self.initialized)
		return ERR_OK;

	self.initialized = true;

	r = mcp4018t_set_value(SELF_TEST_VALUE);
	if (r != ERR_OK) {
		self.initialized = false;
		return r;
	}

	r = mcp4018t_get_value(&value);
	if (r != ERR_OK) {
		self.initialized = false;
		return r;
	}

	if (value != SELF_TEST_VALUE) {
		self.initialized = false;
		return EMCP4018T_INVALID_RESPONSE;
	}

	r = mcp4018t_set_value(DEFAULT_VALUE);
	if (r != ERR_OK) {
		self.initialized = false;
		return r;
	}

	self.value = DEFAULT_VALUE;

	return ERR_OK;
}
