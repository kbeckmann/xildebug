#pragma once

#include "errors.h"

#include <usbd_def.h>

#define EUSB_NO_INIT			(EUSB_BASE + 0)
#define EUSB_USBD_INIT			(EUSB_BASE + 1)
#define EUSB_USBD_START			(EUSB_BASE + 2)

uint8_t *usb_desc_usr_str(USBD_HandleTypeDef *p_dev, uint8_t idx, uint16_t *p_len);
err_t usb_init(void);
