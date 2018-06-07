#include "FreeRTOS.h"
#include "hal_errors.h"
#include "platform/platform.h"
#include "stm32f0xx_hal.h"
#include "task.h"
#include "persistent.h"

#define SYSMEM_ADDRESS 0x1FFFC800

static TIM_HandleTypeDef tim_handle;
extern void xPortSysTickHandler(void);

HAL_StatusTypeDef SystemClock_Config(void)
{
	HAL_StatusTypeDef status;

	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_PeriphCLKInitTypeDef PeriphClkInit;

	/* Initializes the CPU, AHB and APB busses clocks */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI14|RCC_OSCILLATORTYPE_HSI48;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
	RCC_OscInitStruct.HSI14State = RCC_HSI14_ON;
	RCC_OscInitStruct.HSICalibrationValue = 16;
	RCC_OscInitStruct.HSI14CalibrationValue = 16;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	status = HAL_RCC_OscConfig(&RCC_OscInitStruct);
	if (status != HAL_OK)
		return status;

	/* Initializes the CPU, AHB and APB busses clocks */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

	status = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);
	if (status != HAL_OK)
		return status;

	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB|RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_I2C1;
	PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
	PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
	PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;

	status = HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
	if (status != HAL_OK)
		return status;

	/* Configure the Systick interrupt time */
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

	/* Configure the Systick */
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	/* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn, 3, 0);

	return status;
}

HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
	RCC_ClkInitTypeDef clkconfig;
	HAL_StatusTypeDef status;
	uint32_t flash_latency;

	HAL_NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn, TickPriority ,0);
	HAL_NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);

	__HAL_RCC_TIM1_CLK_ENABLE();

	HAL_RCC_GetClockConfig(&clkconfig, &flash_latency);

	const uint32_t clock_freq = HAL_RCC_GetPCLK1Freq();
	const uint32_t prescaler = (uint32_t) ((clock_freq / 1000000) - 1);

	/* Initialize TIMx peripheral as follow:
	 * Period = [(TIM1CLK/1000) - 1]. to have a (1/1000) s time base.
	 * Prescaler = (uwTimclock/1000000 - 1) to have a 1MHz counter clock.
	 * ClockDivision = 0
	 * Counter direction = Up
	 */
	tim_handle.Instance = TIM1;
	tim_handle.Init.Period = (1000000 / 1000) - 1;
	tim_handle.Init.Prescaler = prescaler;
	tim_handle.Init.ClockDivision = 0;
	tim_handle.Init.CounterMode = TIM_COUNTERMODE_UP;

	status = HAL_TIM_Base_Init(&tim_handle);
	if (status != HAL_OK)
		return status;

	status = HAL_TIM_Base_Start_IT(&tim_handle);
	if (status != HAL_OK)
		return status;

	return status;
}

void HAL_SuspendTick(void)
{
	__HAL_TIM_DISABLE_IT(&tim_handle, TIM_IT_UPDATE);
}

void HAL_ResumeTick(void)
{
	__HAL_TIM_ENABLE_IT(&tim_handle, TIM_IT_UPDATE);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM1) {
		HAL_IncTick();
	}
}

void HAL_MspInit(void)
{
	__HAL_RCC_SYSCFG_CLK_ENABLE();
	//__HAL_RCC_PWR_CLK_ENABLE(); // wat dis?

	HAL_NVIC_SetPriority(SVC_IRQn, 0, 0);
	HAL_NVIC_SetPriority(PendSV_IRQn, 3, 0);
	HAL_NVIC_SetPriority(SysTick_IRQn, 3, 0);

	HAL_DBGMCU_EnableDBGStopMode();
	HAL_DBGMCU_EnableDBGStandbyMode();
}

void SysTick_Handler(void)
{
#if (INCLUDE_xTaskGetSchedulerState  == 1 )
	if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
#endif
		xPortSysTickHandler();
#if (INCLUDE_xTaskGetSchedulerState  == 1 )
	}
#endif
}

void TIM1_BRK_UP_TRG_COM_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&tim_handle);
}

static void jump_to_bootloader(void)
{
	const uint32_t jump_address = *(__IO uint32_t*)(SYSMEM_ADDRESS + 4);

	HAL_RCC_DeInit();
	HAL_DeInit();

	SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL = 0;

	__HAL_SYSCFG_REMAPMEMORY_SYSTEMFLASH();

	__set_MSP(*(__IO uint32_t*)SYSMEM_ADDRESS);

	((void (*)(void)) jump_address)();

	while(1);
}

void platform_reboot_to_dfu(void)
{
	persistent_data.reboot_to_bootloader = true;
	platform_reset();
}

err_t platform_init(void)
{
	HAL_StatusTypeDef status;

	if (persistent_data.reboot_to_bootloader) {
		persistent_data.reboot_to_bootloader = false;
		jump_to_bootloader();
	}

	status = HAL_Init();
	HAL_ERR_CHECK(status, EPLATFORM_INIT);

	status = SystemClock_Config();
	HAL_ERR_CHECK(status, EPLATFORM_INIT);

	return ERR_OK;
}

void platform_delay_us(uint32_t delay_us) {
	const uint64_t ticks = ((uint64_t) delay_us) * ((uint64_t) HAL_RCC_GetSysClockFreq()) / 1000000;
	register uint32_t loops = ticks / 6; /* 6 cycles per loop */

	while (loops--) {
		__ASM("nop");
	}
}

void platform_reset(void)
{
	HAL_NVIC_SystemReset();
}
