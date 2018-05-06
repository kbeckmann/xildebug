/**
  ******************************************************************************
  * @file    usbd_req.c
  * @author  MCD Application Team
  * @version V2.4.2
  * @date    11-December-2015
  * @brief   This file provides the standard USB requests following chapter 9.
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
#include "drivers/usb/ctlreq.h"
#include "stm32l4xx_hal.h"

#include <string.h>

static void USBD_GetDescriptor(USBD_HandleTypeDef *p_dev, PCD_HandleTypeDef *p_pcd, USBD_SetupReqTypedef *p_req)
{
	uint8_t *p_buf;
	uint16_t len;

	switch (p_req->wValue >> 8) {
#if (USBD_LPM_ENABLED == 1)
	case USB_DESC_TYPE_BOS:
		pbuf = pdev->pDesc->GetBOSDescriptor(pdev->dev_speed, &len);
		break;
#endif

	case USB_DESC_TYPE_DEVICE:
		p_buf = p_dev->pDesc->GetDeviceDescriptor(p_dev->dev_speed, &len);
		break;

	case USB_DESC_TYPE_CONFIGURATION:
		p_buf = (uint8_t *)p_dev->pDesc->GetConfigDescriptor(p_dev->dev_speed, &len);
		p_buf[1] = USB_DESC_TYPE_CONFIGURATION;
		break;

	case USB_DESC_TYPE_STRING:
		switch ((uint8_t)(p_req->wValue)) {
		case USBD_IDX_LANGID_STR:
			p_buf = p_dev->pDesc->GetLangIDStrDescriptor(p_dev->dev_speed, &len);
			break;

		default:
			p_buf = p_dev->pDesc->GetUsrStrDescriptor(p_dev, (p_req->wValue), &len);
			break;
		}
		break;

	case USB_DESC_TYPE_DEVICE_QUALIFIER:
		if(p_dev->dev_speed == USBD_SPEED_HIGH) {
			p_buf = (uint8_t *)p_dev->pDesc->GetDeviceQualifierDescriptor(&len);
			break;
		} else {
			USBD_CtlError(p_pcd);
			return;
		}

	case USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION:
		if(p_dev->dev_speed == USBD_SPEED_HIGH) {
			p_buf = (uint8_t *)p_dev->pDesc->GetConfigDescriptor(USBD_SPEED_HIGH, &len);
			p_buf[1] = USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION;
			break;
		} else {
			USBD_CtlError(p_pcd);
			return;
		}

	default:
		USBD_CtlError(p_pcd);
		return;
	}

	if((len != 0) && (p_req->wLength != 0)) {
		len = MIN(len, p_req->wLength);

		USBD_CtlSendData(p_dev, p_buf, len);
	}
}

static void USBD_SetAddress(USBD_HandleTypeDef *p_dev, PCD_HandleTypeDef *p_pcd, USBD_SetupReqTypedef *p_req)
{
	uint8_t  dev_addr;

	if ((p_req->wIndex == 0) && (p_req->wLength == 0)) {
		dev_addr = (uint8_t)(p_req->wValue) & 0x7F;

		if (p_dev->dev_state == USBD_STATE_CONFIGURED) {
			USBD_CtlError(p_pcd);
		} else {
			p_dev->dev_address = dev_addr;
			HAL_PCD_SetAddress(p_pcd, dev_addr);
			USBD_CtlSendStatus(p_dev);

			if (dev_addr != 0) {
				p_dev->dev_state = USBD_STATE_ADDRESSED;
			} else {
				p_dev->dev_state = USBD_STATE_DEFAULT;
			}
		}
	} else {
		USBD_CtlError(p_pcd);
	}
}

static void USBD_SetConfig(USBD_HandleTypeDef *p_dev, PCD_HandleTypeDef *p_pcd, USBD_SetupReqTypedef *p_req)
{
	static uint8_t cfgidx;

	cfgidx = (uint8_t)(p_req->wValue);

	if (cfgidx > USBD_MAX_NUM_CONFIGURATION) {
		USBD_CtlError(p_pcd);
	} else {
		switch (p_dev->dev_state) {
		case USBD_STATE_ADDRESSED:
			if (cfgidx) {
				p_dev->dev_config = cfgidx;
				p_dev->dev_state = USBD_STATE_CONFIGURED;

				for (int i = 0; i < USBD_MAX_NUM_CLASSES; ++i) {
					if(USBD_SetClassConfig(p_dev, i, cfgidx) == HAL_ERROR) {
						USBD_CtlError(p_pcd);
						return;
					}
				}
			}

			USBD_CtlSendStatus(p_dev);
			break;

		case USBD_STATE_CONFIGURED:
			if (cfgidx == 0) {
				p_dev->dev_state = USBD_STATE_ADDRESSED;
				p_dev->dev_config = cfgidx;

				for (int i = 0; i < USBD_MAX_NUM_CLASSES; ++i)
					USBD_ClrClassConfig(p_dev, i, cfgidx);

				USBD_CtlSendStatus(p_dev);

			} else if (cfgidx != p_dev->dev_config) {
				/* Clear old configuration */
				for (int i = 0; i < USBD_MAX_NUM_CLASSES; ++i)
					USBD_ClrClassConfig(p_dev, i, p_dev->dev_config);

				/* set new configuration */
				p_dev->dev_config = cfgidx;
				for (int i = 0; i < USBD_MAX_NUM_CLASSES; ++i) {
					if(USBD_SetClassConfig(p_dev, i, cfgidx) == HAL_ERROR) {
						USBD_CtlError(p_pcd);
						return;
					}
				}
			}

			USBD_CtlSendStatus(p_dev);
			break;

		default:
			USBD_CtlError(p_pcd);
			break;
		}
	}
}

