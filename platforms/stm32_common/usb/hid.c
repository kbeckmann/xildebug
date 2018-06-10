#include <FreeRTOS.h>
#include <semphr.h>
#include <queue.h>
#include <stdbool.h>

#include "hal_errors.h"
#include "hid_internal.h"
#include "platform/usb/hid.h"
#include "platform/usb/usb.h"
#include "stm32_hal.h"
#include "usb/core.h"
#include "usb/ctlreq.h"

#define MODULE_NAME		usb_hid
#include "macros.h"

#define CLASS_IDX		0
#define QUEUE_LENGTH	10
#define QUEUE_ITEM_SIZE	sizeof(struct usb_rx_queue_item)

static struct {
	bool initialized;
	USBD_HandleTypeDef *p_usbd;
	PCD_HandleTypeDef *p_pcd;
	struct usb_rx_queue_item rx_buf;
	bool report_available;
	uint32_t protocol;
	uint32_t idle_state;
	uint32_t alt_interface;
	StaticQueue_t rx_queue;
	QueueHandle_t rx_queue_handle;
	uint8_t rx_queue_storage[QUEUE_LENGTH * QUEUE_ITEM_SIZE];
	SemaphoreHandle_t tx_done_semaphore;
	StaticSemaphore_t tx_done_semaphore_buffer;
} SELF;

static uint8_t hid_init(USBD_HandleTypeDef *p_dev, uint8_t cfgidx);
static uint8_t hid_deinit(USBD_HandleTypeDef *p_dev, uint8_t cfgidx);
static uint8_t hid_setup(USBD_HandleTypeDef *p_dev, USBD_SetupReqTypedef *p_req);
static uint8_t hid_ep0_rx_ready(USBD_HandleTypeDef *p_dev);
static uint8_t hid_data_in(USBD_HandleTypeDef *p_dev, uint8_t epnum);
static uint8_t hid_data_out(USBD_HandleTypeDef *p_dev, uint8_t epnum);

static USBD_ClassTypeDef hid_class_def = {
	hid_init,
	hid_deinit,
	hid_setup,
	NULL,
	hid_ep0_rx_ready,
	hid_data_in,
	hid_data_out,
	NULL,
	NULL,
	NULL,
};

__attribute__ ((aligned (4)))
static uint8_t desc_hid_report[HID_REPORT_DESC_SIZ] = {
	HID_UsagePageVendor(0x00),
	HID_Usage(0x01),
	HID_Collection(HID_Application),
	HID_LogicalMin(0),
	HID_LogicalMaxS(0xFF),
	HID_ReportSize(8),

	HID_ReportCount(USB_FS_MAX_PACKET_SIZE),
	HID_Usage(0x01),
	HID_Input(HID_Data | HID_Variable | HID_Absolute),

	HID_ReportCount(USB_FS_MAX_PACKET_SIZE),
	HID_Usage(0x01),
	HID_Output(HID_Data | HID_Variable | HID_Absolute),

	HID_ReportCount(1),
	HID_Usage(0x01),
	HID_Feature(HID_Data | HID_Variable | HID_Absolute),
	HID_EndCollection,
};

__attribute__ ((aligned (4)))
static uint8_t desc_hid[] = {
	USB_LEN_HID_DESC,					/* bLength */
	USB_DESC_TYPE_HID,					/* bDescriptorType */
	0x00, 0x01,							/* bcdHID */
	0x00,								/* bCountryCode */
	1,									/* bNumDescriptors */
	USB_DESC_TYPE_HID_REPORT,			/* bDescriptorType */
	LOBYTE(sizeof(desc_hid_report)),	/* wItemLength */
	HIBYTE(sizeof(desc_hid_report)),
};

static uint8_t hid_init(USBD_HandleTypeDef *p_dev, uint8_t cfgidx)
{
	HAL_PCD_EP_Open(SELF.p_pcd, HID_IN_EP, USB_FS_MAX_PACKET_SIZE, USBD_EP_TYPE_INTR);
	HAL_PCD_EP_Open(SELF.p_pcd, HID_OUT_EP, USB_FS_MAX_PACKET_SIZE, USBD_EP_TYPE_INTR);

	HAL_PCD_EP_Receive(SELF.p_pcd, HID_OUT_EP, SELF.rx_buf.data, USB_FS_MAX_PACKET_SIZE);

	return HAL_OK;
}

static uint8_t hid_deinit(USBD_HandleTypeDef *p_dev, uint8_t cfgidx)
{
	HAL_PCD_EP_Close(SELF.p_pcd, HID_IN_EP);
	HAL_PCD_EP_Close(SELF.p_pcd, HID_OUT_EP);

	return HAL_OK;
}

