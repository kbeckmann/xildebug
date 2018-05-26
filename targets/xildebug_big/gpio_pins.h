#pragma once

#include "stm32l4xx_hal.h"

#define GPIO0_Pin GPIO_PIN_8
#define GPIO0_GPIO_Port ((uint32_t) GPIOB)
#define GPIO1_Pin GPIO_PIN_9
#define GPIO1_GPIO_Port ((uint32_t) GPIOB)
#define GPIO2_Pin GPIO_PIN_13
#define GPIO2_GPIO_Port ((uint32_t) GPIOC)
#define GPIO3_Pin GPIO_PIN_14
#define GPIO3_GPIO_Port ((uint32_t) GPIOC)

#define DUT_UART_TX_LED_Pin GPIO_PIN_0
#define DUT_UART_TX_LED_GPIO_Port ((uint32_t) GPIOH)
#define DUT_UART_RX_LED_Pin GPIO_PIN_1
#define DUT_UART_RX_LED_GPIO_Port ((uint32_t) GPIOH)

#define DUT_VDD_IN_Pin GPIO_PIN_0
#define DUT_VDD_IN_GPIO_Port ((uint32_t) GPIOA)
#define DUT_VDD_OUT_Pin GPIO_PIN_1
#define DUT_VDD_OUT_GPIO_Port ((uint32_t) GPIOA)

#define DUT_SWD_LED_Pin GPIO_PIN_4
#define DUT_SWD_LED_GPIO_Port ((uint32_t) GPIOA)

#define SHUNT1_EN_Pin GPIO_PIN_5
#define SHUNT1_EN_GPIO_Port ((uint32_t) GPIOA)

#define SHUNT2_EN_Pin GPIO_PIN_6
#define SHUNT2_EN_GPIO_Port ((uint32_t) GPIOA)

#define DUT_VDD_EN_Pin GPIO_PIN_0
#define DUT_VDD_EN_GPIO_Port ((uint32_t) GPIOB)

#define DUT_SWCLK_Pin GPIO_PIN_1
#define DUT_SWCLK_GPIO_Port ((uint32_t) GPIOB)
#define DUT_SWDIO_Pin GPIO_PIN_2
#define DUT_SWDIO_GPIO_Port ((uint32_t) GPIOB)

#define DUT_UART_TX_Pin GPIO_PIN_10
#define DUT_UART_TX_GPIO_Port ((uint32_t) GPIOB)

#define DUT_UART_RX_Pin GPIO_PIN_11
#define DUT_UART_RX_GPIO_Port ((uint32_t) GPIOB)

#define LED_RGB_B_Pin GPIO_PIN_12
#define LED_RGB_B_GPIO_Port ((uint32_t) GPIOB)

#define LED_RGB_G_Pin GPIO_PIN_8
#define LED_RGB_G_GPIO_Port ((uint32_t) GPIOA)

#define LED_RGB_R_Pin GPIO_PIN_15
#define LED_RGB_R_GPIO_Port ((uint32_t) GPIOA)
