#include "drivers/led.h"
#include "platform/gpio.h"

void led_rgb_set(uint8_t color)
{
	if (color & RGB_RED)
		gpio_write(LED_RGB_R_GPIO_Port, LED_RGB_R_Pin, false);
	else
		gpio_write(LED_RGB_R_GPIO_Port, LED_RGB_R_Pin, true);

	if (color & RGB_GREEN)
		gpio_write(LED_RGB_G_GPIO_Port, LED_RGB_G_Pin, false);
	else
		gpio_write(LED_RGB_G_GPIO_Port, LED_RGB_G_Pin, true);

	if (color & RGB_BLUE)
		gpio_write(LED_RGB_B_GPIO_Port, LED_RGB_B_Pin, false);
	else
		gpio_write(LED_RGB_B_GPIO_Port, LED_RGB_B_Pin, true);
}

void led_tx_set(bool enabled)
{
	gpio_write(DUT_UART_TX_LED_GPIO_Port, DUT_UART_TX_LED_Pin, !enabled);
}

void led_rx_set(bool enabled)
{
	gpio_write(DUT_UART_RX_LED_GPIO_Port, DUT_UART_RX_LED_Pin, !enabled);
}

void led_swd_set(bool enabled)
{
	gpio_write(DUT_SWD_LED_GPIO_Port, DUT_SWD_LED_Pin, !enabled);
}

void led_init(void)
{
	led_tx_set(false);
	led_rx_set(false);
	led_swd_set(false);
	led_rgb_set(0);
}
