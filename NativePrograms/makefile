#
#             LUFA Library
#     Copyright (C) Dean Camera, 2014.
#
#  dean [at] fourwalledcubicle [dot] com
#           www.lufa-lib.org
# 
#  Modified by https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
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

#MCU          = atmega16u2
ARCH         = AVR8
F_CPU        = 16000000
F_USB        = $(F_CPU)
OPTIMIZATION = s
#TARGET       = TurboA
SRC          = $(TARGET).c
LUFA_PATH    = ./LUFA
CC_FLAGS     = -DUSE_LUFA_CONFIG_HEADER -I./ -I../ -ILUFA/ -Wno-unused-function -Werror




#   Board
ifeq ($(BOARD_TYPE), ArduinoUnoR3)
MCU := atmega16u2
SRC += DeviceFramework/Board-atmega16u2-ArduinoUnoR3.c
endif
ifeq ($(BOARD_TYPE), ProMicro)
MCU := atmega32u4
SRC += DeviceFramework/Board-atmega32u4-ProMicro.c
endif
ifeq ($(BOARD_TYPE), Teensy2)
MCU := atmega32u4
SRC += DeviceFramework/Board-atmega32u4-Teensy2.c
endif
ifeq ($(BOARD_TYPE), TeensyPP2)
MCU := at90usb1286
SRC += DeviceFramework/Board-at90usb1286-Teensy2.c
endif

#   Framework
SRC += $(LUFA_SRC_USB)
SRC += ../Common/SwitchFramework/FrameworkSettings.c
SRC += DeviceFramework/DeviceSettings.c
SRC += PokemonSwShLibraries/PokemonCallbacks.c
ifneq ("$(wildcard ../../Internal/NativePrograms/SwitchFramework/Switch_PushButtons.c)","")
CC_FLAGS += -I../../Internal
SRC += ../Common/CRC32.c
SRC += ../../Internal/NativePrograms/SwitchFramework/uart.c
SRC += ../../Internal/NativePrograms/SwitchFramework/HardwareUSB.c
ifeq ($(TARGET), PABotBase)
SRC += ../../Internal/NativePrograms/SwitchFramework/CommandQueue.c
else
SRC += ../../Internal/NativePrograms/SwitchFramework/CommandQueueNull.c
endif
SRC += ../../Internal/NativePrograms/SwitchFramework/HardwareSerial.c
SRC += ../../Internal/NativePrograms/SwitchFramework/Controller.c
SRC += ../../Internal/NativePrograms/SwitchFramework/Switch_PushButtons.c
SRC += ../../Internal/NativePrograms/SwitchFramework/Switch_ScalarButtons.c
else
LD_FLAGS += obj/obj-$(MCU)/CRC32.o
LD_FLAGS += obj/obj-$(MCU)/uart.o
LD_FLAGS += obj/obj-$(MCU)/HardwareUSB.o
ifeq ($(TARGET), PABotBase)
LD_FLAGS += obj/obj-$(MCU)/CommandQueue.o
else
LD_FLAGS += obj/obj-$(MCU)/CommandQueueNull.o
endif
LD_FLAGS += obj/obj-$(MCU)/HardwareSerial.o
LD_FLAGS += obj/obj-$(MCU)/Controller.o
LD_FLAGS += obj/obj-$(MCU)/Switch_PushButtons.o
LD_FLAGS += obj/obj-$(MCU)/Switch_ScalarButtons.o
endif

#   Pokemon Sword/Shield
SRC += ../Common/PokemonSwSh/PokemonSettings.c
ifneq ("$(wildcard ../../Internal/NativePrograms/PokemonSwShPrograms/$(TARGET)_Core.c)","")
SRC += ../../Internal/NativePrograms/PokemonSwShPrograms/$(TARGET)_Core.c
else ifneq ("$(wildcard PokemonSwShPrograms/$(TARGET)_Core.c)","")
SRC += PokemonSwShPrograms/$(TARGET)_Core.c
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
