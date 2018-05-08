#pragma once

#include "drivers/usb.h"
#include "errors.h"

#define EUSB_HID_NO_INIT		(EUSB_HID_BASE + 0)
#define EUSB_HID_REG_CLASS		(EUSB_HID_BASE + 1)
#define EUSB_HID_TRANSMIT		(EUSB_CDC_BASE + 2)
#define EUSB_HID_BUSY			(EUSB_HID_BASE + 3)
#define EUSB_HID_NOT_READY		(EUSB_HID_BASE + 4)
#define EUSB_HID_INVALID_ARG	(EUSB_CDC_BASE + 5)
#define EUSB_HID_RECV_TIMEOUT	(EUSB_CDC_BASE + 6)

#define HID_IN_EP					0x83
#define HID_OUT_EP					0x03

#define USB_HID_CONFIG_DESC_SIZ		99
#define HID_REPORT_DESC_SIZ			33

err_t usb_hid_recv(struct usb_rx_queue_item *p_rx_queue_item, uint32_t timeout_ticks);
err_t usb_hid_send(uint8_t *p_buf, uint16_t len);
err_t usb_hid_init(USBD_HandleTypeDef *p_usbd, PCD_HandleTypeDef *p_pcd);