static void USBD_GetConfig(USBD_HandleTypeDef *p_dev, PCD_HandleTypeDef *p_pcd, USBD_SetupReqTypedef *p_req)
{
	if (p_req->wLength != 1) {
		USBD_CtlError(p_pcd);
	} else {
		switch (p_dev->dev_state) {
		case USBD_STATE_ADDRESSED:
			p_dev->dev_default_config = 0;
			USBD_CtlSendData(p_dev, (uint8_t *)&p_dev->dev_default_config, 1);
			break;

		case USBD_STATE_CONFIGURED:
			USBD_CtlSendData(p_dev, (uint8_t *)&p_dev->dev_config, 1);
			break;

		default:
			USBD_CtlError(p_pcd);
			break;
		}
	}
}

static void USBD_GetStatus(USBD_HandleTypeDef *p_dev, PCD_HandleTypeDef *p_pcd)
{
	switch (p_dev->dev_state) {
	case USBD_STATE_ADDRESSED:
	case USBD_STATE_CONFIGURED:
#if (USBD_SELF_POWERED == 1)
		pdev->dev_config_status = USB_CONFIG_SELF_POWERED;
#else
		p_dev->dev_config_status = 0;
#endif

		if (p_dev->dev_remote_wakeup)
			p_dev->dev_config_status |= USB_CONFIG_REMOTE_WAKEUP;

		USBD_CtlSendData(p_dev, (uint8_t *)&p_dev->dev_config_status, 2);
		break;

	default :
		USBD_CtlError(p_pcd);
		break;
	}
}

static void USBD_SetFeature(USBD_HandleTypeDef *p_dev, USBD_SetupReqTypedef *p_req)
{
	if (p_req->wValue == USB_FEATURE_REMOTE_WAKEUP) {
		p_dev->dev_remote_wakeup = 1;

		for (int i = 0; i < USBD_MAX_NUM_CLASSES; ++i)
			p_dev->pClasses[i]->Setup(p_dev, p_req);

		USBD_CtlSendStatus(p_dev);
	}

}

static void USBD_ClrFeature(USBD_HandleTypeDef *p_dev, PCD_HandleTypeDef *p_pcd, USBD_SetupReqTypedef *p_req)
{
	switch (p_dev->dev_state) {
	case USBD_STATE_ADDRESSED:
	case USBD_STATE_CONFIGURED:
		if (p_req->wValue == USB_FEATURE_REMOTE_WAKEUP) {
			p_dev->dev_remote_wakeup = 0;

			for (int i = 0; i < USBD_MAX_NUM_CLASSES; ++i)
				p_dev->pClasses[i]->Setup(p_dev, p_req);

			USBD_CtlSendStatus(p_dev);
		}
		break;

	default :
		USBD_CtlError(p_pcd);
		break;
	}
}

HAL_StatusTypeDef USBD_StdDevReq(USBD_HandleTypeDef *p_dev, PCD_HandleTypeDef *p_pcd, USBD_SetupReqTypedef *p_req)
{
	HAL_StatusTypeDef ret = HAL_OK;

	switch (p_req->bRequest) {
	case USB_REQ_GET_DESCRIPTOR:
		USBD_GetDescriptor(p_dev, p_pcd, p_req);
		break;

	case USB_REQ_SET_ADDRESS:
		USBD_SetAddress(p_dev, p_pcd, p_req);
		break;

	case USB_REQ_SET_CONFIGURATION:
		USBD_SetConfig(p_dev, p_pcd, p_req);
		break;

	case USB_REQ_GET_CONFIGURATION:
		USBD_GetConfig(p_dev, p_pcd, p_req);
		break;

	case USB_REQ_GET_STATUS:
		USBD_GetStatus(p_dev, p_pcd);
		break;

	case USB_REQ_SET_FEATURE:
		USBD_SetFeature(p_dev, p_req);
		break;

	case USB_REQ_CLEAR_FEATURE:
		USBD_ClrFeature(p_dev, p_pcd, p_req);
		break;

	default:
		USBD_CtlError(p_pcd);
		break;
	}

	return ret;
}

