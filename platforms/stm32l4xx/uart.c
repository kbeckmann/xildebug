#include <FreeRTOS.h>
#include <semphr.h>
#include <queue.h>
#include <task.h>

#include "drivers/max14662.h"
#include "hal_errors.h"
#include "platform/gpio.h"
#include "platform/platform.h"
#include "platform/uart.h"
#include "platform/usb/cdc.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_ll_dma.h"

#define MODULE_NAME				uart
#include "macros.h"

static struct {
	bool enabled;
	bool initialized;

	DMA_HandleTypeDef hdma_usart3_tx;
	DMA_HandleTypeDef hdma_usart3_rx;
	UART_HandleTypeDef uart_handle;

	struct usb_rx_queue_item rx_item;
	QueueHandle_t rx_queue_handle;

	SemaphoreHandle_t tx_busy_semaphore;
	StaticSemaphore_t tx_busy_semaphore_buffer;
	SemaphoreHandle_t tx_done_semaphore;
	StaticSemaphore_t tx_done_semaphore_buffer;

	SemaphoreHandle_t rx_busy_semaphore;
	StaticSemaphore_t rx_busy_semaphore_buffer;
	SemaphoreHandle_t rx_done_semaphore;
	StaticSemaphore_t rx_done_semaphore_buffer;
} SELF;

void HAL_UART_MspInit(UART_HandleTypeDef *p_handle)
{
	GPIO_InitTypeDef gpio_config;
	HAL_StatusTypeDef status;

	if (p_handle->Instance == USART3) {
		__HAL_RCC_USART3_CLK_ENABLE();
		__HAL_RCC_DMA1_CLK_ENABLE();

		gpio_config.Pin = DUT_UART_TX_Pin|DUT_UART_RX_Pin;
		gpio_config.Mode = GPIO_MODE_AF_PP;
		gpio_config.Pull = GPIO_PULLUP;
		gpio_config.Speed = GPIO_SPEED_FREQ_LOW;
		gpio_config.Alternate = GPIO_AF7_USART3;
		HAL_GPIO_Init(GPIOB, &gpio_config);
		
		SELF.hdma_usart3_tx.Instance = DMA1_Channel2;
		SELF.hdma_usart3_tx.Init.Request = DMA_REQUEST_2;
		SELF.hdma_usart3_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
		SELF.hdma_usart3_tx.Init.PeriphInc = DMA_PINC_DISABLE;
		SELF.hdma_usart3_tx.Init.MemInc = DMA_MINC_ENABLE;
		SELF.hdma_usart3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		SELF.hdma_usart3_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		SELF.hdma_usart3_tx.Init.Mode = DMA_NORMAL;
		SELF.hdma_usart3_tx.Init.Priority = DMA_PRIORITY_LOW;
		status = HAL_DMA_Init(&SELF.hdma_usart3_tx);
		if (status != HAL_OK)
			for (;;);

		__HAL_LINKDMA(p_handle, hdmatx, SELF.hdma_usart3_tx);

		/* USART3_RX Init */
		SELF.hdma_usart3_rx.Instance = DMA1_Channel3;
		SELF.hdma_usart3_rx.Init.Request = DMA_REQUEST_2;
		SELF.hdma_usart3_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
		SELF.hdma_usart3_rx.Init.PeriphInc = DMA_PINC_DISABLE;
		SELF.hdma_usart3_rx.Init.MemInc = DMA_MINC_ENABLE;
		SELF.hdma_usart3_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		SELF.hdma_usart3_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		SELF.hdma_usart3_rx.Init.Mode = DMA_NORMAL;
		SELF.hdma_usart3_rx.Init.Priority = DMA_PRIORITY_LOW;
		status = HAL_DMA_Init(&SELF.hdma_usart3_rx);
		if (status != HAL_OK)
			for (;;);

		__HAL_LINKDMA(p_handle, hdmarx, SELF.hdma_usart3_rx);

		/* USART3 interrupt Init */
		HAL_NVIC_SetPriority(USART3_IRQn, 5, 0);
		HAL_NVIC_EnableIRQ(USART3_IRQn);

		/* DMA interrupt init */
		/* DMA1_Channel2_IRQn interrupt configuration */
		HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 5, 0);
		HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
		/* DMA1_Channel3_IRQn interrupt configuration */
		HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 5, 0);
		HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
	}
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *p_handle)
{
	if (p_handle->Instance == USART3) {
		__HAL_RCC_DMA1_CLK_DISABLE();
		__HAL_RCC_USART3_CLK_DISABLE();

		HAL_GPIO_DeInit(GPIOB, DUT_UART_TX_Pin|DUT_UART_RX_Pin);
		HAL_DMA_DeInit(p_handle->hdmatx);
		HAL_DMA_DeInit(p_handle->hdmarx);
		HAL_NVIC_DisableIRQ(USART3_IRQn);
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *p_uart)
{
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	xSemaphoreGiveFromISR(SELF.tx_done_semaphore, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *p_uart)
{
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	SELF.rx_item.len = USB_FS_MAX_PACKET_SIZE;
	xQueueSendFromISR(SELF.rx_queue_handle, &SELF.rx_item, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	HAL_UART_Receive_DMA(&SELF.uart_handle, (uint8_t*) SELF.rx_item.data, USB_FS_MAX_PACKET_SIZE);
}

void HAL_UART_AbortReceiveCpltCallback(UART_HandleTypeDef *p_uart)
{
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	SELF.rx_item.len = USB_FS_MAX_PACKET_SIZE - LL_DMA_GetDataLength(DMA1, LL_DMA_CHANNEL_3);

	if (SELF.rx_item.len) {
		xQueueSendFromISR(SELF.rx_queue_handle, &SELF.rx_item, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}

	HAL_UART_Receive_DMA(&SELF.uart_handle, (uint8_t*) SELF.rx_item.data, USB_FS_MAX_PACKET_SIZE);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	platform_force_hardfault();
}

void USART3_IRQHandler(void)
{
	HAL_UART_IRQHandler(&SELF.uart_handle);
}

void DMA1_Channel2_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&SELF.hdma_usart3_tx);
}

void DMA1_Channel3_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&SELF.hdma_usart3_rx);
}

