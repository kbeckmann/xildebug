#pragma once

#include "gpio_pins.h"
#include "drivers/led.h"
#include "stm32l4xx_ll_gpio.h"

#include <stdint.h>

#define CPU_CLOCK					HAL_RCC_GetHCLKFreq()
#define IO_PORT_WRITE_CYCLES		2
#define DAP_SWD						1
#define DAP_JTAG					0
#define DAP_JTAG_DEV_CNT			0
#define DAP_DEFAULT_PORT			1
#define DAP_DEFAULT_SWJ_CLOCK		10000
#define DAP_PACKET_SIZE				64
#define DAP_PACKET_COUNT			2
#define SWO_UART					0
#define SWO_UART_MAX_BAUDRATE		10000000U
#define SWO_MANCHESTER				0
#define SWO_BUFFER_SIZE				4096U
#define TARGET_DEVICE_FIXED			0

static inline void PORT_SWD_SETUP(void)
{
	LL_GPIO_SetOutputPin((GPIO_TypeDef*)DUT_SWCLK_GPIO_Port, DUT_SWCLK_Pin);
	LL_GPIO_SetOutputPin((GPIO_TypeDef*)DUT_SWDIO_GPIO_Port, DUT_SWDIO_Pin);

	LL_GPIO_SetPinMode((GPIO_TypeDef*)DUT_SWDIO_GPIO_Port, DUT_SWDIO_Pin, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinOutputType((GPIO_TypeDef*)DUT_SWDIO_GPIO_Port, DUT_SWDIO_Pin, LL_GPIO_OUTPUT_PUSHPULL);
	LL_GPIO_SetPinPull((GPIO_TypeDef*)DUT_SWDIO_GPIO_Port, DUT_SWDIO_Pin, LL_GPIO_PULL_UP);

	LL_GPIO_SetPinMode((GPIO_TypeDef*)DUT_SWCLK_GPIO_Port, DUT_SWCLK_Pin, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinOutputType((GPIO_TypeDef*)DUT_SWCLK_GPIO_Port, DUT_SWCLK_Pin, LL_GPIO_OUTPUT_PUSHPULL);
	LL_GPIO_SetPinPull((GPIO_TypeDef*)DUT_SWCLK_GPIO_Port, DUT_SWCLK_Pin, LL_GPIO_PULL_UP);

	led_swd_set(1);
}

static inline void PORT_OFF(void)
{
	LL_GPIO_SetPinMode((GPIO_TypeDef*)DUT_SWCLK_GPIO_Port, DUT_SWCLK_Pin, LL_GPIO_MODE_INPUT);
	LL_GPIO_SetPinPull((GPIO_TypeDef*)DUT_SWCLK_GPIO_Port, DUT_SWCLK_Pin, LL_GPIO_PULL_NO);

	LL_GPIO_SetPinMode((GPIO_TypeDef*)DUT_SWDIO_GPIO_Port, DUT_SWDIO_Pin, LL_GPIO_MODE_INPUT);
	LL_GPIO_SetPinPull((GPIO_TypeDef*)DUT_SWDIO_GPIO_Port, DUT_SWDIO_Pin, LL_GPIO_PULL_NO);

	led_swd_set(0);
}

#define DAP_SETUP()	PORT_OFF()

static inline uint8_t PIN_SWCLK_TCK_IN(void)
{
	return LL_GPIO_IsOutputPinSet((GPIO_TypeDef*)DUT_SWCLK_GPIO_Port, DUT_SWCLK_Pin);
}

static inline void PIN_SWCLK_TCK_SET(void)
{
	LL_GPIO_SetOutputPin((GPIO_TypeDef*)DUT_SWCLK_GPIO_Port, DUT_SWCLK_Pin);
}

static inline void PIN_SWCLK_TCK_CLR(void)
{
	LL_GPIO_ResetOutputPin((GPIO_TypeDef*)DUT_SWCLK_GPIO_Port, DUT_SWCLK_Pin);
}

static inline uint8_t PIN_SWDIO_TMS_IN(void)
{
	return LL_GPIO_IsInputPinSet((GPIO_TypeDef*)DUT_SWDIO_GPIO_Port, DUT_SWDIO_Pin);
}

static inline void PIN_SWDIO_TMS_SET(void)
{
	LL_GPIO_SetOutputPin((GPIO_TypeDef*)DUT_SWDIO_GPIO_Port, DUT_SWDIO_Pin);
}

static inline void PIN_SWDIO_TMS_CLR(void)
{
	LL_GPIO_ResetOutputPin((GPIO_TypeDef*)DUT_SWDIO_GPIO_Port, DUT_SWDIO_Pin);
}

static inline uint8_t PIN_SWDIO_IN (void)
{
	return LL_GPIO_IsInputPinSet((GPIO_TypeDef*)DUT_SWDIO_GPIO_Port, DUT_SWDIO_Pin);
}

static inline void PIN_SWDIO_OUT(uint8_t bit)
{
	if (bit & 1)
		LL_GPIO_SetOutputPin((GPIO_TypeDef*)DUT_SWDIO_GPIO_Port, DUT_SWDIO_Pin);
	else
		LL_GPIO_ResetOutputPin((GPIO_TypeDef*)DUT_SWDIO_GPIO_Port, DUT_SWDIO_Pin);
}

static inline void PIN_SWDIO_OUT_ENABLE(void)
{
	LL_GPIO_SetPinMode((GPIO_TypeDef*)DUT_SWDIO_GPIO_Port, DUT_SWDIO_Pin, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinOutputType((GPIO_TypeDef*)DUT_SWDIO_GPIO_Port, DUT_SWDIO_Pin, LL_GPIO_OUTPUT_PUSHPULL);
	LL_GPIO_SetPinPull((GPIO_TypeDef*)DUT_SWDIO_GPIO_Port, DUT_SWDIO_Pin, LL_GPIO_PULL_UP);
}

static inline void PIN_SWDIO_OUT_DISABLE(void)
{
	LL_GPIO_SetPinMode((GPIO_TypeDef*)DUT_SWDIO_GPIO_Port, DUT_SWDIO_Pin, LL_GPIO_MODE_INPUT);
}

static inline uint8_t PIN_TDI_IN(void)
{
	return 0;
}

static inline void PIN_TDI_OUT(uint8_t bit)
{

}

static inline uint8_t PIN_TDO_IN(void)
{
	return 0;
}

static inline uint8_t PIN_nTRST_IN(void)
{
	return 0;
}

static inline void PIN_nTRST_OUT(uint8_t bit)
{

}

static inline uint8_t PIN_nRESET_IN(void)
{
	return 0;
}

static inline void PIN_nRESET_OUT(uint8_t bit)
{

}

static inline uint8_t RESET_TARGET(void)
{
	return 0;
}

static inline void LED_CONNECTED_OUT(uint8_t bit)
{

}

static inline void LED_RUNNING_OUT(uint8_t bit)
{
}
