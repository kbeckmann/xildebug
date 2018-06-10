#include <stdbool.h>

#include "hal_errors.h"
#include "pcd.h"
#include "platform/usb/cdc.h"
#include "platform/usb/hid.h"
#include "platform/usb/usb.h"
#include "usb/cdc_internal.h"
#include "usb/core.h"
#include "usb/ctlreq.h"
#include "usb/hid_internal.h"

#define MODULE_NAME				stm32_usb
#include "macros.h"

static struct {
	bool initialized;
	USBD_HandleTypeDef usbd_handle;
	PCD_HandleTypeDef pcd_handle;
	char usb_serialnumber_string[17];
} SELF;

static uint8_t *usb_desc_device(USBD_SpeedTypeDef speed, uint16_t *p_len);
static uint8_t *usb_desc_langid(USBD_SpeedTypeDef speed, uint16_t *p_len);
static uint8_t *usb_desc_cfg(USBD_SpeedTypeDef speed, uint16_t *p_len);
static uint8_t *usb_desc_device_qualifier(uint16_t *p_len);
static uint8_t *usb_desc_usr_str(USBD_HandleTypeDef *p_dev, uint8_t idx, uint16_t *p_len);

#define USBD_LANGID_STRING				1033
#define USBD_VID						0x1209
#define USBD_PID_FS						0x9450
#define USBD_MANUFACTURER_STRING		"xil.se"
#define USBD_PRODUCT_STRING_FS			"XilDebug CMSIS-DAP"

#define USBD_USER_STRING_CDC			"CMSIS-DAP CDC"
#define USBD_USER_STRING_CDC_IDX		4

#define USBD_USER_STRING_DCI			"CMSIS-DAP DCI"
#define USBD_USER_STRING_DCI_IDX		5

#define USBD_USER_STRING_HID			"CMSIS-DAP HID"
#define USBD_USER_STRING_HID_IDX		6

#if (USBD_LPM_ENABLED == 1)
static uint8_t * usb_desc_bos(USBD_SpeedTypeDef speed, uint16_t *p_len);
#endif

static USBD_DescriptorsTypeDef desc_funcs =
{
	usb_desc_device,
	usb_desc_langid,
	usb_desc_cfg,
	usb_desc_device_qualifier,
	usb_desc_usr_str,

	#if (USBD_LPM_ENABLED == 1)
	usb_desc_bos,
	#endif
};

__attribute__ ((aligned(4)))
static uint8_t desc_device[USB_LEN_DEV_DESC] =
{
	USB_LEN_DEV_DESC,				/* bLength */
	USB_DESC_TYPE_DEVICE,			/* bDescriptorType */
	0x00, 0x02,						/* bcdUSB */
	0x02,							/* bDeviceClass */
	0x00,							/* bDeviceSubClass */
	0x00,							/* bDeviceProtocol */
	USB_MAX_EP0_SIZE,				/* bMaxPacketSize */
	LOBYTE(USBD_VID),				/* idVendor */
	HIBYTE(USBD_VID),				/* idVendor */
	LOBYTE(USBD_PID_FS),			/* idProduct */
	HIBYTE(USBD_PID_FS),			/* idProduct */
	0x00, 0x02,						/* bcdDevice rel. 2.00 */
	USBD_IDX_MFC_STR,				/* Index of manufacturer string */
	USBD_IDX_PRODUCT_STR,			/* Index of product string */
	USBD_IDX_SERIAL_STR,			/* Index of serial number string */
	USBD_MAX_NUM_CONFIGURATION		/* bNumConfigurations */
};

#if (USBD_LPM_ENABLED == 1)
__attribute__ ((aligned(4)))
static uint8_t desc_bos[USB_SIZ_BOS_DESC] =
{
	0x5,
	USB_DESC_TYPE_BOS,
	0xC,
	0x0,
	0x1,  /* 1 device capability*/
	/* device capability*/
	0x7,
	USB_DEVICE_CAPABITY_TYPE,
	0x2,
	0x2,  /* LPM capability bit set*/
	0x0,
	0x0,
	0x0
};
#endif

