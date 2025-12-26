/*  Push Button Framework
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#include "Controllers/JoystickTools.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch_Commands_Superscalar.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void pbf_wait(ProControllerContext& context, Milliseconds duration){
    ssf_do_nothing(context, duration);
}
void pbf_press_button(ProControllerContext& context, Button button, Milliseconds hold, Milliseconds release){
    ssf_press_button(context, button, hold + release, hold, 0ms);
}
void pbf_press_dpad_old(ProControllerContext& context, DpadPosition position, uint16_t hold_ticks, uint16_t release_ticks){
    ssf_press_dpad(context, position, (hold_ticks + release_ticks) * 8ms, hold_ticks * 8ms, 0ms);
}
void pbf_press_dpad(ProControllerContext& context, DpadPosition position, Milliseconds hold, Milliseconds release){
    ssf_press_dpad(context, position, hold + release, hold, 0ms);
}
void pbf_move_left_joystick_old(ProControllerContext& context, uint8_t x, uint8_t y, Milliseconds hold, Milliseconds release){
    ssf_press_left_joystick_old(context, x, y, hold + release, hold, 0ms);
}
void pbf_move_left_joystick(ProControllerContext& context, const JoystickPosition& position, Milliseconds hold, Milliseconds release){
    ssf_press_left_joystick(context, position, hold + release, hold, 0ms);
}
void pbf_move_right_joystick_old(ProControllerContext& context, uint8_t x, uint8_t y, Milliseconds hold, Milliseconds release){
    ssf_press_right_joystick_old(context, x, y, hold + release, hold, 0ms);
}
void pbf_move_right_joystick(ProControllerContext& context, const JoystickPosition& position, Milliseconds hold, Milliseconds release){
    ssf_press_right_joystick(context, position, hold + release, hold, 0ms);
}
void pbf_mash_button(ProControllerContext& context, Button button, Milliseconds duration){
    ssf_mash1_button(context, button, duration);
}

void grip_menu_connect_go_home(ProControllerContext& context){
    pbf_press_button(context, BUTTON_L | BUTTON_R, 80ms, 320ms);
    pbf_press_button(context, BUTTON_A, 80ms, 1120ms);
    pbf_press_button(context, BUTTON_HOME, 80ms, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
}


void pbf_controller_state(
    ProControllerContext& context,
    Button button,
    DpadPosition position,
    const JoystickPosition& left_joystick,
    const JoystickPosition& right_joystick,
    Milliseconds duration
){
    context->issue_full_controller_state(
        &context,
        true,
        duration,
        button, position,
        left_joystick,
        right_joystick
    );
}




void pbf_wait(JoyconContext& context, Milliseconds duration){
    ssf_do_nothing(context, duration);
}
void pbf_press_button(JoyconContext& context, Button button, Milliseconds hold, Milliseconds release){
    ssf_press_button(context, button, hold + release, hold, 0ms);
}
void pbf_move_joystick(JoyconContext& context, const JoystickPosition& position, Milliseconds hold, Milliseconds release){
    ssf_press_joystick(context, position, hold + release, hold, 0ms);
}
void pbf_mash_button(JoyconContext& context, Button button, Milliseconds duration){
    ssf_mash1_button(context, button, duration);
}

void pbf_controller_state(
    JoyconContext& context,
    Button button,
    const JoystickPosition& joystick,
    Milliseconds duration
){
    context->issue_full_controller_state(
        &context,
        true,
        duration,
        button,
        joystick
    );
}


























}
}