HAL_StatusTypeDef USBD_StdItfReq(USBD_HandleTypeDef *p_dev, PCD_HandleTypeDef *p_pcd, USBD_SetupReqTypedef *p_req)
{
	HAL_StatusTypeDef ret = HAL_OK;

	switch (p_dev->dev_state) {
	case USBD_STATE_CONFIGURED:
		if (LOBYTE(p_req->wIndex) <= USBD_MAX_NUM_INTERFACES) {
			for (int i = 0; i < USBD_MAX_NUM_CLASSES; ++i)
				p_dev->pClasses[i]->Setup(p_dev, p_req);

			if((p_req->wLength == 0) && (ret == HAL_OK))
				USBD_CtlSendStatus(p_dev);
		} else {
			USBD_CtlError(p_pcd);
		}
		break;

	default:
		USBD_CtlError(p_pcd);
		break;
	}
	return HAL_OK;
}

HAL_StatusTypeDef USBD_StdEPReq(USBD_HandleTypeDef *p_dev, PCD_HandleTypeDef *p_pcd, USBD_SetupReqTypedef *p_req)
{
	const uint8_t ep_addr = LOBYTE(p_req->wIndex);
	HAL_StatusTypeDef ret = HAL_OK;
	USBD_EndpointTypeDef *p_ep;

	/* Check if it is a class request */
	if (p_req->bmRequest.type == USB_REQ_TYPE_CLASS) {
		for (int i = 0; i < USBD_MAX_NUM_CLASSES; ++i)
			p_dev->pClasses[i]->Setup(p_dev, p_req);

		return HAL_OK;
	}

	switch (p_req->bRequest) {
	case USB_REQ_SET_FEATURE :
		switch (p_dev->dev_state) {
		case USBD_STATE_ADDRESSED:
			if ((ep_addr != 0x00) && (ep_addr != 0x80))
				HAL_PCD_EP_SetStall(p_pcd, ep_addr);
			break;

		case USBD_STATE_CONFIGURED:
			if (p_req->wValue == USB_FEATURE_EP_HALT) {
				if ((ep_addr != 0x00) && (ep_addr != 0x80))
					HAL_PCD_EP_SetStall(p_pcd, ep_addr);
			}

			for (int i = 0; i < USBD_MAX_NUM_CLASSES; ++i)
				p_dev->pClasses[i]->Setup(p_dev, p_req);

			USBD_CtlSendStatus(p_dev);
			break;

		default:
			USBD_CtlError(p_pcd);
			break;
		}
		break;

	case USB_REQ_CLEAR_FEATURE :
		switch (p_dev->dev_state) {
		case USBD_STATE_ADDRESSED:
			if ((ep_addr != 0x00) && (ep_addr != 0x80))
				HAL_PCD_EP_SetStall(p_pcd, ep_addr);
			break;

		case USBD_STATE_CONFIGURED:
			if (p_req->wValue == USB_FEATURE_EP_HALT) {
				if ((ep_addr & 0x7F) != 0x00) {
					HAL_PCD_EP_ClrStall(p_pcd, ep_addr);
					for (int i = 0; i < USBD_MAX_NUM_CLASSES; ++i)
						p_dev->pClasses[i]->Setup(p_dev, p_req);
				}

				USBD_CtlSendStatus(p_dev);
			}
			break;

		default:
			USBD_CtlError(p_pcd);
			break;
		}
		break;

	case USB_REQ_GET_STATUS:
		switch (p_dev->dev_state) {
		case USBD_STATE_ADDRESSED:
			if ((ep_addr & 0x7F) != 0x00)
				HAL_PCD_EP_SetStall(p_pcd, ep_addr);
			break;

		case USBD_STATE_CONFIGURED:
			p_ep = ((ep_addr & 0x80) == 0x80) ? &p_dev->ep_in[ep_addr & 0x7F]: &p_dev->ep_out[ep_addr & 0x7F];
			if(USBD_LL_IsStallEP(p_dev, ep_addr)) {
				p_ep->status = 0x0001;
			} else {
				p_ep->status = 0x0000;
			}

			USBD_CtlSendData(p_dev, (uint8_t *)&p_ep->status, 2);
			break;

		default:
			USBD_CtlError(p_pcd);
			break;
		}
		break;

	default:
		break;
	}

	return ret;
}

void USBD_CtlError(PCD_HandleTypeDef *p_pcd)
{
	HAL_PCD_EP_SetStall(p_pcd, 0x80);
	HAL_PCD_EP_SetStall(p_pcd, 0);
}

void USBD_GetString(uint8_t *p_desc, uint8_t *p_unicode, uint16_t *p_len)
{
	uint8_t idx = 0;

	if (p_desc != NULL) {
		*p_len = strlen((char *)p_desc) * 2 + 2;
		p_unicode[idx++] = *p_len;
		p_unicode[idx++] = USB_DESC_TYPE_STRING;

		while(*p_desc != '\0') {
			p_unicode[idx++] = *p_desc++;
			p_unicode[idx++] = 0x00;
		}
	}
}
