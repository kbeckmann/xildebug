
PLATFORM_SRCS := \
	platforms/$(PLATFORM)/gpio.c \
	platforms/$(PLATFORM)/startup_stm32f072xb.s \
	platforms/$(PLATFORM)/stm32f0_hal.c \
	platforms/$(PLATFORM)/uart.c \
	platforms/stm32_common/pcd.c \
	platforms/stm32_common/usb/cdc.c \
	platforms/stm32_common/usb/core.c \
	platforms/stm32_common/usb/ctlreq.c \
	platforms/stm32_common/usb/hid.c \
	platforms/stm32_common/usb/usb.c \
	SDK/Drivers/CMSIS/Device/ST/STM32F0xx/Source/Templates/system_stm32f0xx.c \
	SDK/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_adc_ex.c \
	SDK/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_adc.c \
	SDK/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_cortex.c \
	SDK/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_dma.c \
	SDK/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_gpio.c \
	SDK/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_i2c_ex.c \
	SDK/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_i2c.c \
	SDK/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_pcd_ex.c \
	SDK/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_pcd.c \
	SDK/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_pwr_ex.c \
	SDK/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_rcc_ex.c \
	SDK/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_rcc.c \
	SDK/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_tim_ex.c \
	SDK/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_tim.c \
	SDK/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_uart_ex.c \
	SDK/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_uart.c \
	SDK/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal.c \
	SDK/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM0/port.c \

# todo
#	platforms/$(PLATFORM)/adc.c \
#	platforms/$(PLATFORM)/i2c.c \

PLATFORM_INCLUDES := \
	platforms/$(PLATFORM)/config \
	platforms/stm32_common \
	SDK/Drivers/CMSIS/Device/ST/STM32F0xx/Include \
	SDK/Drivers/CMSIS/Include \
	SDK/Drivers/STM32F0xx_HAL_Driver/Inc \
	SDK/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM0

LDSCRIPT := scripts/ld/STM32F072C8Tx_FLASH.ld
DEFS     += USE_HAL_DRIVER STM32F072xB

CFLAGS  += -mcpu=cortex-m0 -mthumb -mfloat-abi=soft
LDFLAGS += -mcpu=cortex-m0 -mthumb -mfloat-abi=soft -T$(LDSCRIPT)

OPENOCD_TARGET := stm32f0x
