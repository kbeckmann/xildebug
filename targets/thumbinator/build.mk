PLATFORM := stm32f0xx

TARGET_SRCS += \
	targets/$(TARGET)/target.c \
	targets/$(TARGET)/led.c \

FEAT_POWER_PROFILER	:= 1
FEAT_DUT_SWITCH		:= 1
