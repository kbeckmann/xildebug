#include <stdbool.h>
#include <string.h>

#include "pcd.h"
#include "stm32_hal.h"
#include "usb/core.h"
#include "usb/ctlreq.h"

#define MODULE_NAME				pcd
#include "macros.h"

static struct {
	USBD_HandleTypeDef *p_usbd;
} SELF;

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

	memcpy(&SELF.p_usbd->request.bmRequest, p_setup, 1);
	SELF.p_usbd->request.bRequest  = *(uint8_t *)(p_setup +  1);
	SELF.p_usbd->request.wValue    =     SWAPBYTE(p_setup +  2);
	SELF.p_usbd->request.wIndex    =     SWAPBYTE(p_setup +  4);
	SELF.p_usbd->request.wLength   =     SWAPBYTE(p_setup +  6);

	SELF.p_usbd->ep0_state = USBD_EP0_SETUP;
	SELF.p_usbd->ep0_data_len = SELF.p_usbd->request.wLength;

	switch (SELF.p_usbd->request.bmRequest.recipient) {
	case USB_REQ_RECIPIENT_DEVICE:
		USBD_StdDevReq(SELF.p_usbd, p_pcd, &SELF.p_usbd->request);
		break;

	case USB_REQ_RECIPIENT_INTERFACE:
		USBD_StdItfReq(SELF.p_usbd, p_pcd, &SELF.p_usbd->request);
		break;

	case USB_REQ_RECIPIENT_ENDPOINT:
		USBD_StdEPReq(SELF.p_usbd, p_pcd, &SELF.p_usbd->request);
		break;

	default:
		HAL_PCD_EP_SetStall(p_pcd, *(uint8_t*)(&SELF.p_usbd->request.bmRequest) & 0x80);
		break;
	}
}

void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *p_pcd, uint8_t epnum)
{
	if (epnum == 0x00) {
		USBD_EndpointTypeDef *p_ep = &SELF.p_usbd->ep_out[0];
		uint8_t *p_data = p_pcd->OUT_ep[epnum].xfer_buff;

		if (SELF.p_usbd->ep0_state == USBD_EP0_DATA_OUT) {
			if (p_ep->rem_length > p_ep->maxpacket) {
				p_ep->rem_length -= p_ep->maxpacket;

				HAL_PCD_EP_Receive(p_pcd, 0x00, p_data, MIN(p_ep->rem_length, p_ep->maxpacket));
			} else {
				for (int i = 0; i < USBD_MAX_NUM_CLASSES; ++i) {
					if ((SELF.p_usbd->pClasses[i]->EP0_RxReady != NULL) && (SELF.p_usbd->dev_state == USBD_STATE_CONFIGURED))
						SELF.p_usbd->pClasses[i]->EP0_RxReady(SELF.p_usbd);
				}

				USBD_CtlSendStatus(SELF.p_usbd);
			}
		}
	} else {
		for (int i = 0; i < USBD_MAX_NUM_CLASSES; ++i) {
			if ((SELF.p_usbd->pClasses[i]->DataOut != NULL) && (SELF.p_usbd->dev_state == USBD_STATE_CONFIGURED))
				SELF.p_usbd->pClasses[i]->DataOut(SELF.p_usbd, epnum);
		}
	}
}

void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *p_pcd, uint8_t epnum)
{
	uint8_t *p_data = p_pcd->IN_ep[epnum].xfer_buff;
	USBD_EndpointTypeDef *p_ep;

	if (epnum == 0x00) {
		p_ep = &SELF.p_usbd->ep_in[0];

		if ( SELF.p_usbd->ep0_state == USBD_EP0_DATA_IN) {
			if (p_ep->rem_length > p_ep->maxpacket) {
				p_ep->rem_length -=  p_ep->maxpacket;

				HAL_PCD_EP_Transmit(p_pcd, 0x00, p_data, p_ep->rem_length);

				/* Prepare endpoint for premature end of transfer */
				HAL_PCD_EP_Receive(p_pcd, 0, NULL, 0);
			} else { /* last packet is MPS multiple, so send ZLP packet */
				if ((p_ep->total_length % p_ep->maxpacket == 0) && (p_ep->total_length >= p_ep->maxpacket) && (p_ep->total_length < SELF.p_usbd->ep0_data_len )) {
					HAL_PCD_EP_Transmit(p_pcd, 0x00, NULL, 0);
					SELF.p_usbd->ep0_data_len = 0;

					/* Prepare endpoint for premature end of transfer */
					HAL_PCD_EP_Receive(p_pcd, 0, NULL, 0);
				} else {
					for (int i = 0; i < USBD_MAX_NUM_CLASSES; ++i) {
						if ((SELF.p_usbd->pClasses[i]->EP0_TxSent != NULL) && (SELF.p_usbd->dev_state == USBD_STATE_CONFIGURED))
							SELF.p_usbd->pClasses[i]->EP0_TxSent(SELF.p_usbd);
					}

					SELF.p_usbd->ep0_state = USBD_EP0_STATUS_OUT;
					HAL_PCD_EP_Receive(p_pcd, 0x00, NULL, 0);
				}
			}
		}
	} else {
		for (int i = 0; i < USBD_MAX_NUM_CLASSES; ++i) {
			if ((SELF.p_usbd->pClasses[i]->DataIn != NULL) && (SELF.p_usbd->dev_state == USBD_STATE_CONFIGURED))
				SELF.p_usbd->pClasses[i]->DataIn(SELF.p_usbd, epnum | 0x80);
		}
	}
}

