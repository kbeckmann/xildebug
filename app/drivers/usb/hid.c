#include "drivers/usb/core.h"
#include "drivers/usb/ctlreq.h"
#include "drivers/usb/hid.h"
#include "drivers/usb.h"

#include <stdbool.h>

#define CLASS_IDX					0

static struct {
	bool initialized;
	USBD_HandleTypeDef *p_usbd;
	PCD_HandleTypeDef *p_pcd;
	uint8_t rx_buff[USB_FS_MAX_PACKET_SIZE];
	uint8_t rx_len;
	bool tx_busy;
	bool report_available;
	uint32_t protocol;
	uint32_t idle_state;
	uint32_t alt_interface;
} self;

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
	HAL_PCD_EP_Open(self.p_pcd, HID_IN_EP, USB_FS_MAX_PACKET_SIZE, USBD_EP_TYPE_INTR);
	HAL_PCD_EP_Open(self.p_pcd, HID_OUT_EP, USB_FS_MAX_PACKET_SIZE, USBD_EP_TYPE_INTR);

	self.tx_busy = false;

	HAL_PCD_EP_Receive(self.p_pcd, HID_OUT_EP, self.rx_buff, USB_FS_MAX_PACKET_SIZE);

	return HAL_OK;
}

static uint8_t hid_deinit(USBD_HandleTypeDef *p_dev, uint8_t cfgidx)
{
	HAL_PCD_EP_Close(self.p_pcd, HID_IN_EP);
	HAL_PCD_EP_Close(self.p_pcd, HID_OUT_EP);

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
				self.protocol = (uint8_t)(p_req->wValue);
				break;

			case HID_REQ_GET_PROTOCOL:
				USBD_CtlSendData(p_dev, (uint8_t *)&self.protocol, 1);
				break;

			case HID_REQ_SET_IDLE:
				self.idle_state = (uint8_t)(p_req->wValue >> 8);
				break;

			case HID_REQ_GET_IDLE:
				USBD_CtlSendData(p_dev, (uint8_t *)&self.idle_state, 1);
				break;

			case HID_REQ_SET_REPORT:
				self.report_available = true;
				USBD_CtlPrepareRx(p_dev, self.rx_buff, p_req->wLength);
				break;

			default:
				USBD_CtlError(self.p_pcd);
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
			USBD_CtlSendData(p_dev, (uint8_t *)&self.alt_interface, 1);
			break;

		case USB_REQ_SET_INTERFACE:
			self.alt_interface = (uint8_t)(p_req->wValue);
			break;
		}
	}

	return HAL_OK;
}

static uint8_t hid_ep0_rx_ready(USBD_HandleTypeDef *p_dev)
{
	if (!self.report_available)
		return HAL_OK;

	self.report_available = false;

	return HAL_OK;
}

static uint8_t hid_data_in(USBD_HandleTypeDef *p_dev, uint8_t epnum)
{
	if (epnum != HID_IN_EP)
		return HAL_OK;

	self.tx_busy = false;

	return HAL_OK;
}

static uint8_t hid_data_out(USBD_HandleTypeDef *p_dev, uint8_t epnum)
{
	HAL_StatusTypeDef status;

	if (epnum != HID_OUT_EP)
		return HAL_OK;

	self.rx_len = HAL_PCD_EP_GetRxCount(self.p_pcd, epnum);

	status = HAL_PCD_EP_Receive(self.p_pcd, HID_OUT_EP, self.rx_buff, USB_FS_MAX_PACKET_SIZE);

	return status;
}

err_t usb_hid_send(uint8_t *p_data, uint16_t len)
{
	HAL_StatusTypeDef status;

	if (!self.initialized)
		return EUSB_HID_NO_INIT;

	if (self.p_usbd->dev_state != USBD_STATE_CONFIGURED)
		return EUSB_HID_NOT_READY;

	if (self.tx_busy)
		return EUSB_HID_BUSY;

	self.tx_busy = true;

	status = HAL_PCD_EP_Transmit(self.p_pcd, HID_IN_EP, p_data, len);
	if (status != HAL_OK)
		return EUSB_HID_TRANSMIT;

	return ERR_OK;
}

err_t usb_hid_init(USBD_HandleTypeDef *p_usbd, PCD_HandleTypeDef *p_pcd)
{
	HAL_StatusTypeDef status;

	if (self.initialized)
		return ERR_OK;

	self.p_usbd = p_usbd;
	self.p_pcd = p_pcd;

	HAL_PCDEx_PMAConfig(p_pcd, HID_OUT_EP, PCD_SNG_BUF, USB_PMA_BASE + 3 * USB_FS_MAX_PACKET_SIZE);
	HAL_PCDEx_PMAConfig(p_pcd, HID_IN_EP,  PCD_SNG_BUF, USB_PMA_BASE + 6 * USB_FS_MAX_PACKET_SIZE);

	status = USBD_RegisterClass(self.p_usbd, CLASS_IDX, &hid_class_def);
	if (status != HAL_OK)
		return EUSB_HID_REG_CLASS;

	self.initialized = true;

	return ERR_OK;
}
