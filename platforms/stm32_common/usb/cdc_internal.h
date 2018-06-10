#pragma once

#include "stm32_hal.h"
#include "usb/core.h"

struct cdc_init_data {
	USBD_HandleTypeDef *p_usbd;
	PCD_HandleTypeDef *p_pcd;
};
