#include <stdbool.h>

#include "drivers/pcd.h"
#include "drivers/usb/ctlreq.h"
#include "drivers/usb/core.h"
#include "stm32l4xx_hal.h"

static struct {
	USBD_HandleTypeDef *p_usbd;
} self;

extern HAL_StatusTypeDef SystemClock_Config(void);

void HAL_PCD_MspInit(PCD_HandleTypeDef *p_pcd)
{
	if(p_pcd->Instance == USB) {
		__HAL_RCC_USB_CLK_ENABLE();

		/* Peripheral interrupt init */
		HAL_NVIC_SetPriority(USB_IRQn, 5, 0);
		HAL_NVIC_EnableIRQ(USB_IRQn);
	}
}

void HAL_PCD_MspDeInit(PCD_HandleTypeDef* p_pcd)
{
	if(p_pcd->Instance == USB) {
		__HAL_RCC_USB_CLK_DISABLE();

		/* Peripheral interrupt Deinit*/
		HAL_NVIC_DisableIRQ(USB_IRQn);
	}
}

void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef *p_pcd)
{
	uint8_t *p_setup = (uint8_t *)p_pcd->Setup;

	self.p_usbd->request.bmRequest = *(uint8_t *)(p_setup);
	self.p_usbd->request.bRequest  = *(uint8_t *)(p_setup +  1);
	self.p_usbd->request.wValue    =     SWAPBYTE(p_setup +  2);
	self.p_usbd->request.wIndex    =     SWAPBYTE(p_setup +  4);
	self.p_usbd->request.wLength   =     SWAPBYTE(p_setup +  6);

	self.p_usbd->ep0_state = USBD_EP0_SETUP;
	self.p_usbd->ep0_data_len = self.p_usbd->request.wLength;

	switch (self.p_usbd->request.bmRequest & 0x1F) {
	case USB_REQ_RECIPIENT_DEVICE:
		USBD_StdDevReq(self.p_usbd, p_pcd, &self.p_usbd->request);
		break;

	case USB_REQ_RECIPIENT_INTERFACE:
		USBD_StdItfReq(self.p_usbd, p_pcd, &self.p_usbd->request);
		break;

	case USB_REQ_RECIPIENT_ENDPOINT:
		USBD_StdEPReq(self.p_usbd, p_pcd, &self.p_usbd->request);
		break;

	default:
		HAL_PCD_EP_SetStall(p_pcd, self.p_usbd->request.bmRequest & 0x80);
		break;
	}
}

void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *p_pcd, uint8_t epnum)
{
	uint8_t *p_data = p_pcd->OUT_ep[epnum].xfer_buff;
	USBD_EndpointTypeDef *p_ep;

	if (epnum == 0x00) {
		p_ep = &self.p_usbd->ep_out[0];

		if (self.p_usbd->ep0_state == USBD_EP0_DATA_OUT) {
			if (p_ep->rem_length > p_ep->maxpacket) {
				p_ep->rem_length -= p_ep->maxpacket;

				HAL_PCD_EP_Receive(p_pcd, 0x00, p_data, MIN(p_ep->rem_length, p_ep->maxpacket));
			} else {
				for (int i = 0; i < USBD_MAX_NUM_CLASSES; ++i) {
					if ((self.p_usbd->pClasses[i]->EP0_RxReady != NULL) && (self.p_usbd->dev_state == USBD_STATE_CONFIGURED))
						self.p_usbd->pClasses[i]->EP0_RxReady(self.p_usbd);
				}

				USBD_CtlSendStatus(self.p_usbd);
			}
		}
	} else {
		for (int i = 0; i < USBD_MAX_NUM_CLASSES; ++i) {
			if ((self.p_usbd->pClasses[i]->DataOut != NULL) && (self.p_usbd->dev_state == USBD_STATE_CONFIGURED))
				self.p_usbd->pClasses[i]->DataOut(self.p_usbd, epnum);
		}
	}
}

