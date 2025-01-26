/*  Superscalar Framework (ssf_*)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Don't use these unless you know what you're doing and you really need to.
 *
 *  If you don't understand how the state machine works, you will not be able
 *  to properly use these functions.
 *
 *  Since the functionality here used to be internal-only, documentation remains
 *  sparse for now and slated to be expanded in the future.
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Commands_Superscalar_H
#define PokemonAutomation_NintendoSwitch_Commands_Superscalar_H

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"


namespace PokemonAutomation{
namespace NintendoSwitch{

using namespace std::chrono_literals;


void ssf_flush_pipeline (SwitchControllerContext& context);
void ssf_do_nothing     (SwitchControllerContext& context, uint16_t ticks);
void ssf_do_nothing     (SwitchControllerContext& context, Milliseconds duration);



void ssf_press_button(
    SwitchControllerContext& context,
    Button button,
    uint16_t delay, uint16_t hold = 5, uint8_t cool = 3
);
void ssf_press_button(
    SwitchControllerContext& context,
    Button button,
    Milliseconds delay, Milliseconds hold = 5*8ms, Milliseconds cool = 3*8ms
);

void ssf_press_dpad(
    SwitchControllerContext& context,
    DpadPosition position,
    uint16_t delay, uint16_t hold = 5, uint8_t cool = 3
);
void ssf_press_dpad(
    SwitchControllerContext& context,
    DpadPosition position,
    Milliseconds delay, Milliseconds hold = 5*8ms, Milliseconds cool = 3*8ms
);


void ssf_press_left_joystick(
    SwitchControllerContext& context,
    uint8_t x, uint8_t y,
    uint16_t delay, uint16_t hold, uint8_t cool = 0
);
void ssf_press_left_joystick(
    SwitchControllerContext& context,
    uint8_t x, uint8_t y,
    Milliseconds delay, Milliseconds hold = 5*8ms, Milliseconds cool = 3*8ms
);
void ssf_press_right_joystick(
    SwitchControllerContext& context,
    uint8_t x, uint8_t y,
    uint16_t delay, uint16_t hold, uint8_t cool = 0
);
void ssf_press_right_joystick(
    SwitchControllerContext& context,
    uint8_t x, uint8_t y,
    Milliseconds delay, Milliseconds hold = 5*8ms, Milliseconds cool = 3*8ms
);


void ssf_mash1_button   (SwitchControllerContext& context, Button button, uint16_t ticks);
void ssf_mash1_button   (SwitchControllerContext& context, Button button, Milliseconds duration);

void ssf_mash2_button   (SwitchControllerContext& context, Button button0, Button button1, uint16_t ticks);
void ssf_mash2_button   (SwitchControllerContext& context, Button button0, Button button1, Milliseconds duration);

void ssf_mash_AZs       (SwitchControllerContext& context, uint16_t ticks);
void ssf_mash_AZs       (SwitchControllerContext& context, Milliseconds duration);


//  Diagonal scrolling seems to count as seperate events for each direction.
//  In other words, they don't work.
#define SSF_SCROLL_UP           DPAD_UP
#define SSF_SCROLL_RIGHT        DPAD_RIGHT
#define SSF_SCROLL_DOWN         DPAD_DOWN
#define SSF_SCROLL_LEFT         DPAD_LEFT
void ssf_issue_scroll(
    SwitchControllerContext& context,
    DpadPosition direction,
    uint16_t delay, uint16_t hold = 5, uint8_t cool = 3
);
void ssf_issue_scroll(
    SwitchControllerContext& context,
    DpadPosition direction,
    Milliseconds delay, Milliseconds hold = 5*8ms, Milliseconds cool = 3*8ms
);










//
//  The C API used by the MC programs. (no overloading allowed there)
//

inline void ssf_press_button2(
    SwitchControllerContext& context,
    Button button, uint16_t duration, uint16_t hold
){
    ssf_press_button(context, button, duration, hold);
}
inline void ssf_press_button1(
    SwitchControllerContext& context,
    Button button, uint16_t duration
){
    ssf_press_button(context, button, duration);
}
inline void ssf_press_dpad2(
    SwitchControllerContext& context,
    DpadPosition dpad, uint16_t duration, uint16_t hold
){
    ssf_press_dpad(context, dpad, duration, hold);
}
inline void ssf_press_dpad1(
    SwitchControllerContext& context,
    DpadPosition dpad, uint16_t duration
){
    ssf_press_dpad(context, dpad, duration);
}
inline void ssf_press_joystick2(
    SwitchControllerContext& context,
    bool left, uint8_t x, uint8_t y,
    uint16_t duration, uint16_t hold
){
    if (left){
        ssf_press_left_joystick(context, x, y, duration, hold);
    }else{
        ssf_press_right_joystick(context, x, y, duration, hold);
    }
}
inline void ssf_hold_joystick1(
    SwitchControllerContext& context,
    bool left, uint8_t x, uint8_t y,
    uint16_t hold
){
    if (left){
        ssf_press_left_joystick(context, x, y, hold, hold);
    }else{
        ssf_press_right_joystick(context, x, y, hold, hold);
    }
}





}
}
#endif