__attribute__ ((aligned(4)))
static uint8_t desc_langid[USB_LEN_LANGID_STR_DESC] =
{
	USB_LEN_LANGID_STR_DESC,
	USB_DESC_TYPE_STRING,
	LOBYTE(USBD_LANGID_STRING),
	HIBYTE(USBD_LANGID_STRING)
};

__attribute__ ((aligned (4)))
static uint8_t desc_configuration[USB_HID_CONFIG_DESC_SIZ] =
{
	USB_LEN_CFG_DESC,					/* bLength */
	USB_DESC_TYPE_CONFIGURATION,		/* bDescriptorType */
	LOBYTE(USB_HID_CONFIG_DESC_SIZ),	/* wTotalLength (LSB) */
	HIBYTE(USB_HID_CONFIG_DESC_SIZ),	/* wTotalLength (MSB) */
	USBD_MAX_NUM_INTERFACES,			/* bNumInterfaces  */
	1,									/* bConfigurationValue */
	0,									/* iConfiguration */
	0x80,								/* bmAttributes  */
	USBD_DESC_MAXPOWER_mA(100),

	/*---------------------------------------------------------------------------*/

	/* CDC Data Interface Descriptor */
	USB_LEN_IF_DESC,					/* bLength */
	USB_DESC_TYPE_INTERFACE,			/* bDescriptorType */
	USB_CDC_CTRL_INTERFACE_NO,			/* bInterfaceNumber */
	0,									/* bAlternateSetting */
	1,									/* bNumEndpoints */
	USB_CLASS_CDC,						/* bInterfaceClass */
	USB_SUBCLASS_CDC_ACM,				/* bInterfaceSubClass */
	0x01,								/* bInterfaceProtocol */
	USBD_USER_STRING_CDC_IDX,			/* iInterface */

	/* Header Functional Descriptor */
	USB_LEN_CDC_HEADER_FND_DESC,		/* bLength */
	USB_DESC_TYPE_CS_INTERFACE,			/* bDescriptorType  */
	USB_SUBTYPE_CDC_HFN,				/* bDescriptorSubtype */
	0x10, 0x01,							/* bcdCDC  */

	/* Call Management Functional Descriptor */
	USB_LEN_CDC_CALLMNG_FND_DESC,		/* bFunctionLength */
	USB_DESC_TYPE_CS_INTERFACE,			/* bDescriptorType */
	USB_SUBTYPE_CDC_CMNGFN,				/* bDescriptorSubtype  */
	0x03,								/* bmCapabilities  */
	USB_CDC_DATA_INTERFACE_NO,			/* bDataInterface */

	/* ACM Functional Descriptor */
	USB_LEN_CDC_ACM_FND_DESC,			/* bFunctionLength */
	USB_DESC_TYPE_CS_INTERFACE,			/* bDescriptorType */
	USB_SUBTYPE_CDC_ACMFN,				/* bDescriptorSubtype */
	0x06,								/* bmCapabilities */

	/* Union Functional Descriptor */
	USB_LEN_CDC_UNION_DESC,				/* bFunctionLength */
	USB_DESC_TYPE_CS_INTERFACE,			/* bDescriptorType */
	USB_SUBTYPE_CDC_UNIONFN,			/* bDescriptorSubtype */
	USB_CDC_CTRL_INTERFACE_NO,			/* bMasterInterface */
	USB_CDC_DATA_INTERFACE_NO,			/* bSlaveInterface0 */

	/* Endpoint Descriptor */
	USB_LEN_EP_DESC,					/* bLength */
	USB_DESC_TYPE_ENDPOINT,				/* bDescriptorType */
	CDC_CMD_EP,							/* bEndpointAddress */
	USBD_EP_TYPE_INTR,					/* bmAttributes */
	LOBYTE(USB_FS_MAX_PACKET_SIZE),		/* wMaxPacketSize (LSB) */
	HIBYTE(USB_FS_MAX_PACKET_SIZE),		/* wMaxPacketSize (MSB) */
	2,									/* bInterval */

	/*---------------------------------------------------------------------------*/

	/* CDC Data Interface Descriptor */
	USB_LEN_IF_DESC,					/* bLength */
	USB_DESC_TYPE_INTERFACE,			/* bDescriptorType */
	USB_CDC_DATA_INTERFACE_NO,			/* bInterfaceNumber */
	0,									/* bAlternateSetting */
	2,									/* bNumEndpoints */
	USB_CLASS_CDC_DATA,					/* bInterfaceClass */
	0,									/* bInterfaceSubClass */
	0,									/* bInterfaceProtocol */
	USBD_USER_STRING_DCI_IDX,			/* iInterface */

	/* Endpoint OUT Descriptor */
	USB_LEN_EP_DESC,					/* bLength */
	USB_DESC_TYPE_ENDPOINT,				/* bDescriptorType */
	CDC_OUT_EP,							/* bEndpointAddress */
	USBD_EP_TYPE_BULK,					/* bmAttributes */
	LOBYTE(USB_FS_MAX_PACKET_SIZE),		/* wMaxPacketSize (LSB) */
	HIBYTE(USB_FS_MAX_PACKET_SIZE),		/* wMaxPacketSize (MSB) */
	0,									/* bInterval */

	/* Endpoint IN Descriptor */
	USB_LEN_EP_DESC,					/* bLength */
	USB_DESC_TYPE_ENDPOINT,				/* bDescriptorType */
	CDC_IN_EP,							/* bEndpointAddress */
	USBD_EP_TYPE_BULK,					/* bmAttributes */
	LOBYTE(USB_FS_MAX_PACKET_SIZE),		/* wMaxPacketSize (LSB) */
	HIBYTE(USB_FS_MAX_PACKET_SIZE),		/* wMaxPacketSize (MSB) */
	0,									/* bInterval */

	/*---------------------------------------------------------------------------*/

	/* HID Interface Descriptor */
	USB_LEN_IF_DESC,					/* bLength */
	USB_DESC_TYPE_INTERFACE,			/* bDescriptorType */
	USB_HID_INTERFACE_NO,				/* bInterfaceNumber */
	0,									/* bAlternateSetting */
	2,									/* bNumEndpoints */
	USB_CLASS_HID,						/* bInterfaceClass */
	0,									/* bInterfaceSubClass */
	0,									/* nInterfaceProtocol */
	USBD_USER_STRING_HID_IDX,			/* iInterface */

	/* HID Descriptor */
	USB_LEN_HID_DESC,					/* bLength */
	USB_DESC_TYPE_HID,					/* bDescriptorType */
	0x00, 0x01,							/* bcdHID */
	0x00,								/* bCountryCode */
	1,									/* bNumDescriptors */
	USB_DESC_TYPE_HID_REPORT,			/* bDescriptorType */
	LOBYTE(HID_REPORT_DESC_SIZ),		/* wItemLength (LSB) */
	HIBYTE(HID_REPORT_DESC_SIZ),		/* wItemLength (MSB) */

	/* Endpoint IN Descriptor */
	USB_LEN_EP_DESC,					/* bLength */
	USB_DESC_TYPE_ENDPOINT,				/* bDescriptorType */
	HID_IN_EP,							/* bEndpointAddress */
	USBD_EP_TYPE_INTR,					/* bmAttributes */
	LOBYTE(USB_FS_MAX_PACKET_SIZE),		/* wMaxPacketSize (LSB) */
	HIBYTE(USB_FS_MAX_PACKET_SIZE),		/* wMaxPacketSize (MSB) */
	1,									/* bInterval */

	/* Endpoint OUT Descriptor */
	USB_LEN_EP_DESC,					/* bLength */
	USB_DESC_TYPE_ENDPOINT,				/* bDescriptorType */
	HID_OUT_EP,							/* bEndpointAddress */
	USBD_EP_TYPE_INTR,					/* bmAttributes */
	LOBYTE(USB_FS_MAX_PACKET_SIZE),		/* wMaxPacketSize (LSB) */
	HIBYTE(USB_FS_MAX_PACKET_SIZE),		/* wMaxPacketSize (MSB) */
	1,									/* bInterval */
};

