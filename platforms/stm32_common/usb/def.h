/**
  ******************************************************************************
  * @file    usbd_def.h
  * @author  MCD Application Team
  * @version V2.4.2
  * @date    11-December-2015
  * @brief   General defines for the usb device library
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

#include <stdint.h>

#include "usbd_conf.h"

#define USB_LEN_DEV_QUALIFIER_DESC						0x0A
#define USB_LEN_DEV_DESC								0x12
#define USB_LEN_CFG_DESC								0x09
#define USB_LEN_IF_DESC									0x09
#define USB_LEN_EP_DESC									0x07
#define USB_LEN_OTG_DESC								0x03
#define USB_LEN_LANGID_STR_DESC							0x04
#define USB_LEN_OTHER_SPEED_DESC_SIZ					0x09
#define USB_LEN_HID_DESC								0x09
#define USB_LEN_CDC_HEADER_FND_DESC						0x05
#define USB_LEN_CDC_CALLMNG_FND_DESC					0x05
#define USB_LEN_CDC_ACM_FND_DESC						0x04
#define USB_LEN_CDC_UNION_DESC							0x05

#define USBD_IDX_LANGID_STR								0x00
#define USBD_IDX_MFC_STR								0x01
#define USBD_IDX_PRODUCT_STR							0x02
#define USBD_IDX_SERIAL_STR								0x03

#define USB_REQ_TYPE_STANDARD							0x00
#define USB_REQ_TYPE_CLASS								0x01
#define USB_REQ_TYPE_VENDOR								0x03

#define USB_REQ_RECIPIENT_DEVICE						0x00
#define USB_REQ_RECIPIENT_INTERFACE						0x01
#define USB_REQ_RECIPIENT_ENDPOINT						0x02

#define USB_REQ_GET_STATUS								0x00
#define USB_REQ_CLEAR_FEATURE							0x01
#define USB_REQ_SET_FEATURE								0x03
#define USB_REQ_SET_ADDRESS								0x05
#define USB_REQ_GET_DESCRIPTOR							0x06
#define USB_REQ_SET_DESCRIPTOR							0x07
#define USB_REQ_GET_CONFIGURATION						0x08
#define USB_REQ_SET_CONFIGURATION						0x09
#define USB_REQ_GET_INTERFACE							0x0A
#define USB_REQ_SET_INTERFACE							0x0B
#define USB_REQ_SYNCH_FRAME								0x0C

#define USB_CLASS_CDC									2
#define USB_CLASS_HID									3
#define USB_CLASS_CDC_DATA								10

#define USB_SUBCLASS_CDC_ACM							2

#define USB_SUBTYPE_CDC_HFN								0
#define USB_SUBTYPE_CDC_CMNGFN							1
#define USB_SUBTYPE_CDC_ACMFN							2
#define USB_SUBTYPE_CDC_UNIONFN							6

#define USB_DESC_TYPE_DEVICE							1
#define USB_DESC_TYPE_CONFIGURATION						2
#define USB_DESC_TYPE_STRING							3
#define USB_DESC_TYPE_INTERFACE							4
#define USB_DESC_TYPE_ENDPOINT							5
#define USB_DESC_TYPE_DEVICE_QUALIFIER					6
#define USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION			7
#define USB_DESC_TYPE_BOS								0x0F
#define USB_DESC_TYPE_HID								0x21
#define USB_DESC_TYPE_HID_REPORT						0x22
#define USB_DESC_TYPE_CS_INTERFACE						0x24
#define USB_DESC_TYPE_CS_ENDPOINT						0x25

#define USB_CONFIG_REMOTE_WAKEUP						2
#define USB_CONFIG_SELF_POWERED							1

#define USBD_DESC_MAXPOWER_mA(x)						(((x) + 1) / 2)

#define USB_FEATURE_EP_HALT								0
#define USB_FEATURE_REMOTE_WAKEUP						1
#define USB_FEATURE_TEST_MODE							2

#define USB_DEVICE_CAPABITY_TYPE						0x10

#define USB_HS_MAX_PACKET_SIZE							512
#define USB_FS_MAX_PACKET_SIZE							64
#define USB_MAX_EP0_SIZE								64

#define CDC_SEND_ENCAPSULATED_COMMAND					0x00
#define CDC_GET_ENCAPSULATED_RESPONSE					0x01
#define CDC_SET_COMM_FEATURE							0x02
#define CDC_GET_COMM_FEATURE							0x03
#define CDC_CLEAR_COMM_FEATURE							0x04
#define CDC_SET_LINE_CODING								0x20
#define CDC_GET_LINE_CODING								0x21
#define CDC_SET_CONTROL_LINE_STATE						0x22
#define CDC_SEND_BREAK									0x23

#define HID_REQ_SET_PROTOCOL							0x0B
#define HID_REQ_GET_PROTOCOL							0x03

#define HID_REQ_SET_IDLE								0x0A
#define HID_REQ_GET_IDLE								0x02

#define HID_REQ_SET_REPORT								0x09
#define HID_REQ_GET_REPORT								0x01

#define HID_Usage(x)									0x09, x
#define HID_Input(x)									0x81, x
#define HID_Output(x)									0x91, x
#define HID_Feature(x)									0xB1, x
#define HID_Collection(x)								0xA1, x
#define HID_EndCollection								0xC0
#define HID_Application									0x01

#define HID_Data										(0 << 0)
#define HID_Variable									(1 << 1)
#define HID_Absolute									(0 << 2)

#define HID_UsagePageVendor(x)							0x06, x, 0xFF
#define HID_LogicalMin(x)								0x15, x
#define HID_LogicalMaxS(x)								0x26, (x & 0xFF), ((x >> 8) & 0xFF)
#define HID_ReportSize(x)								0x75, x
#define HID_ReportCount(x)								0x95, x

/*  Device Status */
#define USBD_STATE_DEFAULT								1
#define USBD_STATE_ADDRESSED							2
#define USBD_STATE_CONFIGURED							3
#define USBD_STATE_SUSPENDED							4

