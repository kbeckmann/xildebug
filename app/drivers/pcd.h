#pragma once

#include "drivers/usb/def.h"
#include "errors.h"

err_t pcd_init(USBD_HandleTypeDef *p_usbd);
