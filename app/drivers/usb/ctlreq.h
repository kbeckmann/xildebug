/**
  ******************************************************************************
  * @file    usbd_req.h
  * @author  MCD Application Team
  * @version V2.4.2
  * @date    11-December-2015
  * @brief   Header file for the usbd_req.c file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

#pragma once

#include "drivers/usb/def.h"
#include "stm32l4xx_hal.h"

HAL_StatusTypeDef USBD_StdDevReq(USBD_HandleTypeDef *p_dev, PCD_HandleTypeDef *p_pcd, USBD_SetupReqTypedef *p_req);
HAL_StatusTypeDef USBD_StdItfReq(USBD_HandleTypeDef *p_dev, PCD_HandleTypeDef *p_pcd, USBD_SetupReqTypedef *p_req);
HAL_StatusTypeDef USBD_StdEPReq(USBD_HandleTypeDef *p_dev, PCD_HandleTypeDef *p_pcd, USBD_SetupReqTypedef *p_req);

void USBD_CtlError(PCD_HandleTypeDef *p_pcd);
void USBD_GetString(uint8_t *p_desc, uint8_t *p_unicode, uint16_t *p_len);
