#pragma once

#include "errors.h"

#define EUSB_NO_INIT			(EUSB_BASE + 0)
#define EUSB_USBD_INIT			(EUSB_BASE + 1)
#define EUSB_USBD_START			(EUSB_BASE + 2)

err_t usb_init(void);
