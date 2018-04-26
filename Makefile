CROSS_COMPILE ?= arm-none-eabi-

CC      := $(CROSS_COMPILE)gcc
LD      := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy

SDK_SRCS := \
	SDK/Drivers/CMSIS/Device/ST/STM32L4xx/Source/Templates/gcc/startup_stm32l433xx.s \
	SDK/Drivers/CMSIS/Device/ST/STM32L4xx/Source/Templates/system_stm32l4xx.c \
	SDK/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_adc.c \
	SDK/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_adc_ex.c \
	SDK/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal.c \
	SDK/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_cortex.c \
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
	SDK/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_usb.c \
	SDK/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c \
	SDK/Middlewares/ST/STM32_USB_Device_Library/Class/HID/Src/usbd_hid.c \
	SDK/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c \
	SDK/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c \
	SDK/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c \
	SDK/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS/cmsis_os.c \
	SDK/Middlewares/Third_Party/FreeRTOS/Source/list.c \
	SDK/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c \
	SDK/Middlewares/Third_Party/FreeRTOS/Source/queue.c \
	SDK/Middlewares/Third_Party/FreeRTOS/Source/tasks.c \
	SDK/Middlewares/Third_Party/FreeRTOS/Source/timers.c

SDK_INCLUDES := \
	SDK/Drivers/CMSIS/Device/ST/STM32L4xx/Include \
	SDK/Drivers/CMSIS/Include \
	SDK/Drivers/STM32L4xx_HAL_Driver/Inc \
	SDK/Middlewares/ST/STM32_USB_Device_Library/Core/Inc \
	SDK/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc \
	SDK/Middlewares/ST/STM32_USB_Device_Library/Class/HID/Inc \
	SDK/Middlewares/Third_Party/FreeRTOS/Source/include \
	SDK/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F

APP_SRCS := \
	app/drivers/adc.c \
	app/drivers/gpio.c \
	app/drivers/i2c.c \
	app/drivers/led.c \
	app/drivers/max14662.c \
	app/drivers/mcp4018t.c \
	app/drivers/pcd.c \
	app/drivers/uart.c \
	app/drivers/usb.c \
	app/drivers/usb_cdc.c \
	app/drivers/usb_hid.c \
	app/drivers/usb_ll.c \
	app/freertos.c \
	app/main.c \
	app/power.c \
	app/stm32l4_hal.c

APP_INCLUDES := \
	app/config \
	app

SRCS     := $(SDK_SRCS) $(APP_SRCS)
INCLUDES := $(SDK_INCLUDES) $(APP_INCLUDES)

DEFS     := USE_HAL_DRIVER STM32L433xx
LDSCRIPT := scripts/ld/STM32L433CCUx_FLASH.ld

CFLAGS   := \
	-Wall -Wextra -Wno-unused-parameter -std=c99 -Os -g \
	-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard \
	-ffunction-sections -fdata-sections -fomit-frame-pointer \
	$(addprefix -D, $(DEFS)) \
	$(addprefix -I, $(INCLUDES))

LDFLAGS  := -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -T$(LDSCRIPT)

LIBS     := -Wl,--gc-sections --specs=nano.specs -lc -lnosys

OBJS     := $(patsubst %.c,out/obj/%.o, $(filter %.c, $(SRCS))) $(patsubst %.s,out/obj/%.o, $(filter %.s, $(SRCS)))
DEPS     := $(patsubst %.o,%.d,$(OBJS))

all: out/app.bin
.PHONY: all

out/obj/%.o: %.s
	@echo "CC $<"
	@mkdir -p $(dir $@)
	@$(CC) -MM -MF $(subst .o,.d,$@) -MP -MT "$@ $(subst .o,.d,$@)" $(CFLAGS) $<
	@$(CC) $(CFLAGS) -c -o $@ $<

out/obj/%.o: %.c
	@echo "CC $<"
	@mkdir -p $(dir $@)
	@$(CC) -MM -MF $(subst .o,.d,$@) -MP -MT "$@ $(subst .o,.d,$@)" $(CFLAGS) $<
	@$(CC) $(CFLAGS) -c -o $@ $<

out/app.elf: $(OBJS)
	@echo "LD $@"
	@$(CC) $(CFLAGS) -o $@ $^ $(LIBS) $(LDFLAGS) -Xlinker -Map=$@.map

%.bin: %.elf
	@$(OBJCOPY) -O binary $< $@

clean:
	@echo "Cleaning"
	@rm -rf out/
.PHONY: clean

stlink:
	@openocd \
	-f interface/stlink-v2.cfg \
	-c "transport select hla_swd" \
	-f target/stm32l4x.cfg \
	-c "init ; reset halt"
.PHONY: stlink

daplink:
	@openocd \
	-f interface/cmsis-dap.cfg \
	-f target/stm32l4x.cfg \
	-c "init ; reset halt"
.PHONY: daplink

flash: out/app.elf
	@echo -e "flash banks\nreset halt\nprogram $< verify\nreset run\nexit\n" | nc localhost 4444
.PHONY: flash

ifneq ("$(MAKECMDGOALS)","clean")
-include $(DEPS)
endif