void HAL_PCD_SOFCallback(PCD_HandleTypeDef *p_pcd)
{
	if (SELF.p_usbd->dev_state != USBD_STATE_CONFIGURED)
		return;

	for (int i = 0; i < USBD_MAX_NUM_CLASSES; ++i) {
		if (SELF.p_usbd->pClasses[i]->SOF != NULL)
			SELF.p_usbd->pClasses[i]->SOF(SELF.p_usbd);
	}
}

void HAL_PCD_ResetCallback(PCD_HandleTypeDef *p_pcd)
{
	SELF.p_usbd->dev_speed = USBD_SPEED_FULL;

	/* Open EP0 OUT */
	HAL_PCD_EP_Open(p_pcd, 0x00, USB_MAX_EP0_SIZE, USBD_EP_TYPE_CTRL);

	SELF.p_usbd->ep_out[0].maxpacket = USB_MAX_EP0_SIZE;

	/* Open EP0 IN */
	HAL_PCD_EP_Open(p_pcd, 0x80, USB_MAX_EP0_SIZE, USBD_EP_TYPE_CTRL);

	SELF.p_usbd->ep_in[0].maxpacket = USB_MAX_EP0_SIZE;

	/* Upon Reset call user call back */
	SELF.p_usbd->dev_state = USBD_STATE_DEFAULT;

	for (int i = 0; i < USBD_MAX_NUM_CLASSES; ++i)
		SELF.p_usbd->pClasses[i]->DeInit(SELF.p_usbd, SELF.p_usbd->dev_config);
}

void HAL_PCD_SuspendCallback(PCD_HandleTypeDef *p_pcd)
{
	/* Inform USB library that core enters in suspend Mode. */
	SELF.p_usbd->dev_old_state = SELF.p_usbd->dev_state;
	SELF.p_usbd->dev_state = USBD_STATE_SUSPENDED;

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

	SELF.p_usbd->dev_state = SELF.p_usbd->dev_old_state;
}

void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *p_pcd)
{
	/* Free Class Resources */
	SELF.p_usbd->dev_state = USBD_STATE_DEFAULT;

	for (int i = 0; i < USBD_MAX_NUM_CLASSES; ++i)
		SELF.p_usbd->pClasses[i]->DeInit(SELF.p_usbd, SELF.p_usbd->dev_config);
}

#if defined(STM32L432xx) || defined(STM32L433xx) || defined(STM32L442xx) || defined(STM32L443xx) || \
    defined(STM32L452xx) || defined(STM32L462xx) || \
    defined(STM32L475xx) || defined(STM32L476xx) || defined(STM32L485xx) || defined(STM32L486xx) || \
    defined(STM32L496xx) || defined(STM32L4A6xx) || \
    defined(STM32L4R5xx) || defined(STM32L4R7xx) || defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
void HAL_PCDEx_LPM_Callback(PCD_HandleTypeDef *p_pcd, PCD_LPM_MsgTypeDef msg)
{
	switch (msg){
	case PCD_LPM_L0_ACTIVE:
		if (p_pcd->Init.low_power_enable){
			SystemClock_Config();

			/* Reset SLEEPDEEP bit of Cortex System Control Register. */
			SCB->SCR &= (uint32_t)~((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
		}

		SELF.p_usbd->dev_state = SELF.p_usbd->dev_old_state;
		break;

	case PCD_LPM_L1_ACTIVE:
		SELF.p_usbd->dev_old_state = SELF.p_usbd->dev_state;
		SELF.p_usbd->dev_state = USBD_STATE_SUSPENDED;

		/* Enter in STOP mode. */
		if (p_pcd->Init.low_power_enable) {
			/* Set SLEEPDEEP bit and SleepOnExit of Cortex System Control Register. */
			SCB->SCR |= (uint32_t)((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
		}
		break;
	}
}
#endif

err_t pcd_init(USBD_HandleTypeDef *p_usbd)
{
	SELF.p_usbd = p_usbd;

	return ERR_OK;
}
