#pragma once

#define HAL_MODULE_ENABLED
#define HAL_ADC_MODULE_ENABLED
#define HAL_PCD_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_I2C_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_PCD_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED

#define HSE_VALUE					((uint32_t)8000000U)  /*!< Value of the External oscillator in Hz */
#define HSE_STARTUP_TIMEOUT			((uint32_t)100U)      /*!< Time out for HSE start up, in ms */
#define MSI_VALUE					((uint32_t)4000000U)  /*!< Value of the Internal oscillator in Hz*/
#define HSI_VALUE					((uint32_t)16000000U) /*!< Value of the Internal oscillator in Hz*/
#define HSI48_VALUE					((uint32_t)48000000U) /*!< Value of the Internal High Speed oscillator for USB FS/SDMMC/RNG in Hz. */
#define LSI_VALUE					((uint32_t)32000U)    /*!< LSI Typical Value in Hz*/
#define LSE_VALUE					((uint32_t)32768U)    /*!< Value of the External oscillator in Hz*/
#define LSE_STARTUP_TIMEOUT			((uint32_t)5000U)     /*!< Time out for LSE start up, in ms */
#define EXTERNAL_SAI1_CLOCK_VALUE	((uint32_t)2097000U)  /*!< Value of the SAI1 External clock source in Hz*/
#define EXTERNAL_SAI2_CLOCK_VALUE	((uint32_t)48000U)    /*!< Value of the SAI2 External clock source in Hz*/

#define VDD_VALUE					((uint32_t)3300U)     /*!< Value of VDD in mv */
#define TICK_INT_PRIORITY			((uint32_t)0U)        /*!< tick interrupt priority */
#define USE_RTOS					0
#define PREFETCH_ENABLE				0
#define INSTRUCTION_CACHE_ENABLE	1
#define DATA_CACHE_ENABLE			1

//#define USE_FULL_ASSERT			1

#define USE_SPI_CRC					0

#include "stm32l4xx_hal_dma.h"
#include "stm32l4xx_hal_adc.h"
#include "stm32l4xx_hal_cortex.h"
#include "stm32l4xx_hal_flash.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_hal_i2c.h"
#include "stm32l4xx_hal_pcd.h"
#include "stm32l4xx_hal_pwr_ex.h"
#include "stm32l4xx_hal_pwr.h"
#include "stm32l4xx_hal_rcc.h"
#include "stm32l4xx_hal_tim.h"
#include "stm32l4xx_hal_uart.h"

#ifdef USE_FULL_ASSERT
	#define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
	void assert_failed(uint8_t* file, uint32_t line);
#else
	#define assert_param(expr) ((void)0U)
#endif
