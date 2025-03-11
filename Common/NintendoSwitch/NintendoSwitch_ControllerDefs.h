/*  Controller Types and Definitions
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#ifndef PokemonAutomation_NintendoSwitch_ControllerDefs_H
#define PokemonAutomation_NintendoSwitch_ControllerDefs_H

#include <stdbool.h>
#include <stdint.h>


#ifdef __cplusplus

#error "C++ shouldn't include this file."

#else

//  One second = 125 ticks. Thus each tick is 8 milliseconds.
#define TICKS_PER_SECOND    125

//  Buttons
#define Button              uint16_t
#define BUTTON_Y            ((uint16_t)1 <<  0)
#define BUTTON_B            ((uint16_t)1 <<  1)
#define BUTTON_A            ((uint16_t)1 <<  2)
#define BUTTON_X            ((uint16_t)1 <<  3)
#define BUTTON_L            ((uint16_t)1 <<  4)
#define BUTTON_R            ((uint16_t)1 <<  5)
#define BUTTON_ZL           ((uint16_t)1 <<  6)
#define BUTTON_ZR           ((uint16_t)1 <<  7)
#define BUTTON_MINUS        ((uint16_t)1 <<  8)
#define BUTTON_PLUS         ((uint16_t)1 <<  9)
#define BUTTON_LCLICK       ((uint16_t)1 << 10)
#define BUTTON_RCLICK       ((uint16_t)1 << 11)
#define BUTTON_HOME         ((uint16_t)1 << 12)
#define BUTTON_CAPTURE      ((uint16_t)1 << 13)

//  Dpad
#define DpadPosition        uint8_t
#define DPAD_UP             0
#define DPAD_UP_RIGHT       1
#define DPAD_RIGHT          2
#define DPAD_DOWN_RIGHT     3
#define DPAD_DOWN           4
#define DPAD_DOWN_LEFT      5
#define DPAD_LEFT           6
#define DPAD_UP_LEFT        7
#define DPAD_NONE           8

//  Joysticks
#define STICK_MIN           0x00
#define STICK_CENTER        0x80
#define STICK_MAX           0xff

#endif






#endif
