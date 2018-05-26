
PLATFORM_SRCS := \
	SDK/Drivers/CMSIS/Device/ST/STM32L4xx/Source/Templates/gcc/startup_stm32l433xx.s \
	SDK/Drivers/CMSIS/Device/ST/STM32L4xx/Source/Templates/system_stm32l4xx.c \
	SDK/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_adc.c \
	SDK/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_adc_ex.c \
	SDK/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal.c \
	SDK/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_cortex.c \
	SDK/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma.c \
	SDK/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_gpio.c \
	SDK/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c.c \
	SDK/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c_ex.c \
	SDK/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pcd.c \
	SDK/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pcd_ex.c \
	SDK/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr_ex.c \
	SDK/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc.c \
	SDK/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc_ex.c \
	SDK/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim.c \
	SDK/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim_ex.c \
	SDK/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart.c \
	SDK/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart_ex.c \
	SDK/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_usb.c \
	SDK/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c \
	platforms/$(PLATFORM)/stm32l4_hal.c \
	platforms/$(PLATFORM)/adc.c \
	platforms/$(PLATFORM)/gpio.c \
	platforms/$(PLATFORM)/i2c.c \

PLATFORM_INCLUDES := \
	platforms/$(PLATFORM)/config \
	SDK/Drivers/CMSIS/Device/ST/STM32L4xx/Include \
	SDK/Drivers/CMSIS/Include \
	SDK/Drivers/STM32L4xx_HAL_Driver/Inc \
	SDK/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F

LDSCRIPT := scripts/ld/STM32L433CCUx_FLASH.ld
DEFS     += USE_HAL_DRIVER STM32L433xx

CFLAGS  += -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
LDFLAGS += -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -T$(LDSCRIPT)

