#include "FreeRTOS.h"
#include "task.h"

#define IDLE_TASK_STACK_SIZE	configMINIMAL_STACK_SIZE
#define TIMER_TASK_STACK_SIZE	configTIMER_TASK_STACK_DEPTH

static StaticTask_t idle_task_tcb;
static StackType_t idle_task_stack[IDLE_TASK_STACK_SIZE];

static StaticTask_t timer_task_tcb;
static StackType_t timer_task_stack[TIMER_TASK_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize)
{
	*ppxIdleTaskTCBBuffer = &idle_task_tcb;
	*ppxIdleTaskStackBuffer = &idle_task_stack[0];
	*pulIdleTaskStackSize = IDLE_TASK_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
	*ppxTimerTaskTCBBuffer = &timer_task_tcb;
	*ppxTimerTaskStackBuffer = &timer_task_stack[0];
	*pulTimerTaskStackSize = TIMER_TASK_STACK_SIZE;
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
	while(1)
		;
}

__attribute__((weak))
void configureTimerForRunTimeStats(void)
{

}

__attribute__((weak))
unsigned long getRunTimeCounterValue(void)
{
	return 0;
}
