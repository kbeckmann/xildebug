#include "platform/gpio.h"
#include "stm32f0xx_hal.h"

void gpio_init(void)
{
	GPIO_InitTypeDef gpio_config;

	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	HAL_GPIO_WritePin((GPIO_TypeDef*) GPIO0_GPIO_Port, GPIO0_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin((GPIO_TypeDef*) GPIO1_GPIO_Port, GPIO1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin((GPIO_TypeDef*) GPIO2_GPIO_Port, GPIO2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin((GPIO_TypeDef*) GPIO3_GPIO_Port, GPIO3_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin((GPIO_TypeDef*) DUT_UART_TX_LED_GPIO_Port, DUT_UART_TX_LED_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin((GPIO_TypeDef*) DUT_UART_RX_LED_GPIO_Port, DUT_UART_RX_LED_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin((GPIO_TypeDef*) DUT_SWD_LED_GPIO_Port, DUT_SWD_LED_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin((GPIO_TypeDef*) LED_RGB_R_GPIO_Port, LED_RGB_R_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin((GPIO_TypeDef*) DUT_SWCLK_GPIO_Port, DUT_SWCLK_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin((GPIO_TypeDef*) DUT_SWDIO_GPIO_Port, DUT_SWDIO_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin((GPIO_TypeDef*) DUT_UART_TX_GPIO_Port, DUT_UART_TX_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin((GPIO_TypeDef*) DUT_UART_RX_GPIO_Port, DUT_UART_RX_Pin, GPIO_PIN_RESET);


	gpio_config.Pin = GPIO_PIN_15;
	gpio_config.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_config.Pull = GPIO_NOPULL;
	gpio_config.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &gpio_config);

	gpio_config.Pin = GPIO0_Pin | GPIO1_Pin | GPIO2_Pin | GPIO3_Pin | DUT_UART_TX_LED_Pin | DUT_UART_RX_LED_Pin | 
					DUT_SWD_LED_Pin | LED_RGB_R_Pin | DUT_SWCLK_Pin | DUT_SWDIO_Pin | DUT_UART_TX_Pin | DUT_UART_RX_Pin;
	gpio_config.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_config.Pull = GPIO_NOPULL;
	gpio_config.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &gpio_config);
}