/*  EP0 State */
#define USBD_EP0_IDLE									0
#define USBD_EP0_SETUP									1
#define USBD_EP0_DATA_IN								2
#define USBD_EP0_DATA_OUT								3
#define USBD_EP0_STATUS_IN								4
#define USBD_EP0_STATUS_OUT								5
#define USBD_EP0_STALL									6

#define USBD_EP_TYPE_CTRL								0
#define USBD_EP_TYPE_ISOC								1
#define USBD_EP_TYPE_BULK								2
#define USBD_EP_TYPE_INTR								3


typedef struct  usb_setup_req
{
	struct {
		uint8_t recipient : 5;
		uint8_t type      : 2;
		uint8_t dir       : 1;
	} bmRequest;
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
}  __attribute((packed)) USBD_SetupReqTypedef ;

struct _USBD_HandleTypeDef;

typedef struct _Device_cb
{
	uint8_t (*Init)(struct _USBD_HandleTypeDef *p_dev, uint8_t cfgidx);
	uint8_t (*DeInit)(struct _USBD_HandleTypeDef *p_dev, uint8_t cfgidx);

	/* Control Endpoints*/
	uint8_t (*Setup)(struct _USBD_HandleTypeDef *p_dev, USBD_SetupReqTypedef *p_req);
	uint8_t (*EP0_TxSent)(struct _USBD_HandleTypeDef *p_dev);
	uint8_t (*EP0_RxReady)(struct _USBD_HandleTypeDef *p_dev);

	/* Class Specific Endpoints*/
	uint8_t (*DataIn)(struct _USBD_HandleTypeDef *p_dev, uint8_t epnum);
	uint8_t (*DataOut)(struct _USBD_HandleTypeDef *p_dev, uint8_t epnum);
	uint8_t (*SOF)(struct _USBD_HandleTypeDef *p_dev);
	uint8_t (*IsoINIncomplete)(struct _USBD_HandleTypeDef *p_dev, uint8_t epnum);
	uint8_t (*IsoOUTIncomplete)(struct _USBD_HandleTypeDef *p_dev, uint8_t epnum);
} USBD_ClassTypeDef;

/* Following USB Device Speed */
typedef enum 
{
	USBD_SPEED_HIGH = 0,
	USBD_SPEED_FULL = 1,
	USBD_SPEED_LOW  = 2,
} USBD_SpeedTypeDef;

/* USB Device descriptors structure */
typedef struct
{
	uint8_t *(*GetDeviceDescriptor)(USBD_SpeedTypeDef speed, uint16_t *p_length);
	uint8_t *(*GetLangIDStrDescriptor)(USBD_SpeedTypeDef speed, uint16_t *p_length);

	uint8_t *(*GetConfigDescriptor)(USBD_SpeedTypeDef speed, uint16_t *p_length);
	uint8_t *(*GetDeviceQualifierDescriptor)(uint16_t *p_length);
	uint8_t *(*GetUsrStrDescriptor)(struct _USBD_HandleTypeDef *p_dev, uint8_t index, uint16_t *p_length);

#if (USBD_LPM_ENABLED == 1)
	uint8_t *(*GetBOSDescriptor)(USBD_SpeedTypeDef speed, uint16_t *p_length);
#endif
} USBD_DescriptorsTypeDef;

/* USB Device handle structure */
typedef struct
{ 
	uint32_t status;
	uint32_t total_length;
	uint32_t rem_length;
	uint32_t maxpacket;
} USBD_EndpointTypeDef;

/* USB Device handle structure */
typedef struct _USBD_HandleTypeDef
{
	uint32_t				dev_config;
	uint32_t				dev_default_config;
	uint32_t				dev_config_status;
	USBD_SpeedTypeDef		dev_speed;
	USBD_EndpointTypeDef	ep_in[15];
	USBD_EndpointTypeDef	ep_out[15];
	uint32_t				ep0_state;
	uint32_t				ep0_data_len;
	uint8_t					dev_state;
	uint8_t					dev_old_state;
	uint8_t					dev_address;
	uint8_t					dev_connection_status;
	uint32_t				dev_remote_wakeup;

	USBD_SetupReqTypedef	request;
	USBD_DescriptorsTypeDef	*pDesc;
	USBD_ClassTypeDef		*pClasses[USBD_MAX_NUM_CLASSES];
} USBD_HandleTypeDef;

#define SWAPBYTE(addr) (((uint16_t)(*((uint8_t *)(addr)))) + (((uint16_t)(*(((uint8_t *)(addr)) + 1))) << 8))

#define LOBYTE(x) ((uint8_t)(x & 0x00FF))
#define HIBYTE(x) ((uint8_t)((x & 0xFF00) >> 8))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
