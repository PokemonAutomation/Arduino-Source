/*  Fixed Interval Wrappers
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Wrappers to reproduce an internal interface with different timing parameters.
 *
 */

#ifndef PokemonAutomation_FixedIntervalWrappers_H
#define PokemonAutomation_FixedIntervalWrappers_H

#include "Common/SwitchFramework/Switch_PushButtons.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


inline void ssf_press_button2(BotBase& device, Button button, uint16_t duration, uint16_t hold){
    pbf_press_button(device, button, hold, duration < hold ? 0 : duration - hold);
}
inline void ssf_press_button1(BotBase& device, Button button, uint16_t duration){
    ssf_press_button2(device, button, duration, 5);
}
inline void ssf_press_dpad2(BotBase& device, DpadPosition dpad, uint16_t duration, uint16_t hold){
    pbf_press_dpad(device, dpad, hold, duration < hold ? 0 : duration - hold);
}
inline void ssf_press_dpad1(BotBase& device, DpadPosition dpad, uint16_t duration){
    ssf_press_dpad2(device, dpad, duration, 5);
}
inline void ssf_press_joystick2(BotBase& device, bool left, uint8_t x, uint8_t y, uint16_t duration, uint16_t hold){
    if (left){
        pbf_move_left_joystick(device, x, y, hold, duration < hold ? 0 : duration - hold);
    }else{
        pbf_move_right_joystick(device, x, y, hold, duration < hold ? 0 : duration - hold);
    }
}
inline void ssf_hold_joystick1(BotBase& device, bool left, uint8_t x, uint8_t y, uint16_t hold){
    ssf_press_joystick2(device, left, x, y, hold, hold);
}




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



}
}
#endif

