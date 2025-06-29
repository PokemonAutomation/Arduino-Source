/*  SerialPABotBase Protocol IDs
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#ifndef PokemonAutomation_SerialPABotBase_Protocol_IDs_H
#define PokemonAutomation_SerialPABotBase_Protocol_IDs_H


//  Program IDs
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


//  Controller IDs
#define PABB_CID_NONE                                       0
#define PABB_CID_NINTENDO_SWITCH_WIRED_PRO_CONTROLLER       1
#define PABB_CID_NINTENDO_SWITCH_WIRELESS_PRO_CONTROLLER    2
#define PABB_CID_NINTENDO_SWITCH_LEFT_JOYCON                3
#define PABB_CID_NINTENDO_SWITCH_RIGHT_JOYCON               4


#endif