void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *p_pcd, uint8_t epnum)
{
	uint8_t *p_data = p_pcd->IN_ep[epnum].xfer_buff;
	USBD_EndpointTypeDef *p_ep;

	if (epnum == 0x00) {
		p_ep = &self.p_usbd->ep_in[0];

		if ( self.p_usbd->ep0_state == USBD_EP0_DATA_IN) {
			if (p_ep->rem_length > p_ep->maxpacket) {
				p_ep->rem_length -=  p_ep->maxpacket;

				HAL_PCD_EP_Transmit(p_pcd, 0x00, p_data, p_ep->rem_length);

				/* Prepare endpoint for premature end of transfer */
				HAL_PCD_EP_Receive(p_pcd, 0, NULL, 0);
			} else { /* last packet is MPS multiple, so send ZLP packet */
				if ((p_ep->total_length % p_ep->maxpacket == 0) && (p_ep->total_length >= p_ep->maxpacket) && (p_ep->total_length < self.p_usbd->ep0_data_len )) {
					HAL_PCD_EP_Transmit(p_pcd, 0x00, NULL, 0);
					self.p_usbd->ep0_data_len = 0;

					/* Prepare endpoint for premature end of transfer */
					HAL_PCD_EP_Receive(p_pcd, 0, NULL, 0);
				} else {
					for (int i = 0; i < USBD_MAX_NUM_CLASSES; ++i) {
						if ((self.p_usbd->pClasses[i]->EP0_TxSent != NULL) && (self.p_usbd->dev_state == USBD_STATE_CONFIGURED))
							self.p_usbd->pClasses[i]->EP0_TxSent(self.p_usbd);
					}

					self.p_usbd->ep0_state = USBD_EP0_STATUS_OUT;
					HAL_PCD_EP_Receive(p_pcd, 0x00, NULL, 0);
				}
			}
		}
	} else {
		for (int i = 0; i < USBD_MAX_NUM_CLASSES; ++i) {
			if ((self.p_usbd->pClasses[i]->DataIn != NULL) && (self.p_usbd->dev_state == USBD_STATE_CONFIGURED))
				self.p_usbd->pClasses[i]->DataIn(self.p_usbd, epnum | 0x80);
		}
	}
}

void HAL_PCD_SOFCallback(PCD_HandleTypeDef *p_pcd)
{
	if (self.p_usbd->dev_state != USBD_STATE_CONFIGURED)
		return;

	for (int i = 0; i < USBD_MAX_NUM_CLASSES; ++i) {
		if (self.p_usbd->pClasses[i]->SOF != NULL)
			self.p_usbd->pClasses[i]->SOF(self.p_usbd);
	}
}

void HAL_PCD_ResetCallback(PCD_HandleTypeDef *p_pcd)
{
	self.p_usbd->dev_speed = USBD_SPEED_FULL;

	/* Open EP0 OUT */
	HAL_PCD_EP_Open(p_pcd, 0x00, USB_MAX_EP0_SIZE, USBD_EP_TYPE_CTRL);

	self.p_usbd->ep_out[0].maxpacket = USB_MAX_EP0_SIZE;

	/* Open EP0 IN */
	HAL_PCD_EP_Open(p_pcd, 0x80, USB_MAX_EP0_SIZE, USBD_EP_TYPE_CTRL);

	self.p_usbd->ep_in[0].maxpacket = USB_MAX_EP0_SIZE;

	/* Upon Reset call user call back */
	self.p_usbd->dev_state = USBD_STATE_DEFAULT;

	for (int i = 0; i < USBD_MAX_NUM_CLASSES; ++i)
		self.p_usbd->pClasses[i]->DeInit(self.p_usbd, self.p_usbd->dev_config);
}

void HAL_PCD_SuspendCallback(PCD_HandleTypeDef *p_pcd)
{
	/* Inform USB library that core enters in suspend Mode. */
	self.p_usbd->dev_old_state = self.p_usbd->dev_state;
	self.p_usbd->dev_state = USBD_STATE_SUSPENDED;

	/* Enter in STOP mode. */
	if (p_pcd->Init.low_power_enable) {
		/* Set SLEEPDEEP bit and SleepOnExit of Cortex System Control Register. */
		SCB->SCR |= (uint32_t)((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
	}
}

void HAL_PCD_ResumeCallback(PCD_HandleTypeDef *p_pcd)
{
	if (p_pcd->Init.low_power_enable) {
		/* Reset SLEEPDEEP bit of Cortex System Control Register. */
		SCB->SCR &= (uint32_t)~((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
		SystemClock_Config();
	}

	self.p_usbd->dev_state = self.p_usbd->dev_old_state;
}

void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *p_pcd)
{
	/* Free Class Resources */
	self.p_usbd->dev_state = USBD_STATE_DEFAULT;

	for (int i = 0; i < USBD_MAX_NUM_CLASSES; ++i)
		self.p_usbd->pClasses[i]->DeInit(self.p_usbd, self.p_usbd->dev_config);
}

void HAL_PCDEx_LPM_Callback(PCD_HandleTypeDef *p_pcd, PCD_LPM_MsgTypeDef msg)
{
	switch (msg){
	case PCD_LPM_L0_ACTIVE:
		if (p_pcd->Init.low_power_enable){
			SystemClock_Config();

			/* Reset SLEEPDEEP bit of Cortex System Control Register. */
			SCB->SCR &= (uint32_t)~((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
		}

		self.p_usbd->dev_state = self.p_usbd->dev_old_state;
		break;

	case PCD_LPM_L1_ACTIVE:
		self.p_usbd->dev_old_state = self.p_usbd->dev_state;
		self.p_usbd->dev_state = USBD_STATE_SUSPENDED;

		/* Enter in STOP mode. */
		if (p_pcd->Init.low_power_enable) {
			/* Set SLEEPDEEP bit and SleepOnExit of Cortex System Control Register. */
			SCB->SCR |= (uint32_t)((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
		}
		break;
	}
}

err_t pcd_init(USBD_HandleTypeDef *p_usbd)
{
	self.p_usbd = p_usbd;

	return ERR_OK;
}
