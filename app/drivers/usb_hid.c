#include "drivers/usb_hid.h"
#include "drivers/usb.h"

#include <stdbool.h>

static struct {
	bool initialized;
	USBD_HandleTypeDef *usbd_handle;
} self;

err_t usb_hid_send(bool left, bool right, bool mid, uint8_t x, uint8_t y)
{
	uint8_t buff[4] = { 0 };

	if (!self.initialized)
		return EUSB_HID_NO_INIT;

	/* Buttons */
	buff[0] |= left  << 0;
	buff[0] |= mid   << 1;
	buff[0] |= right << 2;

	buff[1] = x;
	buff[2] = y;

	/* Wheel */
	buff[3] = 0;

	USBD_HID_SendReport(self.usbd_handle, buff, 4);

	return ERR_OK;
}

err_t usb_hid_init(USBD_HandleTypeDef *p_usbd)
{
	USBD_StatusTypeDef status;

	if (self.initialized)
		return ERR_OK;

	self.usbd_handle = p_usbd;

	USBD_HID.GetUsrStrDescriptor = usb_desc_usr_str;

	status = USBD_RegisterClass(self.usbd_handle, &USBD_HID);
	if (status != USBD_OK)
		return EUSB_HID_REG_CLASS;

	self.initialized = true;

	return ERR_OK;
}