static uint8_t hid_setup(USBD_HandleTypeDef *p_dev, USBD_SetupReqTypedef *p_req)
{
	uint8_t *p_buf = NULL;
	uint16_t len = 0;

	switch (p_req->bmRequest.type) {
	case USB_REQ_TYPE_CLASS :
		if ((p_req->bmRequest.recipient == USB_REQ_RECIPIENT_INTERFACE) &&
				(p_req->wIndex == USB_HID_INTERFACE_NO)) {
			switch (p_req->bRequest) {
			case HID_REQ_SET_PROTOCOL:
				SELF.protocol = (uint8_t)(p_req->wValue);
				break;

			case HID_REQ_GET_PROTOCOL:
				USBD_CtlSendData(p_dev, (uint8_t *)&SELF.protocol, 1);
				break;

			case HID_REQ_SET_IDLE:
				SELF.idle_state = (uint8_t)(p_req->wValue >> 8);
				break;

			case HID_REQ_GET_IDLE:
				USBD_CtlSendData(p_dev, (uint8_t *)&SELF.idle_state, 1);
				break;

			case HID_REQ_SET_REPORT:
				SELF.report_available = true;
				USBD_CtlPrepareRx(p_dev, SELF.rx_buf.data, p_req->wLength);
				break;

			default:
				USBD_CtlError(SELF.p_pcd);
				return HAL_ERROR;
			}
		}
		break;

	case USB_REQ_TYPE_STANDARD:
		switch (p_req->bRequest) {
		case USB_REQ_GET_DESCRIPTOR:
			if ((p_req->wValue >> 8) == USB_DESC_TYPE_HID_REPORT) {
				len = MIN(sizeof(desc_hid_report) , p_req->wLength);
				p_buf = desc_hid_report;
			} else if ((p_req->wValue >> 8) == USB_DESC_TYPE_HID) {
				p_buf = desc_hid;
				len = MIN(USB_LEN_HID_DESC , p_req->wLength);
			}

			USBD_CtlSendData(p_dev, p_buf, len);
			break;

		case USB_REQ_GET_INTERFACE:
			USBD_CtlSendData(p_dev, (uint8_t *)&SELF.alt_interface, 1);
			break;

		case USB_REQ_SET_INTERFACE:
			SELF.alt_interface = (uint8_t)(p_req->wValue);
			break;
		}
	}

	return HAL_OK;
}

static uint8_t hid_ep0_rx_ready(USBD_HandleTypeDef *p_dev)
{
	if (!SELF.report_available)
		return HAL_OK;

	SELF.report_available = false;

	return HAL_OK;
}

static uint8_t hid_data_in(USBD_HandleTypeDef *p_dev, uint8_t epnum)
{
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if (epnum != HID_IN_EP)
		return HAL_OK;

	xSemaphoreGiveFromISR(SELF.tx_done_semaphore, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

	return HAL_OK;
}

static uint8_t hid_data_out(USBD_HandleTypeDef *p_dev, uint8_t epnum)
{
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	HAL_StatusTypeDef status;

	if (epnum != HID_OUT_EP)
		return HAL_OK;

	SELF.rx_buf.len = HAL_PCD_EP_GetRxCount(SELF.p_pcd, epnum);
	status = HAL_PCD_EP_Receive(SELF.p_pcd, HID_OUT_EP, SELF.rx_buf.data, USB_FS_MAX_PACKET_SIZE);

	xQueueSendFromISR(SELF.rx_queue_handle, &SELF.rx_buf, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

	return status;
}

err_t usb_hid_recv(struct usb_rx_queue_item *p_rx_queue_item, uint32_t timeout_ticks)
{
	if (!SELF.initialized)
		return EUSB_HID_NO_INIT;

	if (!p_rx_queue_item)
		return EUSB_HID_INVALID_ARG;

	if (xQueueReceive(SELF.rx_queue_handle, p_rx_queue_item, timeout_ticks) == pdFALSE)
		return EUSB_HID_RECV_TIMEOUT;

	return ERR_OK;
}

err_t usb_hid_send(uint8_t *p_buf, uint16_t len)
{
	HAL_StatusTypeDef status;

	if (!SELF.initialized)
		return EUSB_HID_NO_INIT;

	if (SELF.p_usbd->dev_state != USBD_STATE_CONFIGURED)
		return EUSB_HID_NOT_READY;

	xSemaphoreTake(SELF.tx_done_semaphore, portMAX_DELAY);

	status = HAL_PCD_EP_Transmit(SELF.p_pcd, HID_IN_EP, p_buf, len);
	HAL_ERR_CHECK(status, EUSB_HID_TRANSMIT);

	return ERR_OK;
}

err_t usb_hid_init(const struct hid_init_data *p_data)
{
	HAL_StatusTypeDef status;

	if (SELF.initialized)
		return ERR_OK;

	SELF.p_usbd = p_data->p_usbd;
	SELF.p_pcd = p_data->p_pcd;

	HAL_PCDEx_PMAConfig(SELF.p_pcd, HID_OUT_EP, PCD_SNG_BUF, USB_PMA_BASE + 3 * USB_FS_MAX_PACKET_SIZE);
	HAL_PCDEx_PMAConfig(SELF.p_pcd, HID_IN_EP,  PCD_SNG_BUF, USB_PMA_BASE + 6 * USB_FS_MAX_PACKET_SIZE);

	status = USBD_RegisterClass(SELF.p_usbd, CLASS_IDX, &hid_class_def);
	HAL_ERR_CHECK(status, EUSB_HID_REG_CLASS);

	SELF.rx_queue_handle = xQueueCreateStatic(QUEUE_LENGTH,
		QUEUE_ITEM_SIZE,
		SELF.rx_queue_storage,
		&SELF.rx_queue);

	SELF.tx_done_semaphore = xSemaphoreCreateBinaryStatic(&SELF.tx_done_semaphore_buffer);
	xSemaphoreGive(SELF.tx_done_semaphore);

	SELF.initialized = true;

	return ERR_OK;
}
