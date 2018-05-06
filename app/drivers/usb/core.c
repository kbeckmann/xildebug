/**
  ******************************************************************************
  * @file    usbd_core.c
  * @author  MCD Application Team
  * @version V2.4.2
  * @date    11-December-2015
  * @brief   This file provides all the USBD core functions.
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

#include "drivers/usb/core.h"
#include "stm32l4xx_hal.h"

static struct {
	PCD_HandleTypeDef *p_pcd;
} self;

HAL_StatusTypeDef USBD_Init(USBD_HandleTypeDef *p_dev, PCD_HandleTypeDef *p_pcd, USBD_DescriptorsTypeDef *p_desc)
{
	if (p_dev == NULL)
		return HAL_ERROR;

	p_dev->pDesc = p_desc;
	p_dev->dev_state = USBD_STATE_DEFAULT;

	/* Enable USB power on Pwrctrl CR2 register. */
	HAL_PWREx_EnableVddUSB();

	self.p_pcd = p_pcd;
	self.p_pcd->Instance = USB;
	self.p_pcd->Init.dev_endpoints = 8;
	self.p_pcd->Init.speed = PCD_SPEED_FULL;
	self.p_pcd->Init.ep0_mps = DEP0CTL_MPS_64;
	self.p_pcd->Init.phy_itface = PCD_PHY_EMBEDDED;
	self.p_pcd->Init.Sof_enable = DISABLE;
	self.p_pcd->Init.low_power_enable = DISABLE;
	self.p_pcd->Init.lpm_enable = DISABLE;
	self.p_pcd->Init.battery_charging_enable = DISABLE;

	if (HAL_PCD_Init(p_pcd) != HAL_OK)
		return HAL_ERROR;

	HAL_PCDEx_PMAConfig(p_pcd, 0x00, PCD_SNG_BUF, 0x18);
	HAL_PCDEx_PMAConfig(p_pcd, 0x80, PCD_SNG_BUF, 0x58);

	return HAL_OK;
}

HAL_StatusTypeDef USBD_DeInit(USBD_HandleTypeDef *p_dev)
{
	/* Set Default State */
	p_dev->dev_state = USBD_STATE_DEFAULT;

	USBD_Stop(p_dev);

	return HAL_PCD_DeInit(self.p_pcd);
}

HAL_StatusTypeDef USBD_RegisterClass(USBD_HandleTypeDef *p_dev, uint8_t idx, USBD_ClassTypeDef *p_class)
{
	if (p_class == NULL)
		return HAL_ERROR;

	/* link the class to the USB Device handle */
	p_dev->pClasses[idx] = p_class;

	return HAL_OK;
}

HAL_StatusTypeDef USBD_Start(void)
{
	return HAL_PCD_Start(self.p_pcd);
}

HAL_StatusTypeDef USBD_Stop(USBD_HandleTypeDef *p_dev)
{
	/* Free Class Resources */
	for (int i = 0; i < USBD_MAX_NUM_CLASSES; ++i)
		p_dev->pClasses[i]->DeInit(p_dev, p_dev->dev_config);

	return HAL_PCD_Stop(self.p_pcd);
}

HAL_StatusTypeDef USBD_SetClassConfig(USBD_HandleTypeDef *p_dev, uint8_t idx, uint8_t cfgidx)
{
	if (p_dev->pClasses[idx] == NULL)
		return HAL_ERROR;

	/* Set configuration and Start the Class*/
	return p_dev->pClasses[idx]->Init(p_dev, cfgidx);
}

HAL_StatusTypeDef USBD_ClrClassConfig(USBD_HandleTypeDef *p_dev, uint8_t idx, uint8_t cfgidx)
{
	if (p_dev->pClasses[idx] == NULL)
		return HAL_ERROR;

	/* Clear configuration  and De-initialize the Class process*/
	return p_dev->pClasses[idx]->DeInit(p_dev, cfgidx);
}

HAL_StatusTypeDef USBD_CtlSendData(USBD_HandleTypeDef *p_dev, uint8_t *p_buf, uint16_t len)
{
	/* Set EP0 State */
	p_dev->ep0_state = USBD_EP0_DATA_IN;
	p_dev->ep_in[0].total_length = len;
	p_dev->ep_in[0].rem_length   = len;

	/* Start the transfer */
	return HAL_PCD_EP_Transmit(self.p_pcd, 0x00, p_buf, len);
}

HAL_StatusTypeDef USBD_CtlPrepareRx(USBD_HandleTypeDef *p_dev, uint8_t *p_buf, uint16_t len)
{
	/* Set EP0 State */
	p_dev->ep0_state = USBD_EP0_DATA_OUT;
	p_dev->ep_out[0].total_length = len;
	p_dev->ep_out[0].rem_length   = len;

	/* Start the transfer */
	return HAL_PCD_EP_Receive(self.p_pcd, 0x00, p_buf, len);
}

HAL_StatusTypeDef USBD_CtlSendStatus(USBD_HandleTypeDef *p_dev)
{
	/* Set EP0 State */
	p_dev->ep0_state = USBD_EP0_STATUS_IN;

	/* Start the transfer */
	return HAL_PCD_EP_Transmit(self.p_pcd, 0x00, NULL, 0);
}

uint8_t USBD_LL_IsStallEP(USBD_HandleTypeDef *p_dev, uint8_t ep_addr)
{
	if((ep_addr & 0x80) == 0x80)
		return self.p_pcd->IN_ep[ep_addr & 0x7F].is_stall;

	return self.p_pcd->OUT_ep[ep_addr & 0x7F].is_stall;
}
