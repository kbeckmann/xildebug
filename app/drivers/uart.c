#include "drivers/gpio.h"
#include "drivers/uart.h"

UART_HandleTypeDef uart_handle;

UART_HandleTypeDef *uart_get_handle(void)
{
	return &uart_handle;
}

err_t uart_init(void)
{
	HAL_StatusTypeDef status;

	uart_handle.Instance = USART3;
	uart_handle.Init.BaudRate = 115200;
	uart_handle.Init.WordLength = UART_WORDLENGTH_8B;
	uart_handle.Init.StopBits = UART_STOPBITS_1;
	uart_handle.Init.Parity = UART_PARITY_NONE;
	uart_handle.Init.Mode = UART_MODE_TX_RX;
	uart_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	uart_handle.Init.OverSampling = UART_OVERSAMPLING_16;
	uart_handle.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	uart_handle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

	status = HAL_UART_Init(&uart_handle);
	HAL_ERR_CHECK(status, EUART_HAL_INIT);

	return ERR_OK;
}

void HAL_UART_MspInit(UART_HandleTypeDef *p_handle)
{
	GPIO_InitTypeDef gpio_config;

	if (p_handle->Instance == USART3) {
		__HAL_RCC_USART3_CLK_ENABLE();
		gpio_config.Pin = DUT_UART_TX_Pin|DUT_UART_RX_Pin;
		gpio_config.Mode = GPIO_MODE_AF_PP;
		gpio_config.Pull = GPIO_NOPULL;
		gpio_config.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		gpio_config.Alternate = GPIO_AF7_USART3;
		HAL_GPIO_Init(GPIOB, &gpio_config);
	}
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *p_handle)
{
	if (p_handle->Instance == USART3) {
		__HAL_RCC_USART3_CLK_DISABLE();

		HAL_GPIO_DeInit(GPIOB, DUT_UART_TX_Pin|DUT_UART_RX_Pin);
	}
}
