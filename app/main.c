#include <FreeRTOS.h>
#include <string.h>
#include <task.h>

#include "cdc_uart_bridge.h"
#include "drivers/led.h"
#include "drivers/max14662.h"
#include "drivers/mcp4018t.h"
#include "drivers/uart.h"
#include "drivers/usb.h"
#include "drivers/usb/cdc.h"
#include "drivers/usb/hid.h"
#include "platform/adc.h"
#include "platform/gpio.h"
#include "platform/i2c.h"
#include "power.h"
#include "target.h"

#define MAIN_TASK_STACK_SIZE	512
#define MAIN_TASK_NAME			"Main"
#define MAIN_TASK_PRIORITY		1

static StackType_t main_task_stack[MAIN_TASK_STACK_SIZE];
static TaskHandle_t main_task_handle;
static StaticTask_t main_task_tcb;

int _write(int fd, const char *msg, int len)
{
	uart_tx((const uint8_t*)msg, len, 100, true);
	return len;
}

int _read(int fd, char *msg, int len)
{
	return 0;
}

void main_task(void *p_arg)
{
	int i = 0;
	err_t r;

	r = i2c_init();
	ERR_CHECK(r);

	r = uart_init();
	ERR_CHECK(r);

	r = adc_init();
	ERR_CHECK(r);

	r = max14662_init(MAX14662_AD_0_0);
	ERR_CHECK(r);

	r = mcp4018t_init();
	ERR_CHECK(r);

	r = usb_init();
	ERR_CHECK(r);

	r = cdc_uart_bridge_init();
	ERR_CHECK(r);

	r = power_init();
	ERR_CHECK(r);

	while (1) {
		i++;
		led_rgb_set(i % 8);

		vTaskDelay(pdMS_TO_TICKS(250));
	}
}

int main(void)
{
	err_t r;

	r = target_init();
	ERR_CHECK(r);

	gpio_init();

	main_task_handle = xTaskCreateStatic(
		main_task,
		MAIN_TASK_NAME,
		MAIN_TASK_STACK_SIZE,
		NULL,
		MAIN_TASK_PRIORITY,
		&main_task_stack[0],
		&main_task_tcb);

	if (main_task_handle == NULL)
		while (1) ;

	vTaskStartScheduler();

	while (1) ;

	return 0;
}
