#include <FreeRTOS.h>
#include <semphr.h>
#include <queue.h>

#include "drivers/gpio.h"
#include "drivers/uart.h"

DMA_HandleTypeDef hdma_usart3_tx;
DMA_HandleTypeDef hdma_usart3_rx;
UART_HandleTypeDef uart_handle;

SemaphoreHandle_t tx_busy_semaphore;
StaticSemaphore_t tx_busy_semaphore_buffer;

// TODO: Add uart_rx_dma

UART_HandleTypeDef *uart_get_handle(void)
{
	return &uart_handle;
}

void HAL_UART_MspInit(UART_HandleTypeDef *p_handle)
{
	GPIO_InitTypeDef gpio_config;
	HAL_StatusTypeDef status;

	if (p_handle->Instance == USART3) {
		__HAL_RCC_USART3_CLK_ENABLE();
		__HAL_RCC_DMA1_CLK_ENABLE();

		gpio_config.Pin = DUT_UART_TX_Pin|DUT_UART_RX_Pin;
		gpio_config.Mode = GPIO_MODE_AF_PP;
		gpio_config.Pull = GPIO_NOPULL;
		gpio_config.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		gpio_config.Alternate = GPIO_AF7_USART3;
		HAL_GPIO_Init(GPIOB, &gpio_config);
		
		hdma_usart3_tx.Instance = DMA1_Channel2;
		hdma_usart3_tx.Init.Request = DMA_REQUEST_2;
		hdma_usart3_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
		hdma_usart3_tx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_usart3_tx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_usart3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_usart3_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_usart3_tx.Init.Mode = DMA_NORMAL;
		hdma_usart3_tx.Init.Priority = DMA_PRIORITY_LOW;
		status = HAL_DMA_Init(&hdma_usart3_tx);
		if (status != HAL_OK)
			for (;;);

		__HAL_LINKDMA(p_handle, hdmatx, hdma_usart3_tx);

		/* USART3_RX Init */
		hdma_usart3_rx.Instance = DMA1_Channel3;
		hdma_usart3_rx.Init.Request = DMA_REQUEST_2;
		hdma_usart3_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
		hdma_usart3_rx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_usart3_rx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_usart3_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_usart3_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_usart3_rx.Init.Mode = DMA_NORMAL;
		hdma_usart3_rx.Init.Priority = DMA_PRIORITY_LOW;
		status = HAL_DMA_Init(&hdma_usart3_rx);
		if (status != HAL_OK)
			for (;;);

		__HAL_LINKDMA(p_handle, hdmarx, hdma_usart3_rx);

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

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	xSemaphoreGiveFromISR(tx_busy_semaphore, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	// TODO
}

void USART3_IRQHandler(void)
{
	HAL_UART_IRQHandler(&uart_handle);
}

void DMA1_Channel2_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&hdma_usart3_tx);
}

void DMA1_Channel3_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&hdma_usart3_rx);
}

err_t uart_tx(UART_HandleTypeDef *p_handle, const uint8_t *p_buf, uint32_t size, uint32_t timeout_ms)
{
	HAL_StatusTypeDef status;

	if (xSemaphoreTake(tx_busy_semaphore, pdMS_TO_TICKS(timeout_ms)) == pdFALSE)
		return EUART_TX_SEMPH;

	/* Ugly const-to-non-const cast because the HAL is annoying. */
	status = HAL_UART_Transmit_DMA(p_handle, (uint8_t*) p_buf, size);
	HAL_ERR_CHECK(status, EUART_TX);

	return ERR_OK;
}

err_t uart_rx(UART_HandleTypeDef *p_handle, uint8_t *p_buf, uint32_t size, uint32_t timeout_ms)
{
	HAL_StatusTypeDef status;

	status = HAL_UART_Receive(p_handle, (uint8_t*) p_buf, size, timeout_ms);
	HAL_ERR_CHECK(status, EUART_RX);

	return ERR_OK;
}

err_t uart_init(void)
{
	HAL_StatusTypeDef status;

	uart_handle.Instance = USART3;
	uart_handle.Init.BaudRate = 115200;
	uart_handle.Init.WordLength = UART_WORDLENGTH_8B;
	uart_handle.Init.StopBits = UART_STOPBITS_1;
	uart_handle.Init.Parity = UART_PARITY_NONE;
	uart_handle.Init.Mode = UART_MODE_TX_RX;
	uart_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	uart_handle.Init.OverSampling = UART_OVERSAMPLING_16;
	uart_handle.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	uart_handle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

	status = HAL_UART_Init(&uart_handle);
	HAL_ERR_CHECK(status, EUART_HAL_INIT);

	tx_busy_semaphore = xSemaphoreCreateBinaryStatic(&tx_busy_semaphore_buffer);

	xSemaphoreGive(tx_busy_semaphore);

	return ERR_OK;
}
