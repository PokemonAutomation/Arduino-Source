/*  Nintendo Switch Controller State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_ControllerState_H
#define PokemonAutomation_NintendoSwitch_ControllerState_H

#include <stdint.h>
#include <string>

namespace PokemonAutomation{
namespace NintendoSwitch{


//  Legacy tick type. This comes from the refresh rate of the wired controller.
//  One second = 125 ticks. Thus each tick is 8 milliseconds.
constexpr uint16_t TICKS_PER_SECOND = 125;


//  Buttons
constexpr size_t TOTAL_BUTTONS = 22;
using ButtonFlagType = uint32_t;
enum Button : ButtonFlagType{
    BUTTON_NONE     =   0,
    BUTTON_Y        =   ((uint32_t)1 <<  0),
    BUTTON_B        =   ((uint32_t)1 <<  1),
    BUTTON_A        =   ((uint32_t)1 <<  2),
    BUTTON_X        =   ((uint32_t)1 <<  3),
    BUTTON_L        =   ((uint32_t)1 <<  4),
    BUTTON_R        =   ((uint32_t)1 <<  5),
    BUTTON_ZL       =   ((uint32_t)1 <<  6),
    BUTTON_ZR       =   ((uint32_t)1 <<  7),
    BUTTON_MINUS    =   ((uint32_t)1 <<  8),
    BUTTON_PLUS     =   ((uint32_t)1 <<  9),
    BUTTON_LCLICK   =   ((uint32_t)1 << 10),
    BUTTON_RCLICK   =   ((uint32_t)1 << 11),
    BUTTON_HOME     =   ((uint32_t)1 << 12),
    BUTTON_CAPTURE  =   ((uint32_t)1 << 13),
    BUTTON_UP       =   ((uint32_t)1 << 14),
    BUTTON_RIGHT    =   ((uint32_t)1 << 15),
    BUTTON_DOWN     =   ((uint32_t)1 << 16),
    BUTTON_LEFT     =   ((uint32_t)1 << 17),
    BUTTON_LEFT_SL  =   ((uint32_t)1 << 18),
    BUTTON_LEFT_SR  =   ((uint32_t)1 << 19),
    BUTTON_RIGHT_SL =   ((uint32_t)1 << 20),
    BUTTON_RIGHT_SR =   ((uint32_t)1 << 21),
};
inline constexpr Button operator|(Button x, Button y){
    return (Button)((ButtonFlagType)x | (ButtonFlagType)y);
}
inline constexpr void operator|=(Button& x, Button y){
    x = (Button)((ButtonFlagType)x | (ButtonFlagType)y);
}
inline constexpr Button operator&(Button x, Button y){
    return (Button)((ButtonFlagType)x & (ButtonFlagType)y);
}
inline constexpr void operator&=(Button& x, Button y){
    x = (Button)((ButtonFlagType)x & (ButtonFlagType)y);
}

std::string button_to_string(Button button);




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

std::string dpad_to_string(DpadPosition dpad);



//  Joysticks
constexpr uint8_t STICK_MIN     =   0x00;
constexpr uint8_t STICK_CENTER  =   0x80;
constexpr uint8_t STICK_MAX     =   0xff;



}
}
#endif
