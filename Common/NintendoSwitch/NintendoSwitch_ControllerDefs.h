/*  Controller Types and Definitions
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#ifndef PokemonAutomation_NintendoSwitch_ControllerDefs_H
#define PokemonAutomation_NintendoSwitch_ControllerDefs_H

#include <stdbool.h>
#include <stdint.h>


#ifdef __cplusplus
namespace PokemonAutomation{
namespace NintendoSwitch{

//  One second = 125 ticks. Thus each tick is 8 milliseconds.
constexpr uint16_t TICKS_PER_SECOND = 125;

//  Buttons
enum Button : uint16_t{
    BUTTON_NONE     =   0,
    BUTTON_Y        =   ((uint16_t)1 <<  0),
    BUTTON_B        =   ((uint16_t)1 <<  1),
    BUTTON_A        =   ((uint16_t)1 <<  2),
    BUTTON_X        =   ((uint16_t)1 <<  3),
    BUTTON_L        =   ((uint16_t)1 <<  4),
    BUTTON_R        =   ((uint16_t)1 <<  5),
    BUTTON_ZL       =   ((uint16_t)1 <<  6),
    BUTTON_ZR       =   ((uint16_t)1 <<  7),
    BUTTON_MINUS    =   ((uint16_t)1 <<  8),
    BUTTON_PLUS     =   ((uint16_t)1 <<  9),
    BUTTON_LCLICK   =   ((uint16_t)1 << 10),
    BUTTON_RCLICK   =   ((uint16_t)1 << 11),
    BUTTON_HOME     =   ((uint16_t)1 << 12),
    BUTTON_CAPTURE  =   ((uint16_t)1 << 13),
};
inline Button operator|(Button x, Button y){
    return (Button)((uint16_t)x | (uint16_t)y);
}
inline void operator|=(Button& x, Button y){
    x = (Button)((uint16_t)x | (uint16_t)y);
}
#if 0
using Button = uint16_t;
constexpr Button BUTTON_Y       =   ((uint16_t)1 <<  0);
constexpr Button BUTTON_B       =   ((uint16_t)1 <<  1);
constexpr Button BUTTON_A       =   ((uint16_t)1 <<  2);
constexpr Button BUTTON_X       =   ((uint16_t)1 <<  3);
constexpr Button BUTTON_L       =   ((uint16_t)1 <<  4);
constexpr Button BUTTON_R       =   ((uint16_t)1 <<  5);
constexpr Button BUTTON_ZL      =   ((uint16_t)1 <<  6);
constexpr Button BUTTON_ZR      =   ((uint16_t)1 <<  7);
constexpr Button BUTTON_MINUS   =   ((uint16_t)1 <<  8);
constexpr Button BUTTON_PLUS    =   ((uint16_t)1 <<  9);
constexpr Button BUTTON_LCLICK  =   ((uint16_t)1 << 10);
constexpr Button BUTTON_RCLICK  =   ((uint16_t)1 << 11);
constexpr Button BUTTON_HOME    =   ((uint16_t)1 << 12);
constexpr Button BUTTON_CAPTURE =   ((uint16_t)1 << 13);
#endif

//  Dpad
enum DpadPosition : uint8_t{
    DPAD_UP          =   0,
    DPAD_UP_RIGHT    =   1,
    DPAD_RIGHT       =   2,
    DPAD_DOWN_RIGHT  =   3,
    DPAD_DOWN        =   4,
    DPAD_DOWN_LEFT   =   5,
    DPAD_LEFT        =   6,
    DPAD_UP_LEFT     =   7,
    DPAD_NONE        =   8,
};

#if 0
using DpadPosition = uint8_t;
constexpr DpadPosition DPAD_UP          =   0;
constexpr DpadPosition DPAD_UP_RIGHT    =   1;
constexpr DpadPosition DPAD_RIGHT       =   2;
constexpr DpadPosition DPAD_DOWN_RIGHT  =   3;
constexpr DpadPosition DPAD_DOWN        =   4;
constexpr DpadPosition DPAD_DOWN_LEFT   =   5;
constexpr DpadPosition DPAD_LEFT        =   6;
constexpr DpadPosition DPAD_UP_LEFT     =   7;
constexpr DpadPosition DPAD_NONE        =   8;
#endif

//  Joysticks
constexpr uint8_t STICK_MIN     =   0x00;
constexpr uint8_t STICK_CENTER  =   0x80;
constexpr uint8_t STICK_MAX     =   0xff;


}
}

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
