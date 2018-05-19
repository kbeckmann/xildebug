#include "stm32l4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

static TIM_HandleTypeDef tim_handle;
extern void xPortSysTickHandler(void);

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
