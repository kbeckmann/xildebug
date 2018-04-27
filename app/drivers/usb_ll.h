#pragma once

#include "stm32l4xx_hal.h"
#include "errors.h"

err_t usb_ll_init(PCD_HandleTypeDef *p_pcd);
