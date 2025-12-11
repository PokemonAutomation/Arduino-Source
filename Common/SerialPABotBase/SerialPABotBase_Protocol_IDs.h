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
#define PABB_PID_PABOTBASE_Pico1W_USB                       0x20
#define PABB_PID_PABOTBASE_Pico1W_UART                      0x21
#define PABB_PID_PABOTBASE_Pico2W_USB                       0x22
#define PABB_PID_PABOTBASE_Pico2W_UART                      0x23


//
//  Controller IDs
//

typedef uint32_t pabb_ControllerID;

#define PABB_CID_NONE                                       0

#define PABB_CID_StandardHid_Keyboard                       0x0100

#define PABB_CID_NintendoSwitch_WiredController             0x1000
#define PABB_CID_NintendoSwitch2_WiredController            0x1010

#define PABB_CID_NintendoSwitch_WiredProController          0x1100
#define PABB_CID_NintendoSwitch_WiredLeftJoycon             0x1101
#define PABB_CID_NintendoSwitch_WiredRightJoycon            0x1102
#define PABB_CID_NintendoSwitch_WirelessProController       0x1180
#define PABB_CID_NintendoSwitch_WirelessLeftJoycon          0x1181
#define PABB_CID_NintendoSwitch_WirelessRightJoycon         0x1182

#define PABB_CID_NintendoSwitch2_WiredProController         0x1200
#define PABB_CID_NintendoSwitch2_WiredLeftJoycon            0x1201
#define PABB_CID_NintendoSwitch2_WiredRightJoycon           0x1202
#define PABB_CID_NintendoSwitch2_WirelessProController      0x1280
#define PABB_CID_NintendoSwitch2_WirelessLeftJoycon         0x1281
#define PABB_CID_NintendoSwitch2_WirelessRightJoycon        0x1282


#endif
