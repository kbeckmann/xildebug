#include "stm32l4xx_hal.h"
#include "FreeRTOS.h"
#include "platform/platform.h"
#include "task.h"

static TIM_HandleTypeDef tim_handle;
extern void xPortSysTickHandler(void);

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
		.PLL.PLLM = 1,
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

HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
	RCC_ClkInitTypeDef clkconfig;
	HAL_StatusTypeDef status;
	uint32_t flash_latency;

	HAL_NVIC_SetPriority(TIM1_UP_TIM16_IRQn, TickPriority ,0);
	HAL_NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);

	__HAL_RCC_TIM1_CLK_ENABLE();

	HAL_RCC_GetClockConfig(&clkconfig, &flash_latency);

	const uint32_t clock_freq = HAL_RCC_GetPCLK2Freq();
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
	__HAL_RCC_PWR_CLK_ENABLE();

	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

	/* MemoryManagement_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(MemoryManagement_IRQn, 0, 0);

	/* BusFault_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(BusFault_IRQn, 0, 0);

	/* UsageFault_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(UsageFault_IRQn, 0, 0);

	/* SVCall_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SVCall_IRQn, 0, 0);

	/* DebugMonitor_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DebugMonitor_IRQn, 0, 0);

	/* PendSV_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0);

	/* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);

	/* ADC1_2_IRQn interrupt configuration
	 * TODO: Review priorities when USB is in place.
	 */
	HAL_NVIC_SetPriority(ADC1_2_IRQn, 5, 0);

	HAL_DBGMCU_EnableDBGSleepMode();
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

void TIM1_UP_TIM16_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&tim_handle);
}

err_t platform_init(void)
{
	HAL_StatusTypeDef status;

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
