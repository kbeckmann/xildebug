#include "platform.h"
#include "stm32l4xx_hal.h"

/* Platform implementation for stm32l433 */

void platform_delay_us(uint32_t delay_us) {
	const uint64_t ticks = ((uint64_t) delay_us) * ((uint64_t) HAL_RCC_GetSysClockFreq()) / 1000000;
	register uint32_t loops = ticks / 6; /* 6 cycles per loop */

	while (loops--) {
		__ASM("nop");
	}
}
