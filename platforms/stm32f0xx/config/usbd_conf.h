#pragma once

#include <stdio.h>

#define USBD_MAX_NUM_INTERFACES			3
#define USBD_MAX_NUM_CLASSES			2
#define USBD_MAX_NUM_CONFIGURATION		1
#define USBD_MAX_NUM_ENDPOINTS			8
#define USBD_LPM_ENABLED				0
#define USBD_MAX_STR_DESC_SIZ			64

#define USB_CDC_CTRL_INTERFACE_NO		0
#define USB_CDC_DATA_INTERFACE_NO		1
#define USB_HID_INTERFACE_NO			2

#define USB_PMA_BASE					(8 * USBD_MAX_NUM_ENDPOINTS)
