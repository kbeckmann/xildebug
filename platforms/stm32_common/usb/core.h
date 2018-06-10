/**
  ******************************************************************************
  * @file    usbd_core.h
  * @author  MCD Application Team
  * @version V2.4.2
  * @date    11-December-2015
  * @brief   Header file for usbd_core.c file
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

#include "stm32_hal.h"
#include "usb/def.h"

HAL_StatusTypeDef USBD_Init(USBD_HandleTypeDef *p_dev, PCD_HandleTypeDef *p_pcd, USBD_DescriptorsTypeDef *p_desc);
HAL_StatusTypeDef USBD_DeInit(USBD_HandleTypeDef *p_dev);
HAL_StatusTypeDef USBD_Start(void);
HAL_StatusTypeDef USBD_Stop(USBD_HandleTypeDef *p_dev);
HAL_StatusTypeDef USBD_RegisterClass(USBD_HandleTypeDef *p_dev, uint8_t idx, USBD_ClassTypeDef *p_class);

HAL_StatusTypeDef USBD_SetClassConfig(USBD_HandleTypeDef *p_dev, uint8_t idx, uint8_t cfgidx);
HAL_StatusTypeDef USBD_ClrClassConfig(USBD_HandleTypeDef *p_dev, uint8_t idx, uint8_t cfgidx);

HAL_StatusTypeDef USBD_CtlSendData(USBD_HandleTypeDef *p_dev, uint8_t *buf, uint16_t len);
HAL_StatusTypeDef USBD_CtlPrepareRx(USBD_HandleTypeDef *p_dev, uint8_t *p_buf, uint16_t len);
HAL_StatusTypeDef USBD_CtlSendStatus(USBD_HandleTypeDef *p_dev);

uint8_t USBD_LL_IsStallEP(USBD_HandleTypeDef *p_dev, uint8_t ep_addr);
