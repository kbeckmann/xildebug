#include <FreeRTOS.h>
#include <string.h>
#include <task.h>

#include "drivers/adc.h"
#include "drivers/gpio.h"
#include "drivers/i2c.h"
#include "drivers/led.h"
#include "drivers/max14662.h"
#include "drivers/mcp4018t.h"
#include "drivers/uart.h"
#include "drivers/usb.h"
#include "drivers/usb_cdc.h"
#include "drivers/usb_hid.h"
#include "power.h"
#include "stm32l4xx_hal.h"

#define MAIN_TASK_STACK_SIZE	512
#define MAIN_TASK_NAME			"Main"
#define MAIN_TASK_PRIORITY		1

static StackType_t main_task_stack[MAIN_TASK_STACK_SIZE];
static TaskHandle_t main_task_handle;
static StaticTask_t main_task_tcb;

HAL_StatusTypeDef SystemClock_Config(void)
{
	HAL_StatusTypeDef status;

	RCC_OscInitTypeDef RCC_OscInitStruct = {
		.OscillatorType = RCC_OSCILLATORTYPE_HSI48 | RCC_OSCILLATORTYPE_HSI,
		.HSIState = RCC_HSI_ON,
		.HSI48State = RCC_HSI48_ON,
		.HSICalibrationValue = 16,
		.PLL.PLLState = RCC_PLL_ON,
		.PLL.PLLSource = RCC_PLLSOURCE_HSI,
		.PLL.PLLN = 10,
		.PLL.PLLP = RCC_PLLP_DIV7,
		.PLL.PLLQ = RCC_PLLQ_DIV2,
		.PLL.PLLR = RCC_PLLR_DIV2,
	};

	status = HAL_RCC_OscConfig(&RCC_OscInitStruct);
	if (status != HAL_OK)
		return status;

	RCC_ClkInitTypeDef RCC_ClkInitStruct = {
		.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2,
		.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK,
		.AHBCLKDivider = RCC_SYSCLK_DIV1,
		.APB1CLKDivider = RCC_HCLK_DIV1,
		.APB2CLKDivider = RCC_HCLK_DIV1,
	};

	status = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);
	if (status != HAL_OK)
		return status;

	RCC_PeriphCLKInitTypeDef PeriphClkInit = {
		.PeriphClockSelection = RCC_PERIPHCLK_USART3 | RCC_PERIPHCLK_I2C1 | RCC_PERIPHCLK_USB | RCC_PERIPHCLK_ADC,
		.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1,
		.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1,
		.AdcClockSelection = RCC_ADCCLKSOURCE_SYSCLK,
		.UsbClockSelection = RCC_USBCLKSOURCE_HSI48,
	};

	status = HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
	if (status != HAL_OK)
		return status;

	status = HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
	if (status != HAL_OK)
		return status;

	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);

	return status;
}

int _write(int fd, const char *msg, int len)
{
	uart_tx(uart_get_handle(), (const uint8_t*)msg, len, 100);
	return len;
}

int _read(int fd, char *msg, int len)
{
	return 0;
}

void main_task(void *p_arg)
{
	uint8_t uart_rx_buf[8];
	int i = 0;
	err_t r;

	/* TODO: Handle this properly later... */
	max14662_set_value(MAX14662_AD_0_0, 0xff);

	while (1) {
		i++;
		led_rgb_set(i % 8);

		printf("Hello world %d! (rx=%s)\r\n", i, uart_rx_buf);

		if (i % 10)
			usb_hid_send(false, false, false, 3, 3);

		memset(uart_rx_buf, 0, sizeof(uart_rx_buf));
		r = uart_rx(uart_get_handle(), uart_rx_buf, sizeof(uart_rx_buf) - 1, 250);
		if (r == ERR_OK)
			r = usb_cdc_tx(uart_rx_buf, sizeof(uint8_t) - 1);
	}
}

int main(void)
{
	HAL_StatusTypeDef status;
	err_t r;

	status = HAL_Init();
	if (status != HAL_OK)
		while (1) ;

	status = SystemClock_Config();
	if (status != HAL_OK)
		while (1) ;

	gpio_init();

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

	r = power_init();
	ERR_CHECK(r);

	r = usb_init();
	ERR_CHECK(r);

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

	while (1) {

	}

	return 0;
}