err_t uart_tx(const uint8_t *p_buf, uint32_t size, uint32_t timeout_ticks, bool blocking)
{
	HAL_StatusTypeDef status;
	err_t r = ERR_OK;

	if (!SELF.initialized)
		return EUART_NO_INIT;

	if (!SELF.enabled)
		return EUART_DISABLED;

	if (xSemaphoreTake(SELF.tx_busy_semaphore, timeout_ticks) == pdFALSE)
		return EUART_TX_SEMPH;

	if (xSemaphoreTake(SELF.tx_done_semaphore, timeout_ticks) == pdFALSE) {
		r = EUART_TX_SEMPH;
		goto out;
	}

	/* Ugly const-to-non-const cast because the HAL is annoying. */
	status = HAL_UART_Transmit_DMA(&SELF.uart_handle, (uint8_t*) p_buf, size);
	if (status != HAL_OK) {
		r = HAL_ERROR_SET(status, EUART_TX);
		/* Give the semaphore in case there are errors, as the callback will not be called */
		xSemaphoreGive(SELF.tx_done_semaphore);
		goto out;
	}

	if (blocking) {
		if (xSemaphoreTake(SELF.tx_done_semaphore, timeout_ticks) == pdFALSE) {
			r = EUART_TX_TIMEOUT;
			goto out;
		}
		xSemaphoreGive(SELF.tx_done_semaphore);
	}

out:
	xSemaphoreGive(SELF.tx_busy_semaphore);
	return r;
}

err_t uart_start_rx(QueueHandle_t queue)
{
	HAL_StatusTypeDef status;

	if (!SELF.initialized)
		return EUART_NO_INIT;

	if (!SELF.enabled)
		return EUART_DISABLED;

	SELF.rx_queue_handle = queue;
	status = HAL_UART_Receive_DMA(&SELF.uart_handle, (uint8_t*) SELF.rx_item.data, USB_FS_MAX_PACKET_SIZE);
	HAL_ERR_CHECK(status, EUART_RX);

	return ERR_OK;
}

err_t uart_flush_rx(void)
{
	HAL_StatusTypeDef status;

	if (!SELF.initialized)
		return EUART_NO_INIT;

	if (!SELF.enabled)
		return EUART_DISABLED;

	/* HAL_UART_AbortReceiveCpltCallback restarts DMA */
	status = HAL_UART_AbortReceive_IT(&SELF.uart_handle);
	HAL_ERR_CHECK(status, EUART_FLUSH_RX);

	return ERR_OK;
}

