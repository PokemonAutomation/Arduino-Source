/*  Superscalar Framework (ssf_*)
 *
 *  From: https://github.com/PokemonAutomation/
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

#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Joycon.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

using namespace std::chrono_literals;


void ssf_flush_pipeline (ProControllerContext& context);
void ssf_do_nothing     (ProControllerContext& context, uint16_t ticks);
void ssf_do_nothing     (ProControllerContext& context, Milliseconds duration);


void ssf_press_button(
    ProControllerContext& context,
    Button button,
    Milliseconds delay = 24ms, Milliseconds hold = 48ms, Milliseconds cool = 24ms
);
void ssf_press_dpad(
    ProControllerContext& context,
    DpadPosition position,
    Milliseconds delay, Milliseconds hold = 48ms, Milliseconds cool = 24ms
);


void ssf_press_left_joystick(
    ProControllerContext& context,
    uint8_t x, uint8_t y,
    uint16_t delay, uint16_t hold, uint8_t cool = 0
);
void ssf_press_left_joystick(
    ProControllerContext& context,
    uint8_t x, uint8_t y,
    Milliseconds delay, Milliseconds hold, Milliseconds cool = 0ms
);
void ssf_press_right_joystick(
    ProControllerContext& context,
    uint8_t x, uint8_t y,
    uint16_t delay, uint16_t hold, uint8_t cool = 0
);
void ssf_press_right_joystick(
    ProControllerContext& context,
    uint8_t x, uint8_t y,
    Milliseconds delay, Milliseconds hold, Milliseconds cool = 0ms
);


void ssf_mash1_button   (ProControllerContext& context, Button button, uint16_t ticks);
void ssf_mash1_button   (ProControllerContext& context, Button button, Milliseconds duration);

void ssf_mash2_button   (ProControllerContext& context, Button button0, Button button1, uint16_t ticks);
void ssf_mash2_button   (ProControllerContext& context, Button button0, Button button1, Milliseconds duration);

void ssf_mash_AZs       (ProControllerContext& context, uint16_t ticks);
void ssf_mash_AZs       (ProControllerContext& context, Milliseconds duration);


//  Diagonal scrolling seems to count as seperate events for each direction.
//  In other words, they don't work.
#define SSF_SCROLL_UP           DPAD_UP
#define SSF_SCROLL_RIGHT        DPAD_RIGHT
#define SSF_SCROLL_DOWN         DPAD_DOWN
#define SSF_SCROLL_LEFT         DPAD_LEFT
void ssf_issue_scroll(
    ProControllerContext& context,
    DpadPosition direction,
    Milliseconds delay
);
void ssf_issue_scroll(
    ProControllerContext& context,
    DpadPosition direction,
    Milliseconds delay, Milliseconds hold, Milliseconds cool
);





//
//  ptv = plus timing variation
//
//  These are simple wrappers that add "context->timing_variation()" to
//  every timing. Mostly intended for Switch menu navigation where we usually
//  attempt fast movements.
//
inline void ssf_press_button_ptv(
    ProControllerContext& context,
    Button button,
    Milliseconds delay = 24ms
){
    Milliseconds tv = context->timing_variation();
    if (delay > 0ms) delay += tv;
    ssf_press_button(context, button, delay);
}
inline void ssf_press_button_ptv(
    ProControllerContext& context,
    Button button,
    Milliseconds delay,
    Milliseconds hold,
    Milliseconds cool = 24ms
){
    Milliseconds tv = context->timing_variation();
    if (delay > 0ms) delay += tv;
    if (hold > 0ms) hold += tv;
    if (cool > 0ms) cool += tv;
    ssf_press_button(context, button, delay, hold, cool);
}
inline void ssf_press_dpad_ptv(
    ProControllerContext& context,
    DpadPosition position,
    Milliseconds delay = 24ms
){
    Milliseconds tv = context->timing_variation();
    if (delay > 0ms) delay += tv;
    ssf_press_dpad(context, position, delay);
}
inline void ssf_press_dpad_ptv(
    ProControllerContext& context,
    DpadPosition position,
    Milliseconds delay,
    Milliseconds hold,
    Milliseconds cool = 24ms
){
    Milliseconds tv = context->timing_variation();
    if (delay > 0ms) delay += tv;
    if (hold > 0ms) hold += tv;
    if (cool > 0ms) cool += tv;
    ssf_press_dpad(context, position, delay, hold, cool);
}
inline void ssf_issue_scroll_ptv(
    ProControllerContext& context,
    DpadPosition direction,
    Milliseconds delay = 24ms
){
    Milliseconds tv = context->timing_variation();
    if (delay > 0ms) delay += tv;
    ssf_issue_scroll(context, direction, delay);
}
inline void ssf_issue_scroll_ptv(
    ProControllerContext& context,
    DpadPosition direction,
    Milliseconds delay,
    Milliseconds hold,
    Milliseconds cool = 24ms
){
    Milliseconds tv = context->timing_variation();
    if (delay > 0ms) delay += tv;
    if (hold > 0ms) hold += tv;
    ssf_issue_scroll(context, direction, delay, hold, cool);
}






void ssf_flush_pipeline (JoyconContext& context);
void ssf_do_nothing     (JoyconContext& context, Milliseconds duration);
void ssf_press_button(
    JoyconContext& context,
    Button button,
    Milliseconds delay, Milliseconds hold = 48ms, Milliseconds cool = 24ms
);
void ssf_press_joystick(
    JoyconContext& context,
    uint8_t x, uint8_t y,
    Milliseconds delay, Milliseconds hold, Milliseconds cool = 0ms
);
void ssf_mash1_button   (JoyconContext& context, Button button, Milliseconds duration);
void ssf_issue_scroll(
    JoyconContext& context,
    DpadPosition direction,
    Milliseconds delay
);
void ssf_issue_scroll(
    JoyconContext& context,
    DpadPosition direction,
    Milliseconds delay, Milliseconds hold, Milliseconds cool
);






}
}
#endif
