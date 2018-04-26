#pragma once

#include "errors.h"

#include <usbd_hid.h>
#include <stdbool.h>

#define EUSB_HID_NO_INIT		(EUSB_HID_BASE + 0)
#define EUSB_HID_REG_CLASS		(EUSB_HID_BASE + 1)

err_t usb_hid_send(bool left, bool right, bool mid, uint8_t x, uint8_t y);
err_t usb_hid_init(USBD_HandleTypeDef *p_usbd);