static err_t parse_config(UART_InitTypeDef *p_dest, const struct uart_line_coding * const p_src)
{
	uint32_t value;

	p_dest->BaudRate = p_src->baudrate_bps;

	switch (p_src->databits) {
	case 7:
		value = UART_WORDLENGTH_7B;
		break;
	case 8:
		value = UART_WORDLENGTH_8B;
		break;
	default:
		return EUART_INVALID_ARG;
	};
	p_dest->WordLength = value;

	switch (p_src->stopbits) {
	case UART_STOPBITS_CONFIG_1:
		value = UART_STOPBITS_1;
		break;
	case UART_STOPBITS_CONFIG_1_5:
		value = UART_STOPBITS_1_5;
		break;
	case UART_STOPBITS_CONFIG_2:
		value = UART_STOPBITS_2;
		break;
	default:
		return EUART_INVALID_ARG;
	};
	p_dest->StopBits = value;

	switch (p_src->parity) {
	case UART_PARITY_CONFIG_NONE:
		value = UART_PARITY_NONE;
		break;
	case UART_PARITY_CONFIG_ODD:
		value = UART_PARITY_ODD;
		break;
	case UART_PARITY_CONFIG_EVEN:
		value = UART_PARITY_EVEN;
		break;
	default:
		return EUART_INVALID_ARG;
	};
	p_dest->Parity = value;

	p_dest->Mode = UART_MODE_TX_RX;
	p_dest->HwFlowCtl = UART_HWCONTROL_NONE;
	p_dest->OverSampling = UART_OVERSAMPLING_16;
	p_dest->OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;

	return ERR_OK;
}

err_t uart_config_set(const struct uart_line_coding * const p_config)
{
	UART_InitTypeDef hal_config;
	HAL_StatusTypeDef status;
	err_t r;
	
	if (!SELF.initialized)
		return EUART_NO_INIT;

	if (!SELF.enabled)
		return EUART_DISABLED;

	r = parse_config(&hal_config, p_config);
	ERR_CHECK(r);

	SELF.uart_handle.Init = hal_config;
	status = UART_SetConfig(&SELF.uart_handle);
	HAL_ERR_CHECK(status, EUART_HAL_INIT);

	return ERR_OK;
}

err_t uart_enable(void)
{
	err_t r;

	if (!SELF.initialized)
		return EUART_NO_INIT;
	
	if (SELF.enabled)
		return ERR_OK;

	r = max14662_set_bit(MAX14662_AD_0_0, MAX14662_BIT_UART_RX, true);
	ERR_CHECK(r);

	r = max14662_set_bit(MAX14662_AD_0_0, MAX14662_BIT_UART_TX, true);
	ERR_CHECK(r);

	SELF.enabled = true;

	return ERR_OK;
}

err_t uart_disable(void)
{
	err_t r;

	if (!SELF.initialized)
		return EUART_NO_INIT;

	if (!SELF.enabled)
		return ERR_OK;

	r = max14662_set_bit(MAX14662_AD_0_0, MAX14662_BIT_UART_RX, false);
	ERR_CHECK(r);

	r = max14662_set_bit(MAX14662_AD_0_0, MAX14662_BIT_UART_TX, false);
	ERR_CHECK(r);

	SELF.enabled = false;

	return ERR_OK;
}

err_t uart_init(void)
{
	HAL_StatusTypeDef status;

	SELF.uart_handle.Instance = USART3;
	SELF.uart_handle.Init.BaudRate = 115200;
	SELF.uart_handle.Init.WordLength = UART_WORDLENGTH_8B;
	SELF.uart_handle.Init.StopBits = UART_STOPBITS_1;
	SELF.uart_handle.Init.Parity = UART_PARITY_NONE;
	SELF.uart_handle.Init.Mode = UART_MODE_TX_RX;
	SELF.uart_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	SELF.uart_handle.Init.OverSampling = UART_OVERSAMPLING_16;
	SELF.uart_handle.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	SELF.uart_handle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

	status = HAL_UART_Init(&SELF.uart_handle);
	HAL_ERR_CHECK(status, EUART_HAL_INIT);

	SELF.tx_busy_semaphore = xSemaphoreCreateMutexStatic(&SELF.tx_busy_semaphore_buffer);
	xSemaphoreGive(SELF.tx_busy_semaphore);
	SELF.tx_done_semaphore = xSemaphoreCreateBinaryStatic(&SELF.tx_done_semaphore_buffer);
	xSemaphoreGive(SELF.tx_done_semaphore);

	SELF.rx_busy_semaphore = xSemaphoreCreateMutexStatic(&SELF.rx_busy_semaphore_buffer);
	xSemaphoreGive(SELF.rx_busy_semaphore);
	SELF.rx_done_semaphore = xSemaphoreCreateBinaryStatic(&SELF.rx_done_semaphore_buffer);
	xSemaphoreGive(SELF.rx_done_semaphore);

	SELF.initialized = true;

	return ERR_OK;
}
