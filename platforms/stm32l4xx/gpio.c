#include "platform/gpio.h"

#include "stm32l4xx_hal.h"

void gpio_init(void)
{
	GPIO_InitTypeDef gpio_config;

	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	HAL_GPIO_WritePin(GPIOC, GPIO2_Pin | GPIO3_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, SHUNT1_EN_Pin | SHUNT2_EN_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, DUT_SWCLK_Pin | DUT_SWDIO_Pin | GPIO0_Pin | GPIO1_Pin, GPIO_PIN_RESET);

	/* LEDs have inverted logic. Should be high (off) during boot sequence. */
	HAL_GPIO_WritePin(GPIOH, DUT_UART_TX_LED_Pin | DUT_UART_RX_LED_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, LED_RGB_R_Pin | LED_RGB_G_Pin | DUT_SWD_LED_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, LED_RGB_B_Pin, GPIO_PIN_SET);

	/* Switches also have inverted logic. Should be high (off) during boot sequence. */
	HAL_GPIO_WritePin((GPIO_TypeDef*) DUT_VDD_EN_GPIO_Port, DUT_VDD_EN_Pin, GPIO_PIN_SET);

	gpio_config.Pin = GPIO2_Pin | GPIO3_Pin;
	gpio_config.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_config.Pull = GPIO_NOPULL;
	gpio_config.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &gpio_config);

	gpio_config.Pin = GPIO_PIN_15;
	gpio_config.Mode = GPIO_MODE_ANALOG;
	gpio_config.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &gpio_config);

	gpio_config.Pin = DUT_UART_TX_LED_Pin | DUT_UART_RX_LED_Pin;
	gpio_config.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_config.Pull = GPIO_NOPULL;
	gpio_config.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOH, &gpio_config);

	gpio_config.Pin = GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_7 | GPIO_PIN_15;
	gpio_config.Mode = GPIO_MODE_ANALOG;
	gpio_config.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &gpio_config);

	gpio_config.Pin = DUT_SWD_LED_Pin | SHUNT1_EN_Pin | SHUNT2_EN_Pin | LED_RGB_R_Pin | LED_RGB_G_Pin;
	gpio_config.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_config.Pull = GPIO_NOPULL;
	gpio_config.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &gpio_config);

	gpio_config.Pin = DUT_VDD_EN_Pin | DUT_SWCLK_Pin | DUT_SWDIO_Pin | LED_RGB_B_Pin | GPIO0_Pin | GPIO1_Pin;
	gpio_config.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_config.Pull = GPIO_NOPULL;
	gpio_config.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &gpio_config);

	gpio_config.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
	gpio_config.Mode = GPIO_MODE_ANALOG;
	gpio_config.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &gpio_config);

	gpio_config.Pin = GPIO_PIN_3;
	gpio_config.Mode = GPIO_MODE_ANALOG;
	gpio_config.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOH, &gpio_config);
}
