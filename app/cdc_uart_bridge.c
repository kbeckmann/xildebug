#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include "drivers/max14662.h"
#include "drivers/uart.h"
#include "drivers/usb/cdc.h"
#include "stm32l4xx_hal.h"
#include "cdc_uart_bridge.h"

#define CDC_RX_TASK_STACK_SIZE		512
#define CDC_RX_TASK_NAME			"CDCrx"
#define CDC_RX_TASK_PRIORITY		1

#define CDC_TX_TASK_STACK_SIZE		512
#define CDC_TX_TASK_NAME			"CDCtx"
#define CDC_TX_TASK_PRIORITY		1

/* Fun test string: 
</////////////////////////////////////////////////////////////>[##################]
<2345678901234567890123456789012345678901234567890123456789012>[ABCDEFGHIJKLMNOPQR]
*/

#define QUEUE_LENGTH    10
#define ITEM_SIZE       sizeof(struct rx_queue_item)

static struct {
	bool initialized;
	StackType_t cdc_rx_task_stack[CDC_RX_TASK_STACK_SIZE];
	TaskHandle_t cdc_rx_task_handle;
	StaticTask_t cdc_rx_task_tcb;
	StackType_t cdc_tx_task_stack[CDC_RX_TASK_STACK_SIZE];
	TaskHandle_t cdc_tx_task_handle;
	StaticTask_t cdc_tx_task_tcb;
	StaticQueue_t cdc_tx_queue;
	QueueHandle_t cdc_tx_queue_handle;
	uint8_t cdc_tx_queue_storage[QUEUE_LENGTH * ITEM_SIZE];
	struct rx_queue_item cdc_rx_queue_item;
} self;

static void cdc_rx_task(void *p_arg)
{
	err_t r;

	for (;;) {
		r = usb_cdc_rx(&self.cdc_rx_queue_item, portMAX_DELAY);
		if (r != ERR_OK) {
			while (1)
				;
		}
		r = uart_tx(self.cdc_rx_queue_item.data, self.cdc_rx_queue_item.len, portMAX_DELAY, true);
		if (r != ERR_OK) {
			while (1)
				;
		}
	}
}

static void cdc_tx_task(void *p_arg)
{
	err_t r;
	struct rx_queue_item item;

	for (;;) {
		// TODO: Timeout every 100ms or so, and stop DMA and check number of bytes written, and forward them
		if (xQueueReceive(self.cdc_tx_queue_handle, &item, portMAX_DELAY) == pdFALSE)
			while(1)
				;

		r = usb_cdc_tx(item.data, item.len);
		if (r != ERR_OK)
			while (1)
				;
	}
}

err_t cdc_uart_bridge_init(void)
{
	err_t r = ERR_OK;

	if (self.initialized)
		return ERR_OK;

	self.cdc_rx_task_handle = xTaskCreateStatic(
		cdc_rx_task,
		CDC_RX_TASK_NAME,
		CDC_RX_TASK_STACK_SIZE,
		NULL,
		CDC_RX_TASK_PRIORITY,
		&self.cdc_rx_task_stack[0],
		&self.cdc_rx_task_tcb);
	if (self.cdc_rx_task_handle == NULL)
		return ECDC_UART_BRIDGE_TASK_CREATE;

	self.cdc_tx_task_handle = xTaskCreateStatic(
		cdc_tx_task,
		CDC_TX_TASK_NAME,
		CDC_TX_TASK_STACK_SIZE,
		NULL,
		CDC_TX_TASK_PRIORITY,
		&self.cdc_tx_task_stack[0],
		&self.cdc_tx_task_tcb);
	if (self.cdc_tx_task_handle == NULL)
		return ECDC_UART_BRIDGE_TASK_CREATE;

	self.cdc_tx_queue_handle = xQueueCreateStatic(QUEUE_LENGTH,
								ITEM_SIZE,
								self.cdc_tx_queue_storage,
								&self.cdc_tx_queue);

	// TODO: enable only rx and tx
	max14662_set_value(MAX14662_AD_0_0, 0xff);

	self.initialized = true;

	//r = uart_start_rx(self.cdc_tx_queue_handle);

	return r;
}
