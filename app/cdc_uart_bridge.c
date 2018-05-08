#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <queue.h>

#include "cdc_uart_bridge.h"
#include "drivers/led.h"
#include "drivers/max14662.h"
#include "drivers/uart.h"
#include "drivers/usb/cdc.h"
#include "stm32l4xx_hal.h"

#define RX_TASK_STACK_SIZE		512
#define RX_TASK_NAME			"CDCrx"
#define RX_TASK_PRIORITY		1

#define TX_TASK_STACK_SIZE		512
#define TX_TASK_NAME			"CDCtx"
#define TX_TASK_PRIORITY		1

#define QUEUE_LENGTH			10
#define QUEUE_ITEM_SIZE			sizeof(struct usb_rx_queue_item)

#define LED_TIMEOUT_MS			25
#define UART_RX_TIMEOUT_MS		50

/* Fun test string: 
</////////////////////////////////////////////////////////////>[##################]
<2345678901234567890123456789012345678901234567890123456789012>[ABCDEFGHIJKLMNOPQR]
*/

static struct {
	bool initialized;
	StackType_t rx_task_stack[RX_TASK_STACK_SIZE];
	TaskHandle_t rx_task_handle;
	StaticTask_t rx_task_tcb;
	StackType_t tx_task_stack[RX_TASK_STACK_SIZE];
	TaskHandle_t tx_task_handle;
	StaticTask_t tx_task_tcb;
	StaticQueue_t tx_queue;
	QueueHandle_t tx_queue_handle;
	uint8_t tx_queue_storage[QUEUE_LENGTH * QUEUE_ITEM_SIZE];
	TimerHandle_t rx_led_timer;
	StaticTimer_t rx_led_timer_storage;
	TimerHandle_t tx_led_timer;
	StaticTimer_t tx_led_timer_storage;
} self;

static void rx_task(void *p_arg)
{
	struct usb_rx_queue_item rx_queue_item;
	err_t r;

	for (;;) {
		r = usb_cdc_rx(&rx_queue_item, portMAX_DELAY);
		if (r != ERR_OK) {
			while (1)
				;
		}

		led_tx_set(true);
		xTimerReset(self.tx_led_timer, 0);

		r = uart_tx(rx_queue_item.data, rx_queue_item.len, portMAX_DELAY, true);
		if (r != ERR_OK) {
			while (1)
				;
		}
	}
}

static void tx_task(void *p_arg)
{
	err_t r;
	struct usb_rx_queue_item item;

	for (;;) {
		/* uart_start_rx starts a continuous DMA transfer of 64 bytes that sends its data to our 
		 * queue that we receive here. In order to get shorter messages in near realtime we need
		 * a timeout and flush the received bytes so far. */
		if (xQueueReceive(self.tx_queue_handle, &item, pdMS_TO_TICKS(UART_RX_TIMEOUT_MS)) == pdFALSE) {
			uart_flush_rx();
			continue;
		}

		led_rx_set(true);
		xTimerReset(self.rx_led_timer, 0);

		r = usb_cdc_tx(item.data, item.len);
		if (r != ERR_OK && r != EUSB_CDC_NOT_READY)
			while (1)
				;
	}
}

static void timer_callback(TimerHandle_t timer_handle)
{
	if (timer_handle == self.tx_led_timer) {
		led_tx_set(false);
	} else if (timer_handle == self.rx_led_timer) {
		led_rx_set(false);
	}
}

err_t cdc_uart_bridge_init(void)
{
	err_t r = ERR_OK;

	if (self.initialized)
		return ERR_OK;

	self.rx_task_handle = xTaskCreateStatic(
		rx_task,
		RX_TASK_NAME,
		RX_TASK_STACK_SIZE,
		NULL,
		RX_TASK_PRIORITY,
		&self.rx_task_stack[0],
		&self.rx_task_tcb);
	if (self.rx_task_handle == NULL)
		return ECDC_UART_BRIDGE_TASK_CREATE;

	self.tx_task_handle = xTaskCreateStatic(
		tx_task,
		TX_TASK_NAME,
		TX_TASK_STACK_SIZE,
		NULL,
		TX_TASK_PRIORITY,
		&self.tx_task_stack[0],
		&self.tx_task_tcb);
	if (self.tx_task_handle == NULL)
		return ECDC_UART_BRIDGE_TASK_CREATE;

	self.tx_queue_handle = xQueueCreateStatic(QUEUE_LENGTH,
		QUEUE_ITEM_SIZE,
		self.tx_queue_storage,
		&self.tx_queue);

	self.tx_led_timer = xTimerCreateStatic(
		"tx_led",
		pdMS_TO_TICKS(LED_TIMEOUT_MS),
		pdFALSE,
		( void * ) 0,
		timer_callback,
		&self.tx_led_timer_storage);

	self.rx_led_timer = xTimerCreateStatic(
		"rx_led",
		pdMS_TO_TICKS(LED_TIMEOUT_MS),
		pdFALSE,
		( void * ) 0,
		timer_callback,
		&self.rx_led_timer_storage);

	// TODO: enable only rx and tx
	r = max14662_set_value(MAX14662_AD_0_0, 0xff);
	ERR_CHECK(r);

	r = uart_start_rx(self.tx_queue_handle);
	ERR_CHECK(r);

	self.initialized = true;

	return r;
}
