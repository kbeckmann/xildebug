#include <FreeRTOS.h>
#include <semphr.h>
#include <stdbool.h>
#include <string.h>

#include "cdc_internal.h"
#include "hal_errors.h"
#include "platform/platform.h"
#include "platform/uart.h"
#include "platform/usb/cdc.h"
#include "platform/usb/usb.h"
#include "stm32_hal.h"
#include "usb/core.h"

#define MODULE_NAME		usb_cdc
#include "macros.h"

#define CLASS_IDX		1

static struct {
	bool initialized;
	USBD_HandleTypeDef *p_usbd;
	PCD_HandleTypeDef *p_pcd;
	uint8_t ctrl_buf[USB_FS_MAX_PACKET_SIZE];
	struct usb_rx_queue_item rx_buf;
	uint8_t ctrl_op_code;
	uint8_t ctrl_len;
	uint8_t alt_interface;
	SemaphoreHandle_t rx_done_semaphore;
	StaticSemaphore_t rx_done_semaphore_buffer;
	SemaphoreHandle_t tx_done_semaphore;
	StaticSemaphore_t tx_done_semaphore_buffer;
} SELF;

static uint8_t cdc_init(USBD_HandleTypeDef *p_dev, uint8_t cfgidx);
static uint8_t cdc_deinit(USBD_HandleTypeDef *p_dev, uint8_t cfgidx);
static uint8_t cdc_setup(USBD_HandleTypeDef *p_dev, USBD_SetupReqTypedef *p_req);
static uint8_t cdc_data_in(USBD_HandleTypeDef *p_dev, uint8_t epnum);
static uint8_t cdc_data_out(USBD_HandleTypeDef *p_dev, uint8_t epnum);
static uint8_t cdc_ep0_rx_ready(USBD_HandleTypeDef *p_dev);

static USBD_ClassTypeDef cdc_class_def = {
	cdc_init,
	cdc_deinit,
	cdc_setup,
	NULL,
	cdc_ep0_rx_ready,
	cdc_data_in,
	cdc_data_out,
	NULL,
	NULL,
	NULL,
};

