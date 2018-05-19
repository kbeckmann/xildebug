#include <FreeRTOS.h>
#include <task.h>
#include <stdint.h>

#include "drivers/led.h"
#include "stm32l4xx_hal.h"
#include "platform.h"

#define TIMEOUT_S 60

void HardFault_Handler(void)
{
	vTaskEndScheduler();

	/* Wait for a debugger to attach */
	for (uint32_t i = 0; i < TIMEOUT_S * 10; i++) {
		led_rgb_set(i & 7);
		led_rx_set(i & 1);
		led_tx_set((i + 1) & 1);
		platform_delay_us(100000);
	}

	HAL_NVIC_SystemReset();
}

void MemManage_Handler(void)
{
	HardFault_Handler();
}

void BusFault_Handler(void)
{
	HardFault_Handler();
}

void assert_failed(uint8_t *p_file, uint32_t line)
{
	printf("ASSERT FAILED IN: %s:%ld\r\n", p_file, line);
	HardFault_Handler();
}
