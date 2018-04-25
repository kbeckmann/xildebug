#pragma once

#include <stm32l4xx_hal.h>
#include <stdio.h>

#define USBD_MAX_NUM_INTERFACES			1
#define USBD_MAX_NUM_CONFIGURATION		1
#define USBD_MAX_STR_DESC_SIZ			512
#define USBD_SUPPORT_USER_STRING		1
#define USBD_DEBUG_LEVEL				3
#define USBD_LPM_ENABLED				1
#define USBD_SELF_POWERED				0
#define USBD_FS_ID						0

#if (USBD_DEBUG_LEVEL > 0)
#define USBD_UsrLog(...)	printf(__VA_ARGS__); printf("\n");
#else
#define USBD_UsrLog(...)
#endif

#if (USBD_DEBUG_LEVEL > 1)
#define USBD_ErrLog(...)	printf("ERROR: "); printf(__VA_ARGS__); printf("\n");
#else
#define USBD_ErrLog(...)
#endif

#if (USBD_DEBUG_LEVEL > 2)
#define USBD_DbgLog(...)	printf("DEBUG: "); printf(__VA_ARGS__); printf("\n");
#else
#define USBD_DbgLog(...)
#endif

void *USBD_malloc(uint32_t size);
void USBD_free(void *p);
