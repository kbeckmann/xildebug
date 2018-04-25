#pragma once

#include "errors.h"

#include <usbd_cdc.h>

#define EUSB_CDC_NO_INIT		(EUSB_CDC_BASE + 0)
#define EUSB_CDC_REG_CLASS		(EUSB_CDC_BASE + 1)
#define EUSB_CDC_REG_IF			(EUSB_CDC_BASE + 2)
#define EUSB_CDC_TRANSMIT		(EUSB_CDC_BASE + 3)
#define EUSB_CDC_BUSY			(EUSB_CDC_BASE + 4)

err_t usb_cdc_tx(uint8_t *p_buf, uint16_t len);
err_t usb_cdc_init(USBD_HandleTypeDef *p_usbd);
