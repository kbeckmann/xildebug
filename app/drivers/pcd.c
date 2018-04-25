#include <usbd_core.h>

#include "stm32l4xx_hal.h"

extern USBD_StatusTypeDef USBD_LL_BatteryCharging(USBD_HandleTypeDef *p_dev);
extern HAL_StatusTypeDef SystemClock_Config(void);

void HAL_PCDEx_SetConnectionState(PCD_HandleTypeDef *p_pcd, uint8_t state);

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
	USBD_LL_SetupStage(p_pcd->pData, (uint8_t *)p_pcd->Setup);
}

void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *p_pcd, uint8_t epnum)
{
	USBD_LL_DataOutStage(p_pcd->pData, epnum, p_pcd->OUT_ep[epnum].xfer_buff);
}

void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *p_pcd, uint8_t epnum)
{
	USBD_LL_DataInStage(p_pcd->pData, epnum, p_pcd->IN_ep[epnum].xfer_buff);
}

void HAL_PCD_SOFCallback(PCD_HandleTypeDef *p_pcd)
{
	USBD_LL_SOF(p_pcd->pData);
}

void HAL_PCD_ResetCallback(PCD_HandleTypeDef *p_pcd)
{
	USBD_LL_SetSpeed(p_pcd->pData, USBD_SPEED_FULL);
	USBD_LL_Reset(p_pcd->pData);
}

void HAL_PCD_SuspendCallback(PCD_HandleTypeDef *p_pcd)
{
	/* Inform USB library that core enters in suspend Mode. */
	USBD_LL_Suspend(p_pcd->pData);

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

	USBD_LL_Resume(p_pcd->pData);
}

void HAL_PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef *p_pcd, uint8_t epnum)
{
	USBD_LL_IsoOUTIncomplete(p_pcd->pData, epnum);
}

void HAL_PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *p_pcd, uint8_t epnum)
{
	USBD_LL_IsoINIncomplete(p_pcd->pData, epnum);
}

void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *p_pcd)
{
	USBD_LL_DevConnected(p_pcd->pData);
}

void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *p_pcd)
{
	USBD_LL_DevDisconnected(p_pcd->pData);
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

			USBD_LL_Resume(p_pcd->pData);
			break;

		case PCD_LPM_L1_ACTIVE:
			USBD_LL_Suspend(p_pcd->pData);

			/* Enter in STOP mode. */
			if (p_pcd->Init.low_power_enable) {
				/* Set SLEEPDEEP bit and SleepOnExit of Cortex System Control Register. */
				SCB->SCR |= (uint32_t)((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
			}
			break;
	}
}

void HAL_PCDEx_SetConnectionState(PCD_HandleTypeDef *p_pcd, uint8_t state)
{

}

void HAL_PCDEx_BCD_Callback(PCD_HandleTypeDef *p_pcd, PCD_BCD_MsgTypeDef msg)
{
	if (p_pcd->battery_charging_active == ENABLE) {
		switch(msg) {
			case PCD_BCD_CONTACT_DETECTION:
				break;

			case PCD_BCD_STD_DOWNSTREAM_PORT:
				break;

			case PCD_BCD_CHARGING_DOWNSTREAM_PORT:
				break;

			case PCD_BCD_DEDICATED_CHARGING_PORT:
				break;

			case PCD_BCD_DISCOVERY_COMPLETED:
				USBD_Start(p_pcd->pData);
				break;

			case PCD_BCD_ERROR:
			default:
				break;
		}
	}
}
