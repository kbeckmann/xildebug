#include <FreeRTOS.h>
#include <semphr.h>
#include <stdbool.h>

#include "drivers/usb_cdc.h"
#include "drivers/usb.h"
#include "drivers/uart.h"

#define APP_RX_DATA_SIZE  1000
#define APP_TX_DATA_SIZE  1000

static struct {
	bool initialized;
	USBD_HandleTypeDef *usbd_handle;
	uint8_t rx_buff[APP_RX_DATA_SIZE];
	uint8_t tx_buff[APP_TX_DATA_SIZE];
	uint16_t rx_len;
	SemaphoreHandle_t rx_done_semaphore;
	StaticSemaphore_t rx_done_semaphore_buffer;
} self;

static int8_t usb_cdc_if_init(void);
static int8_t usb_cdc_if_deinit(void);
static int8_t usb_cdc_ctrl(uint8_t cmd, uint8_t *p_buf, uint16_t len);
static int8_t usb_cdc_rx_irq(uint8_t *p_buf, uint32_t *p_len);

static USBD_CDC_ItfTypeDef usb_cdc_if_ops =
{
	usb_cdc_if_init,
	usb_cdc_if_deinit,
	usb_cdc_ctrl,
	usb_cdc_rx_irq
};

static int8_t usb_cdc_if_init(void)
{
	USBD_CDC_SetTxBuffer(self.usbd_handle, self.tx_buff, 0);
	USBD_CDC_SetRxBuffer(self.usbd_handle, self.rx_buff);

	return USBD_OK;
}

static int8_t usb_cdc_if_deinit(void)
{
	return USBD_OK;
}

static int8_t usb_cdc_ctrl(uint8_t cmd, uint8_t *p_buf, uint16_t len)
{
	switch(cmd) {
	case CDC_SEND_ENCAPSULATED_COMMAND:
		break;

	case CDC_GET_ENCAPSULATED_RESPONSE:
		break;

	case CDC_SET_COMM_FEATURE:
		break;

	case CDC_GET_COMM_FEATURE:
		break;

	case CDC_CLEAR_COMM_FEATURE:
		break;

		/*******************************************************************************/
		/* Line Coding Structure                                                       */
		/*-----------------------------------------------------------------------------*/
		/* Offset | Field       | Size | Value  | Description                          */
		/* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
		/* 4      | bCharFormat |   1  | Number | Stop bits                            */
		/*                                        0 - 1 Stop bit                       */
		/*                                        1 - 1.5 Stop bits                    */
		/*                                        2 - 2 Stop bits                      */
		/* 5      | bParityType |  1   | Number | Parity                               */
		/*                                        0 - None                             */
		/*                                        1 - Odd                              */
		/*                                        2 - Even                             */
		/*                                        3 - Mark                             */
		/*                                        4 - Space                            */
		/* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
		/*******************************************************************************/
	case CDC_SET_LINE_CODING:
		break;

	case CDC_GET_LINE_CODING:
		break;

	case CDC_SET_CONTROL_LINE_STATE:
		break;

	case CDC_SEND_BREAK:
		break;

	default:
		break;
	}

	return USBD_OK;
}

static int8_t usb_cdc_rx_irq(uint8_t *p_buf, uint32_t *p_len)
{
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	USBD_CDC_SetRxBuffer(self.usbd_handle, &p_buf[0]);
	USBD_CDC_ReceivePacket(self.usbd_handle);

	self.rx_len = *p_len;

	xSemaphoreGiveFromISR(self.rx_done_semaphore, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

	return USBD_OK;
}

err_t usb_cdc_rx(const uint8_t **pp_buf, uint16_t *p_len, uint32_t timeout_ticks)
{
	if (!self.initialized)
		return EUSB_CDC_NO_INIT;

	if (!pp_buf || !p_len)
		return EUSB_CDC_INVALID_ARG;

	if (xSemaphoreTake(self.rx_done_semaphore, timeout_ticks) == pdFALSE)
		return EUSB_CDC_RX_TIMEOUT;

	*pp_buf = self.rx_buff;
	*p_len = self.rx_len;

	return ERR_OK;
}

err_t usb_cdc_tx(uint8_t *p_buf, uint16_t len)
{
	USBD_StatusTypeDef status;

	if (!self.initialized)
		return EUSB_CDC_NO_INIT;

	USBD_CDC_HandleTypeDef *p_cdc = (USBD_CDC_HandleTypeDef*)self.usbd_handle->pClassData;
	if (p_cdc->TxState != 0)
		return EUSB_CDC_BUSY;

	USBD_CDC_SetTxBuffer(self.usbd_handle, p_buf, len);
	status = USBD_CDC_TransmitPacket(self.usbd_handle);
	if (status != USBD_OK)
		return EUSB_CDC_TRANSMIT;

	return ERR_OK;
}

err_t usb_cdc_init(USBD_HandleTypeDef *p_usbd)
{
	USBD_StatusTypeDef status;

	if (self.initialized)
		return ERR_OK;

	self.usbd_handle = p_usbd;

	USBD_CDC.GetUsrStrDescriptor = usb_desc_usr_str;

	status = USBD_RegisterClass(self.usbd_handle, &USBD_CDC);
	if (status != USBD_OK)
		return EUSB_CDC_REG_CLASS;

	status = USBD_CDC_RegisterInterface(self.usbd_handle, &usb_cdc_if_ops);
	if (status != USBD_OK)
		return EUSB_CDC_REG_IF;

	self.rx_done_semaphore = xSemaphoreCreateBinaryStatic(&self.rx_done_semaphore_buffer);

	self.initialized = true;

	return ERR_OK;
}