__attribute__ ((aligned (4)))
static uint8_t desc_device_qualifier[] =
{
	USB_LEN_DEV_QUALIFIER_DESC,
	USB_DESC_TYPE_DEVICE_QUALIFIER,
	0x00,
	0x02,
	0x00,
	0x00,
	0x00,
	0x40,
	0x01,
	0x00,
};

__attribute__ ((aligned(4)))
static uint8_t desc_str_buf[USBD_MAX_STR_DESC_SIZ];

static uint8_t *usb_desc_device(USBD_SpeedTypeDef speed, uint16_t *p_len)
{
	*p_len = sizeof(desc_device);

	return desc_device;
}

static uint8_t *usb_desc_langid(USBD_SpeedTypeDef speed, uint16_t *p_len)
{
	*p_len = sizeof(desc_langid);

	return desc_langid;
}

static uint8_t *usb_desc_cfg(USBD_SpeedTypeDef speed, uint16_t *p_len)
{
	*p_len = sizeof(desc_configuration);

	return desc_configuration;
}

static uint8_t *usb_desc_device_qualifier(uint16_t *p_len)
{
	*p_len = sizeof(desc_device_qualifier);

	return desc_device_qualifier;
}

#if (USBD_LPM_ENABLED == 1)
static uint8_t *usb_desc_bos(USBD_SpeedTypeDef speed, uint16_t *p_len)
{
	*p_len = sizeof(desc_bos);

	return (uint8_t*)desc_bos;
}
#endif

