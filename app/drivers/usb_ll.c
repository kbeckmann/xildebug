#include <usbd_core.h>
#include <usbd_cdc.h>

#include "drivers/usb.h"

static struct {
	PCD_HandleTypeDef *pcd_handle;
} self;

USBD_StatusTypeDef hal_to_usb_status(HAL_StatusTypeDef hal_status)
{
	switch (hal_status) {
	case HAL_OK :
		return USBD_OK;

	case HAL_BUSY :
		return USBD_BUSY;

	default:
		return USBD_FAIL;
	}

	return USBD_FAIL;
}

void *USBD_malloc(uint32_t size)
{
	static uint32_t mem[(sizeof(USBD_CDC_HandleTypeDef) / 4) + 1];

	return mem;
}

void USBD_free(void *p)
{

}

void USBD_LL_Delay(uint32_t Delay)
{
	HAL_Delay(Delay);
}

USBD_StatusTypeDef USBD_LL_Init(USBD_HandleTypeDef *p_dev)
{
	/* Enable USB power on Pwrctrl CR2 register. */
	HAL_PWREx_EnableVddUSB();

	/* Link the driver to the stack. */
	p_dev->pData = self.pcd_handle;

	self.pcd_handle->pData = p_dev;
	self.pcd_handle->Instance = USB;
	self.pcd_handle->Init.dev_endpoints = 8;
	self.pcd_handle->Init.speed = PCD_SPEED_FULL;
	self.pcd_handle->Init.ep0_mps = DEP0CTL_MPS_64;
	self.pcd_handle->Init.phy_itface = PCD_PHY_EMBEDDED;
	self.pcd_handle->Init.Sof_enable = DISABLE;
	self.pcd_handle->Init.low_power_enable = DISABLE;
	self.pcd_handle->Init.lpm_enable = DISABLE;
	self.pcd_handle->Init.battery_charging_enable = DISABLE;

	if (HAL_PCD_Init(self.pcd_handle) != HAL_OK)
		return USBD_FAIL;

	HAL_PCDEx_PMAConfig(self.pcd_handle, 0x00 , PCD_SNG_BUF, 0x18);
	HAL_PCDEx_PMAConfig(self.pcd_handle, 0x80 , PCD_SNG_BUF, 0x58);
	HAL_PCDEx_PMAConfig(self.pcd_handle, 0x81 , PCD_SNG_BUF, 0xC0);
	HAL_PCDEx_PMAConfig(self.pcd_handle, 0x01 , PCD_SNG_BUF, 0x110);
	HAL_PCDEx_PMAConfig(self.pcd_handle, 0x82 , PCD_SNG_BUF, 0x100);

	return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_DeInit(USBD_HandleTypeDef *p_dev)
{
	const HAL_StatusTypeDef hal_status = HAL_PCD_DeInit(p_dev->pData);

	return hal_to_usb_status(hal_status);
}

USBD_StatusTypeDef USBD_LL_Start(USBD_HandleTypeDef *p_dev)
{
	const HAL_StatusTypeDef hal_status = HAL_PCD_Start(p_dev->pData);

	return hal_to_usb_status(hal_status);
}

USBD_StatusTypeDef USBD_LL_Stop(USBD_HandleTypeDef *p_dev)
{
	const HAL_StatusTypeDef hal_status = HAL_PCD_Stop(p_dev->pData);

	return hal_to_usb_status(hal_status);
}

USBD_StatusTypeDef USBD_LL_OpenEP(USBD_HandleTypeDef *p_dev, uint8_t ep_addr, uint8_t ep_type, uint16_t ep_mps)
{
	const HAL_StatusTypeDef hal_status = HAL_PCD_EP_Open(p_dev->pData, ep_addr, ep_mps, ep_type);

	return hal_to_usb_status(hal_status);
}

USBD_StatusTypeDef USBD_LL_CloseEP(USBD_HandleTypeDef *p_dev, uint8_t ep_addr)
{
	const HAL_StatusTypeDef hal_status = HAL_PCD_EP_Close(p_dev->pData, ep_addr);

	return hal_to_usb_status(hal_status);
}

USBD_StatusTypeDef USBD_LL_FlushEP(USBD_HandleTypeDef *p_dev, uint8_t ep_addr)
{
	const HAL_StatusTypeDef hal_status = HAL_PCD_EP_Flush(p_dev->pData, ep_addr);

	return hal_to_usb_status(hal_status);
}

USBD_StatusTypeDef USBD_LL_StallEP(USBD_HandleTypeDef *p_dev, uint8_t ep_addr)
{
	const HAL_StatusTypeDef hal_status = HAL_PCD_EP_SetStall(p_dev->pData, ep_addr);

	return hal_to_usb_status(hal_status);
}

USBD_StatusTypeDef USBD_LL_ClearStallEP(USBD_HandleTypeDef *p_dev, uint8_t ep_addr)
{
	const HAL_StatusTypeDef hal_status = HAL_PCD_EP_ClrStall(p_dev->pData, ep_addr);

	return hal_to_usb_status(hal_status);
}

uint8_t USBD_LL_IsStallEP(USBD_HandleTypeDef *p_dev, uint8_t ep_addr)
{
	PCD_HandleTypeDef *p_hpcd = (PCD_HandleTypeDef*)p_dev->pData;

	if((ep_addr & 0x80) == 0x80)
		return p_hpcd->IN_ep[ep_addr & 0x7F].is_stall;

	return p_hpcd->OUT_ep[ep_addr & 0x7F].is_stall;
}

USBD_StatusTypeDef USBD_LL_SetUSBAddress(USBD_HandleTypeDef *p_dev, uint8_t dev_addr)
{
	const HAL_StatusTypeDef hal_status = HAL_PCD_SetAddress(p_dev->pData, dev_addr);

	return hal_to_usb_status(hal_status);
}

USBD_StatusTypeDef USBD_LL_Transmit(USBD_HandleTypeDef *p_dev, uint8_t ep_addr, uint8_t *p_buf, uint16_t size)
{
	const HAL_StatusTypeDef hal_status = HAL_PCD_EP_Transmit(p_dev->pData, ep_addr, p_buf, size);

	return hal_to_usb_status(hal_status);
}

USBD_StatusTypeDef USBD_LL_PrepareReceive(USBD_HandleTypeDef *p_dev, uint8_t ep_addr, uint8_t *p_buf, uint16_t size)
{
	const HAL_StatusTypeDef hal_status = HAL_PCD_EP_Receive(p_dev->pData, ep_addr, p_buf, size);

	return hal_to_usb_status(hal_status);
}

uint32_t USBD_LL_GetRxDataSize(USBD_HandleTypeDef *p_dev, uint8_t ep_addr)
{
	return HAL_PCD_EP_GetRxCount((PCD_HandleTypeDef*) p_dev->pData, ep_addr);
}

USBD_StatusTypeDef USBD_LL_BatteryCharging(USBD_HandleTypeDef *p_dev)
{
	PCD_HandleTypeDef *p_hpcd = (PCD_HandleTypeDef*)p_dev->pData;
	if (p_hpcd->Init.battery_charging_enable == ENABLE)
		return USBD_OK;


	return USBD_FAIL;
}

err_t usb_ll_init(PCD_HandleTypeDef *p_pcd)
{
	self.pcd_handle = p_pcd;

	return ERR_OK;
}
