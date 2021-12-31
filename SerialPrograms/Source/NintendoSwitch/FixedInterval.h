/*  Fixed Interval Wrappers
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Wrappers to reproduce an internal interface with different timing parameters.
 *
 */

#ifndef PokemonAutomation_FixedIntervalWrappers_H
#define PokemonAutomation_FixedIntervalWrappers_H

#include "Commands/NintendoSwitch_Commands_PushButtons.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


inline void ssf_press_button2(const BotBaseContext& context, Button button, uint16_t duration, uint16_t hold){
    pbf_press_button(context, button, hold, duration < hold ? 0 : duration - hold);
}
inline void ssf_press_button1(const BotBaseContext& context, Button button, uint16_t duration){
    ssf_press_button2(context, button, duration, 5);
}
inline void ssf_press_dpad2(const BotBaseContext& context, DpadPosition dpad, uint16_t duration, uint16_t hold){
    pbf_press_dpad(context, dpad, hold, duration < hold ? 0 : duration - hold);
}
inline void ssf_press_dpad1(const BotBaseContext& context, DpadPosition dpad, uint16_t duration){
    ssf_press_dpad2(context, dpad, duration, 5);
}
inline void ssf_press_joystick2(const BotBaseContext& context, bool left, uint8_t x, uint8_t y, uint16_t duration, uint16_t hold){
    if (left){
        pbf_move_left_joystick(context, x, y, hold, duration < hold ? 0 : duration - hold);
    }else{
        pbf_move_right_joystick(context, x, y, hold, duration < hold ? 0 : duration - hold);
    }
}
inline void ssf_hold_joystick1(const BotBaseContext& context, bool left, uint8_t x, uint8_t y, uint16_t hold){
    ssf_press_joystick2(context, left, x, y, hold, hold);
}






#if 0
inline void ssf_press_button2(Button button, uint16_t duration, uint16_t hold){
    ssf_press_button2(*global_connection, button, duration, hold);
}
inline void ssf_press_button1(Button button, uint16_t duration){
    ssf_press_button1(*global_connection, button, duration);
}

inline void ssf_press_dpad2(DpadPosition dpad, uint16_t duration, uint16_t hold){
    ssf_press_dpad2(*global_connection, dpad, duration, hold);
}
inline void ssf_press_dpad1(DpadPosition dpad, uint16_t duration){
    ssf_press_dpad1(*global_connection, dpad, duration);
}

inline void ssf_press_joystick2(bool left, uint8_t x, uint8_t y, uint16_t duration, uint16_t hold){
    ssf_press_joystick2(*global_connection, left, x, y, duration, hold);
}
inline void ssf_hold_joystick1(bool left, uint8_t x, uint8_t y, uint16_t hold){
    ssf_hold_joystick1(*global_connection, left, x, y, hold);
}
#endif



}
}
#endif