static void cdc_ctrl(uint8_t cmd, uint8_t *p_buf, uint16_t len)
{
	err_t r;

	switch (cmd) {
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
		if (len != sizeof(struct uart_line_coding))
			break;
		r = uart_config_set((struct uart_line_coding *)p_buf);
		(void) r;
		/* TODO: Handle r somehow */
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
}

static uint8_t cdc_init(USBD_HandleTypeDef *p_dev, uint8_t cfgidx)
{
	HAL_PCD_EP_Open(SELF.p_pcd, CDC_IN_EP, USB_FS_MAX_PACKET_SIZE, USBD_EP_TYPE_BULK);
	HAL_PCD_EP_Open(SELF.p_pcd, CDC_OUT_EP, USB_FS_MAX_PACKET_SIZE, USBD_EP_TYPE_BULK);
	HAL_PCD_EP_Open(SELF.p_pcd, CDC_CMD_EP, USB_FS_MAX_PACKET_SIZE, USBD_EP_TYPE_INTR);

	HAL_PCD_EP_Receive(SELF.p_pcd, CDC_OUT_EP, SELF.rx_buf.data, USB_FS_MAX_PACKET_SIZE);

	return HAL_OK;
}

static uint8_t cdc_deinit(USBD_HandleTypeDef *p_dev, uint8_t cfgidx)
{
	HAL_PCD_EP_Close(SELF.p_pcd, CDC_IN_EP);
	HAL_PCD_EP_Close(SELF.p_pcd, CDC_OUT_EP);
	HAL_PCD_EP_Close(SELF.p_pcd, CDC_CMD_EP);

	return HAL_OK;
}

static uint8_t cdc_setup(USBD_HandleTypeDef *p_dev, USBD_SetupReqTypedef *p_req)
{
	switch (p_req->bmRequest.type) {
	case USB_REQ_TYPE_CLASS :
		if ((p_req->bmRequest.recipient == USB_REQ_RECIPIENT_INTERFACE) &&
				(p_req->wIndex == USB_CDC_CTRL_INTERFACE_NO)) {
			if (p_req->wLength) {
				if (p_req->bmRequest.dir) {
					cdc_ctrl(p_req->bRequest, (uint8_t *)SELF.ctrl_buf, p_req->wLength);
					USBD_CtlSendData(p_dev, (uint8_t *)SELF.ctrl_buf, p_req->wLength);
				} else {
					SELF.ctrl_op_code = p_req->bRequest;
					SELF.ctrl_len = p_req->wLength;

					USBD_CtlPrepareRx(p_dev, (uint8_t *)SELF.ctrl_buf, p_req->wLength);
				}
			} else {
				cdc_ctrl(p_req->bRequest, (uint8_t *)p_req, 0);
			}
		}
		break;

	case USB_REQ_TYPE_STANDARD:
		switch (p_req->bRequest) {
		case USB_REQ_GET_INTERFACE:
			USBD_CtlSendData(p_dev, &SELF.alt_interface, 1);
			break;

		case USB_REQ_SET_INTERFACE:
			SELF.alt_interface = p_req->wValue;
			break;
		}

	default:
		break;
	}

	return HAL_OK;
}

static uint8_t cdc_data_in(USBD_HandleTypeDef *p_dev, uint8_t epnum)
{
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if (epnum != CDC_IN_EP)
		return HAL_OK;

	xSemaphoreGiveFromISR(SELF.tx_done_semaphore, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

	return HAL_OK;
}

static uint8_t cdc_data_out(USBD_HandleTypeDef *p_dev, uint8_t epnum)
{
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	HAL_StatusTypeDef status;

	if (epnum != CDC_OUT_EP)
		return HAL_OK;

	SELF.rx_buf.len = HAL_PCD_EP_GetRxCount(SELF.p_pcd, epnum);
	if (xSemaphoreGiveFromISR(SELF.rx_done_semaphore, &xHigherPriorityTaskWoken) != pdTRUE)
		platform_force_hardfault();
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

	return status;
}

static uint8_t cdc_ep0_rx_ready(USBD_HandleTypeDef *p_dev)
{
	if (SELF.ctrl_op_code == 0xFF)
		return HAL_OK;

	cdc_ctrl(SELF.ctrl_op_code, (uint8_t *)SELF.ctrl_buf, SELF.ctrl_len);

	SELF.ctrl_op_code = 0xFF;

	return HAL_OK;
}

err_t usb_cdc_rx(struct usb_rx_queue_item *p_rx_queue_item, uint32_t timeout_ticks)
{
	if (!SELF.initialized)
		return EUSB_CDC_NO_INIT;

	if (!p_rx_queue_item)
		return EUSB_CDC_INVALID_ARG;

	if (xSemaphoreTake(SELF.rx_done_semaphore, portMAX_DELAY) != pdTRUE)
		return EUSB_CDC_RX_TIMEOUT;

	*p_rx_queue_item = SELF.rx_buf;

	/* The reason to this backwards logic is that FreeRTOS seem to require a queue to be
	 * received when posting to it from an ISR, otherwise we get errQUEUE_FULL.
	 */
	HAL_PCD_EP_Receive(SELF.p_pcd, CDC_OUT_EP, SELF.rx_buf.data, USB_FS_MAX_PACKET_SIZE);

	return ERR_OK;
}

err_t usb_cdc_tx(uint8_t *p_buf, uint16_t len)
{
	HAL_StatusTypeDef status;

	if (!SELF.initialized)
		return EUSB_CDC_NO_INIT;

	if (SELF.p_usbd->dev_state != USBD_STATE_CONFIGURED)
		return EUSB_CDC_NOT_READY;

	xSemaphoreTake(SELF.tx_done_semaphore, portMAX_DELAY);

	status = HAL_PCD_EP_Transmit(SELF.p_pcd, CDC_IN_EP, p_buf, len);
	HAL_ERR_CHECK(status, EUSB_CDC_TRANSMIT);

	return ERR_OK;
}

err_t usb_cdc_init(const struct cdc_init_data *p_data)
{
	HAL_StatusTypeDef status;

	if (SELF.initialized)
		return ERR_OK;

	SELF.p_usbd = p_data->p_usbd;
	SELF.p_pcd = p_data->p_pcd;

	HAL_PCDEx_PMAConfig(SELF.p_pcd, CDC_CMD_EP, PCD_SNG_BUF, USB_PMA_BASE + 2 * USB_FS_MAX_PACKET_SIZE);

	HAL_PCDEx_PMAConfig(SELF.p_pcd, CDC_IN_EP,  PCD_SNG_BUF, USB_PMA_BASE + 4 * USB_FS_MAX_PACKET_SIZE);
	HAL_PCDEx_PMAConfig(SELF.p_pcd, CDC_OUT_EP, PCD_SNG_BUF, USB_PMA_BASE + 5 * USB_FS_MAX_PACKET_SIZE);

	status = USBD_RegisterClass(SELF.p_usbd, CLASS_IDX, &cdc_class_def);
	HAL_ERR_CHECK(status, EUSB_CDC_REG_CLASS);

	SELF.rx_done_semaphore = xSemaphoreCreateBinaryStatic(&SELF.rx_done_semaphore_buffer);

	SELF.tx_done_semaphore = xSemaphoreCreateBinaryStatic(&SELF.tx_done_semaphore_buffer);
	xSemaphoreGive(SELF.tx_done_semaphore);

	SELF.initialized = true;

	return ERR_OK;
}
