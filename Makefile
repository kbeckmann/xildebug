CROSS_COMPILE ?= arm-none-eabi-

CC      := $(CROSS_COMPILE)gcc
LD      := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy

TARGET ?= xildebug_big

# Include target first since it chooses a platform
include targets/$(TARGET)/build.mk
include platforms/$(PLATFORM)/build.mk

TARGET_SRCS += \
	$(PLATFORM_SRCS) \

TARGET_INCLUDES += \
	targets \
	targets/$(TARGET) \
	platforms \
	platforms/$(PLATFORM) \
	$(PLATFORM_INCLUDES) \

MIDDLEWARE_SRCS += \
	SDK/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS/cmsis_os.c \
	SDK/Middlewares/Third_Party/FreeRTOS/Source/list.c \
	SDK/Middlewares/Third_Party/FreeRTOS/Source/queue.c \
	SDK/Middlewares/Third_Party/FreeRTOS/Source/tasks.c \
	SDK/Middlewares/Third_Party/FreeRTOS/Source/timers.c

MIDDLEWARE_INCLUDES += \
	SDK/Middlewares/Third_Party/FreeRTOS/Source/include \

APP_SRCS := \
	app/drivers/led.c \
	app/drivers/max14662.c \
	app/drivers/mcp4018t.c \
	app/drivers/pcd.c \
	app/drivers/uart.c \
	app/drivers/usb.c \
	app/drivers/usb/core.c \
	app/drivers/usb/cdc.c \
	app/drivers/usb/ctlreq.c \
	app/drivers/usb/hid.c \
	app/cdc_uart_bridge.c \
	app/errorhandler.c \
	app/freertos.c \
	app/freertos-openocd.c \
	app/main.c \
	app/power.c

APP_INCLUDES := \
	app/config \
	app

SRCS     := $(TARGET_SRCS) $(MIDDLEWARE_SRCS) $(APP_SRCS)
INCLUDES := $(TARGET_INCLUDES) $(MIDDLEWARE_INCLUDES) $(APP_INCLUDES)

CFLAGS   += \
	-Wall -Wextra -Wpedantic -Wno-unused-parameter -std=c99 -Os -g \
	-ffunction-sections -fdata-sections -fomit-frame-pointer \
	$(addprefix -D, $(DEFS)) \
	$(addprefix -I, $(INCLUDES))

LDFLAGS  +=
LIBS     := -Wl,--gc-sections,--undefined=uxTopUsedPriority --specs=nano.specs -lc -lnosys

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
	@./scripts/size.sh $@

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

ifneq ($(DAPLINK_SERIAL),)
DAPLINK_SERIAL_CMD := "-c cmsis_dap_serial $(DAPLINK_SERIAL)"
endif

daplink:
	@openocd \
	-f interface/cmsis-dap.cfg \
	-f target/stm32l4x.cfg \
	$(DAPLINK_SERIAL_CMD) \
	-c "stm32l4x.cpu configure -rtos FreeRTOS" \
	-c "init ; reset halt"
.PHONY: daplink

flash: out/app.elf
	@echo -e "flash banks\nreset halt\nprogram $< verify\nreset run\nexit\n" | nc localhost 4444
.PHONY: flash

ifneq ("$(MAKECMDGOALS)","clean")
-include $(DEPS)
endif
