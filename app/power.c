#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include "drivers/adc.h"
#include "drivers/gpio.h"
#include "drivers/mcp4018t.h"
#include "power.h"
#include "stm32l4xx_hal.h"

#define POWER_TASK_STACK_SIZE	512
#define POWER_TASK_NAME			"Power"
#define POWER_TASK_PRIORITY		1

#define QUEUE_LENGTH			10
#define QUEUE_ITEM_SIZE			(sizeof(uint16_t) * NUM_OF_ADC_CHANNELS)

#define DEFAULT_CALIB_MIN_MV	1606
#define DEFAULT_CALIB_MAX_MV	5065

static struct {
	bool initialized;
	StackType_t task_stack[POWER_TASK_STACK_SIZE];
	TaskHandle_t task_handle;
	StaticTask_t task_tcb;
	QueueHandle_t queue_handle;
	StaticQueue_t queue;
	uint8_t queue_storage[QUEUE_LENGTH * QUEUE_ITEM_SIZE];

	bool shunt1_enabled;
	bool shunt2_enabled;
	bool dut_vdd_enabled;
	uint16_t ldo_voltage;

	/* TODO: Add support to configure these calibration values */
	uint32_t calib_min_mv;
	uint32_t calib_max_mv;
} self;

/*
TODO: Uncomment when we need them
static float calib_shunt_00 = 512.2f;
static float calib_shunt_01 = 4.1f;
static float calib_shunt_10 = 33.3f;
static float calib_shunt_11 = 512.2f;
*/

/* Simple schematic of the resistor switch network.
 *
 * DUT_VDD_IN----[510R]----DUT_VDD_OUT----[DUT]----GND
 *            |           |
 *     shunt1 |---[33R]---|
 *     shunt2 |---[1R8]---|
 *
 * I.e. Current always flows through the 510R, but the total resistance may be lowered
 * by toggling the shunt resistor switches. The ts5a3167 switches have inverted logic.
 * 
 * S1 S2      R   Measured resistance
 *  0  0 510.00   512.2
 *  0  1   1.79   4.1
 *  1  0  30.99   33.3
 *  1  1   1.70   4.0
 */

static void shunt1_set_enabled(bool enabled)
{
	gpio_write(SHUNT1_EN_GPIO_Port, SHUNT1_EN_Pin, !enabled);
	self.shunt1_enabled = enabled;
}

static void shunt2_set_enabled(bool enabled)
{
	gpio_write(SHUNT2_EN_GPIO_Port, SHUNT2_EN_Pin, !enabled);
	self.shunt2_enabled = enabled;
}

static void power_task(void *p_arg)
{
	uint16_t adc_values[NUM_OF_ADC_CHANNELS];

	for (;;) {
		xQueueReceive(self.queue_handle, adc_values, portMAX_DELAY);

		/* TODO: Do stuff with the values */
	}
}

static void adc_conversion_ready_handler(const uint16_t adc_values[NUM_OF_ADC_CHANNELS])
{
	BaseType_t higher_priority_task_woken = pdFALSE;

	xQueueSendFromISR(self.queue_handle, adc_values, &higher_priority_task_woken);
	portYIELD_FROM_ISR(higher_priority_task_woken);
}

void power_dut_set_enabled(bool enabled)
{
	gpio_write(DUT_VDD_EN_GPIO_Port, DUT_VDD_EN_Pin, !enabled);
	self.dut_vdd_enabled = enabled;
}

err_t power_dut_get_enabled(bool *p_enabled)
{
	if (!self.initialized)
		return EPOWER_NO_INIT;

	if (!p_enabled)
		return EPOWER_INVALID_ARG;

	*p_enabled = self.dut_vdd_enabled;

	return ERR_OK;
}

err_t power_dut_ldo_set(uint32_t millivolt)
{
	err_t r;

	if (!self.initialized)
		return EPOWER_NO_INIT;

	if (millivolt < self.calib_min_mv)
		millivolt = self.calib_min_mv;
	else if(millivolt > self.calib_max_mv)
		millivolt = self.calib_max_mv;

	const float factor = (self.calib_max_mv - ((float)millivolt)) / (self.calib_max_mv - self.calib_min_mv);
	const uint8_t value = (uint8_t) (factor * 127 - 0.5f);

	r = mcp4018t_set_value(value);
	ERR_CHECK(r);

	self.ldo_voltage = millivolt;

	return r;
}

err_t power_dut_ldo_get(uint32_t *p_millivolt)
{
	if (!self.initialized)
		return EPOWER_NO_INIT;

	if (!p_millivolt)
		return EPOWER_INVALID_ARG;

	*p_millivolt = self.ldo_voltage;

	return ERR_OK;
}

err_t power_init(void)
{
	err_t r;

	if (self.initialized)
		return ERR_OK;

	self.task_handle = xTaskCreateStatic(
		power_task,
		POWER_TASK_NAME,
		POWER_TASK_STACK_SIZE,
		NULL,
		POWER_TASK_PRIORITY,
		&self.task_stack[0],
		&self.task_tcb);
	if (self.task_handle == NULL)
		return EPOWER_TASK_CREATE;

	self.queue_handle = xQueueCreateStatic(QUEUE_LENGTH, QUEUE_ITEM_SIZE, self.queue_storage, &self.queue);
	if (self.queue_handle == NULL)
		return EPOWER_QUEUE_CREATE;

	adc_set_callback(adc_conversion_ready_handler);
	r = adc_start();
	ERR_CHECK(r);

	power_dut_set_enabled(false);
	shunt1_set_enabled(true);
	shunt2_set_enabled(true);

	self.calib_min_mv = DEFAULT_CALIB_MIN_MV;
	self.calib_max_mv = DEFAULT_CALIB_MAX_MV;

	self.initialized = true;

	/* TODO: Might want to keep this in persistent ram and/or flash so we can
	 * resume with the previous values after a cold boot.
	 */
	power_dut_ldo_set(self.calib_min_mv);

	return ERR_OK;
}
