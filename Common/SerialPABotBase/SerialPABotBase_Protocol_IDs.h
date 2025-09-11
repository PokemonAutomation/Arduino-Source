/*  SerialPABotBase Protocol IDs
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SerialPABotBase_Protocol_IDs_H
#define PokemonAutomation_SerialPABotBase_Protocol_IDs_H

#include <stdint.h>

//
//  Program IDs
//

typedef uint8_t pabb_ProgramID;

#define PABB_PID_UNSPECIFIED                                0x00

//  Old AVR8
#define PABB_PID_PABOTBASE_12KB                             0x08
#define PABB_PID_PABOTBASE_31KB                             0x09

//  New AVR8
#define PABB_PID_PABOTBASE_ArduinoUnoR3                     0x01
#define PABB_PID_PABOTBASE_ArduinoLeonardo                  0x02
#define PABB_PID_PABOTBASE_ProMicro                         0x03
#define PABB_PID_PABOTBASE_Teensy2                          0x04
#define PABB_PID_PABOTBASE_TeensyPP2                        0x05

//  Misc.
#define PABB_PID_PABOTBASE_CH552                            0x0a

//  ESP32
#define PABB_PID_PABOTBASE_ESP32                            0x10
#define PABB_PID_PABOTBASE_ESP32S3                          0x12

//  Raspberry Pi
#define PABB_PID_PABOTBASE_PicoW_USB                        0x20
#define PABB_PID_PABOTBASE_PicoW_UART                       0x21


//
//  Controller IDs
//

typedef uint32_t pabb_ControllerID;

#define PABB_CID_NONE                                       0

#define PABB_CID_StandardHid_Keyboard                       0x0100

#define PABB_CID_NintendoSwitch_WiredController             0x1000
#define PABB_CID_NintendoSwitch_WiredProController          0x1001
#define PABB_CID_NintendoSwitch_WirelessProController       0x1002
#define PABB_CID_NintendoSwitch_LeftJoycon                  0x1003
#define PABB_CID_NintendoSwitch_RightJoycon                 0x1004

#define PABB_CID_NintendoSwitch2_WiredController            0x1010
#define PABB_CID_NintendoSwitch2_WiredProController         0x1011
#define PABB_CID_NintendoSwitch2_WirelessProController      0x1012
#define PABB_CID_NintendoSwitch2_LeftJoycon                 0x1013
#define PABB_CID_NintendoSwitch2_RightJoycon                0x1014


#endif
