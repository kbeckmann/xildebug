#include "config/usbd_desc.h"
#include "drivers/usb.h"
#include "drivers/usb_cdc.h"
#include "drivers/usb_ll.h"

#include <stdbool.h>

#include <usbd_core.h>
#include <usbd_cdc.h>

static struct
{
	bool initialized;
	USBD_HandleTypeDef usbd_handle;
	PCD_HandleTypeDef pcd_handle;
} self;

static uint8_t *usb_desc_device(USBD_SpeedTypeDef speed, uint16_t *p_len);
static uint8_t *usb_desc_langid(USBD_SpeedTypeDef speed, uint16_t *p_len);
static uint8_t *usb_desc_manuf_str(USBD_SpeedTypeDef speed, uint16_t *p_len);
static uint8_t *usb_desc_product_str(USBD_SpeedTypeDef speed, uint16_t *p_len);
static uint8_t *usb_desc_serial_str(USBD_SpeedTypeDef speed, uint16_t *p_len);
static uint8_t *usb_desc_config_str(USBD_SpeedTypeDef speed, uint16_t *p_len);
static uint8_t *usb_desc_if_str(USBD_SpeedTypeDef speed, uint16_t *p_len);

#if (USBD_LPM_ENABLED == 1)
static uint8_t * usb_desc_bos(USBD_SpeedTypeDef speed, uint16_t *p_len);
#endif

USBD_DescriptorsTypeDef desc_funcs =
{
	usb_desc_device,
	usb_desc_langid,
	usb_desc_manuf_str,
	usb_desc_product_str,
	usb_desc_serial_str,
	usb_desc_config_str,
	usb_desc_if_str,
#if (USBD_LPM_ENABLED == 1)
	usb_desc_bos,
#endif
};

__attribute__ ((aligned(4)))
static uint8_t desc_device[USB_LEN_DEV_DESC] =
{
	0x12,							/* bLength */
	USB_DESC_TYPE_DEVICE,			/* bDescriptorType */
	0x01, 0x02,						/* bcdUSB */
	0x02,							/* bDeviceClass */
	0x02,							/* bDeviceSubClass */
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

/* USB_DeviceDescriptor */
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

/** USB lang indentifier descriptor. */
__attribute__ ((aligned(4)))
static uint8_t desc_langid[USB_LEN_LANGID_STR_DESC] =
{
	USB_LEN_LANGID_STR_DESC,
	USB_DESC_TYPE_STRING,
	LOBYTE(USBD_LANGID_STRING),
	HIBYTE(USBD_LANGID_STRING)
};

/* Internal string descriptor. */
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

static uint8_t *usb_desc_product_str(USBD_SpeedTypeDef speed, uint16_t *p_len)
{
	USBD_GetString((uint8_t *)USBD_PRODUCT_STRING_FS, desc_str_buf, p_len);

	return desc_str_buf;
}

static uint8_t *usb_desc_manuf_str(USBD_SpeedTypeDef speed, uint16_t *p_len)
{
	USBD_GetString((uint8_t *)USBD_MANUFACTURER_STRING, desc_str_buf, p_len);

	return desc_str_buf;
}

static uint8_t *usb_desc_serial_str(USBD_SpeedTypeDef speed, uint16_t *p_len)
{
	USBD_GetString((uint8_t *)USBD_SERIALNUMBER_STRING_FS, desc_str_buf, p_len);
	return desc_str_buf;
}

static uint8_t *usb_desc_config_str(USBD_SpeedTypeDef speed, uint16_t *p_len)
{
	USBD_GetString((uint8_t *)USBD_CONFIGURATION_STRING_FS, desc_str_buf, p_len);

	return desc_str_buf;
}

static uint8_t *usb_desc_if_str(USBD_SpeedTypeDef speed, uint16_t *p_len)
{
	USBD_GetString((uint8_t *)USBD_INTERFACE_STRING_FS, desc_str_buf, p_len);

	return desc_str_buf;
}

#if (USBD_LPM_ENABLED == 1)
static uint8_t *usb_desc_bos(USBD_SpeedTypeDef speed, uint16_t *p_len)
{
	*p_len = sizeof(desc_bos);

	return (uint8_t*)desc_bos;
}
#endif

uint8_t * usb_desc_usr_str(USBD_HandleTypeDef *p_dev, uint8_t idx, uint16_t *p_len)
{
	printf("%s: %d\r\n", __func__, idx);
	return NULL;
}

void USB_IRQHandler(void)
{
	HAL_PCD_IRQHandler(&self.pcd_handle);
}

err_t usb_init(void)
{
	USBD_StatusTypeDef status;
	err_t r;

	if (self.initialized)
		return ERR_OK;

	r = usb_ll_init(&self.pcd_handle);
	ERR_CHECK(r);

	status = USBD_Init(&self.usbd_handle, &desc_funcs, USBD_FS_ID);
	if (status != USBD_OK)
		return EUSB_USBD_INIT;

	r = usb_cdc_init(&self.usbd_handle);
	ERR_CHECK(r);

	status = USBD_Start(&self.usbd_handle);
	if (status != USBD_OK)
		return EUSB_USBD_START;

	self.initialized = true;

	return ERR_OK;
}
