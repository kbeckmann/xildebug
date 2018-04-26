#include "drivers/mcp4018t.h"
#include "drivers/i2c.h"

#define MCP4018T_ADDRESS	0x2F
#define I2C_TIMEOUT			100
#define SELF_TEST_VALUE		0x2A
#define DEFAULT_VALUE		0x01
#define MAX_VALUE			0x7f

static uint8_t mcp4018t_value;

err_t mcp4018t_set_value(uint8_t val)
{
	if (val > MAX_VALUE)
		return EMCP4018T_INVALID_ARG;

	if (mcp4018t_value == val)
		return ERR_OK;

	return i2c_master_tx(MCP4018T_ADDRESS << 1, &val, sizeof(val), I2C_TIMEOUT);
}

err_t mcp4018t_get_value(uint8_t *p_val)
{
	return i2c_master_rx(MCP4018T_ADDRESS << 1, p_val, sizeof(*p_val), I2C_TIMEOUT);
}

err_t mcp4018t_init(void)
{
	uint8_t value;
	err_t r;

	r = mcp4018t_set_value(SELF_TEST_VALUE);
	ERR_CHECK(r);

	r = mcp4018t_get_value(&value);
	ERR_CHECK(r);

	if (value != SELF_TEST_VALUE)
		return EMCP4018T_INVALID_RESPONSE;

	r = mcp4018t_set_value(DEFAULT_VALUE);
	ERR_CHECK(r);

	mcp4018t_value = DEFAULT_VALUE;

	return r;
}
