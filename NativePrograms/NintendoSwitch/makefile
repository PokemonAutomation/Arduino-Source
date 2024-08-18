#
#             LUFA Library
#     Copyright (C) Dean Camera, 2014.
#
#  dean [at] fourwalledcubicle [dot] com
#           www.lufa-lib.org
# 
#  Modified by https://github.com/PokemonAutomation/Arduino-Source
#
# --------------------------------------
#         LUFA Project Makefile.
# --------------------------------------

#   MCU Types:
#     atmega16u2    for Arduino UNO R3
#     atmega32u4    for Arduino Micro, and Teensy 2.0
#     at90usb1286   for Teensy 2.0++

#   Set MCU and TARGET here:

ifeq ($(BOARD_TYPE),)
BOARD_TYPE := Teensy2
endif
ifeq ($(TARGET),)
TARGET := TurboA
#TARGET := Sandbox
endif


################################################################################
#   Ignore all this stuff below.

PATH_PUBLIC = ../../
PATH_INTERNAL = ../../../Internal

CATEGORY = NintendoSwitch

#MCU          = atmega16u2
ARCH         = AVR8
F_CPU        = 16000000
F_USB        = $(F_CPU)
OPTIMIZATION = s
#TARGET       = TurboA
SRC          = $(TARGET).c
LUFA_PATH    = $(PATH_PUBLIC)/NativePrograms/LUFA
CC_FLAGS     = -DUSE_LUFA_CONFIG_HEADER -I$(PATH_PUBLIC) -I$(PATH_PUBLIC)/NativePrograms/LUFA/ -Wno-unused-function -Werror




#   Board
ifeq ($(BOARD_TYPE), ArduinoUnoR3)
MCU := atmega16u2
SRC += $(PATH_PUBLIC)/NativePrograms/DeviceFramework/Board-atmega16u2-ArduinoUnoR3.c
endif
ifeq ($(BOARD_TYPE), ArduinoLeonardo)
MCU := atmega32u4
SRC += $(PATH_PUBLIC)/NativePrograms/DeviceFramework/Board-atmega32u4-ArduinoLeonardo.c
endif
ifeq ($(BOARD_TYPE), ProMicro)
MCU := atmega32u4
SRC += $(PATH_PUBLIC)/NativePrograms/DeviceFramework/Board-atmega32u4-ProMicro.c
endif
ifeq ($(BOARD_TYPE), Teensy2)
MCU := atmega32u4
SRC += $(PATH_PUBLIC)/NativePrograms/DeviceFramework/Board-atmega32u4-Teensy2.c
endif
ifeq ($(BOARD_TYPE), TeensyPP2)
MCU := at90usb1286
SRC += $(PATH_PUBLIC)/NativePrograms/DeviceFramework/Board-at90usb1286-Teensy2.c
endif

#   Framework
SRC += $(LUFA_SRC_USB)
SRC += $(PATH_PUBLIC)/NativePrograms/DeviceFramework/Framework/DeviceSettings.c
SRC += $(PATH_PUBLIC)/NativePrograms/NintendoSwitch/FrameworkSettings.c
ifneq ("$(wildcard $(PATH_INTERNAL)/NativePrograms/NintendoSwitch/Framework/NintendoSwitch_PushButtons.c)","")
CC_FLAGS += -I$(PATH_INTERNAL)
SRC += $(PATH_PUBLIC)/Common/CRC32.c
SRC += $(PATH_INTERNAL)/NativePrograms/NintendoSwitch/Framework/HardwareUSB.c
ifeq ($(TARGET), PABotBase)
SRC += $(PATH_INTERNAL)/NativePrograms/NintendoSwitch/Framework/Uart-PABotBase.c
SRC += $(PATH_INTERNAL)/NativePrograms/NintendoSwitch/Framework/CommandQueue.c
else
SRC += $(PATH_INTERNAL)/NativePrograms/NintendoSwitch/Framework/Uart.c
SRC += $(PATH_INTERNAL)/NativePrograms/NintendoSwitch/Framework/CommandQueueNull.c
endif
SRC += $(PATH_INTERNAL)/NativePrograms/NintendoSwitch/Framework/HardwareSerial.c
SRC += $(PATH_INTERNAL)/NativePrograms/NintendoSwitch/Framework/Controller.c
SRC += $(PATH_INTERNAL)/NativePrograms/NintendoSwitch/Framework/NintendoSwitch_PushButtons.c
SRC += $(PATH_INTERNAL)/NativePrograms/NintendoSwitch/Framework/NintendoSwitch_Superscalar.c
else
LD_FLAGS += obj/obj-$(MCU)/CRC32.o
LD_FLAGS += obj/obj-$(MCU)/HardwareUSB.o
ifeq ($(TARGET), PABotBase)
LD_FLAGS += obj/obj-$(MCU)/Uart-PABotBase.o
LD_FLAGS += obj/obj-$(MCU)/CommandQueue.o
else
LD_FLAGS += obj/obj-$(MCU)/Uart.o
LD_FLAGS += obj/obj-$(MCU)/CommandQueueNull.o
endif
LD_FLAGS += obj/obj-$(MCU)/HardwareSerial.o
LD_FLAGS += obj/obj-$(MCU)/Controller.o
LD_FLAGS += obj/obj-$(MCU)/NintendoSwitch_PushButtons.o
LD_FLAGS += obj/obj-$(MCU)/NintendoSwitch_Superscalar.o
endif

#   Program
SRC += $(PATH_PUBLIC)/NativePrograms/PokemonSwSh/PokemonSwSh_Settings.c
ifneq ("$(wildcard $(PATH_INTERNAL)/NativePrograms/$(CATEGORY)/$(TARGET)_Core.c)","")
SRC += $(PATH_INTERNAL)/NativePrograms/$(CATEGORY)/$(TARGET)_Core.c
else ifneq ("$(wildcard Programs/$(TARGET)_Core.c)","")
SRC += Programs/$(TARGET)_Core.c
else ifneq ("$(wildcard obj/obj-$(MCU)/$(TARGET)_Core.o)","")
LD_FLAGS += obj/obj-$(MCU)/$(TARGET)_Core.o
endif

#   Must go at the end or it blows up memory if floating-point is used.
LD_FLAGS += -lm


# Default target
all:

# Include LUFA build script makefiles
include $(LUFA_PATH)/Build/lufa_core.mk
include $(LUFA_PATH)/Build/lufa_sources.mk
include $(LUFA_PATH)/Build/lufa_build.mk
include $(LUFA_PATH)/Build/lufa_cppcheck.mk
include $(LUFA_PATH)/Build/lufa_doxygen.mk
include $(LUFA_PATH)/Build/lufa_dfu.mk
include $(LUFA_PATH)/Build/lufa_hid.mk
include $(LUFA_PATH)/Build/lufa_avrdude.mk
include $(LUFA_PATH)/Build/lufa_atprogram.mk