static uint8_t *usb_desc_usr_str(USBD_HandleTypeDef *p_dev, uint8_t idx, uint16_t *p_len)
{
	switch (idx) {
	case USBD_IDX_MFC_STR:
		USBD_GetString((uint8_t *)USBD_MANUFACTURER_STRING, desc_str_buf, p_len);
		break;

	case USBD_IDX_PRODUCT_STR:
		USBD_GetString((uint8_t *)USBD_PRODUCT_STRING_FS, desc_str_buf, p_len);
		break;

	case USBD_IDX_SERIAL_STR:
		USBD_GetString((uint8_t *)SELF.usb_serialnumber_string, desc_str_buf, p_len);
		break;

	case USBD_USER_STRING_CDC_IDX:
		USBD_GetString((uint8_t *)USBD_USER_STRING_CDC, desc_str_buf, p_len);
		break;

	case USBD_USER_STRING_DCI_IDX:
		USBD_GetString((uint8_t *)USBD_USER_STRING_DCI, desc_str_buf, p_len);
		break;

	case USBD_USER_STRING_HID_IDX:
		USBD_GetString((uint8_t *)USBD_USER_STRING_HID, desc_str_buf, p_len);
		break;

	default:
		break;
	}

	return desc_str_buf;
}

void USB_IRQHandler(void)
{
	HAL_PCD_IRQHandler(&SELF.pcd_handle);
}

err_t usb_init(void)
{
	HAL_StatusTypeDef status;
	err_t r;

	if (SELF.initialized)
		return ERR_OK;

	sprintf(SELF.usb_serialnumber_string, "%08lx%08lx", HAL_GetUIDw0() + HAL_GetUIDw2(), HAL_GetUIDw1());

	r = pcd_init(&SELF.usbd_handle);
	ERR_CHECK(r);

	status = USBD_Init(&SELF.usbd_handle, &SELF.pcd_handle, &desc_funcs);
	HAL_ERR_CHECK(status, EUSB_USBD_INIT);

	r = usb_hid_init(&((struct hid_init_data){
		.p_usbd = &SELF.usbd_handle,
		.p_pcd = &SELF.pcd_handle
	}));
	ERR_CHECK(r);

	r = usb_cdc_init(&((struct cdc_init_data){
		.p_usbd = &SELF.usbd_handle,
		.p_pcd = &SELF.pcd_handle
	}));
	ERR_CHECK(r);

	status = USBD_Start();
	HAL_ERR_CHECK(status, EUSB_USBD_START);

	SELF.initialized = true;

	return ERR_OK;
}
