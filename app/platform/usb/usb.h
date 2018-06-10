#pragma once

#include "errors.h"
#include "usb/def.h"

#define EUSB_NO_INIT			(EUSB_BASE + 0)
#define EUSB_USBD_INIT			(EUSB_BASE + 1)
#define EUSB_USBD_START			(EUSB_BASE + 2)

struct usb_rx_queue_item {
	uint8_t len;
	uint8_t data[USB_FS_MAX_PACKET_SIZE];
} __packed;

err_t usb_init(void);
