/*  Push Button Framework
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#include "ClientSource/Libraries/MessageConverter.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch_Commands_Superscalar.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void pbf_wait(ProControllerContext& context, uint16_t ticks){
    ssf_do_nothing(context, ticks);
}
void pbf_wait(ProControllerContext& context, Milliseconds duration){
    ssf_do_nothing(context, duration);
}
void pbf_press_button(ProControllerContext& context, Button button, uint16_t hold_ticks, uint16_t release_ticks){
    ssf_press_button(context, button, (hold_ticks + release_ticks) * 8ms, hold_ticks * 8ms, 0ms);
}
void pbf_press_button(ProControllerContext& context, Button button, Milliseconds hold, Milliseconds release){
    ssf_press_button(context, button, hold + release, hold, 0ms);
}
void pbf_press_dpad(ProControllerContext& context, DpadPosition position, uint16_t hold_ticks, uint16_t release_ticks){
    ssf_press_dpad(context, position, (hold_ticks + release_ticks) * 8ms, hold_ticks * 8ms, 0ms);
}
void pbf_press_dpad(ProControllerContext& context, DpadPosition position, Milliseconds hold, Milliseconds release){
    ssf_press_dpad(context, position, hold + release, hold, 0ms);
}
void pbf_move_left_joystick(ProControllerContext& context, uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks){
    uint32_t delay = (uint32_t)hold_ticks + release_ticks;
    if ((uint16_t)delay == delay){
        ssf_press_left_joystick(context, x, y, (uint16_t)delay, hold_ticks, 0);
    }else{
        ssf_press_left_joystick(context, x, y, hold_ticks, hold_ticks, 0);
        ssf_do_nothing(context, release_ticks);
    }
}
void pbf_move_left_joystick (ProControllerContext& context, uint8_t x, uint8_t y, Milliseconds hold, Milliseconds release){
    ssf_press_left_joystick(context, x, y, hold + release, hold, 0ms);
}
void pbf_move_right_joystick(ProControllerContext& context, uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks){
    uint32_t delay = (uint32_t)hold_ticks + release_ticks;
    if ((uint16_t)delay == delay){
        ssf_press_right_joystick(context, x, y, (uint16_t)delay, hold_ticks, 0);
    }else{
        ssf_press_right_joystick(context, x, y, hold_ticks, hold_ticks, 0);
        ssf_do_nothing(context, release_ticks);
    }
}
void pbf_move_right_joystick (ProControllerContext& context, uint8_t x, uint8_t y, Milliseconds hold, Milliseconds release){
    ssf_press_right_joystick(context, x, y, hold + release, hold, 0ms);
}
void pbf_mash_button(ProControllerContext& context, Button button, uint16_t ticks){
    ssf_mash1_button(context, button, ticks);
}
void pbf_mash_button(ProControllerContext& context, Button button, Milliseconds duration){
    ssf_mash1_button(context, button, duration);
}

void grip_menu_connect_go_home(ProControllerContext& context){
    pbf_press_button(context, BUTTON_L | BUTTON_R, 10, 40);
    pbf_press_button(context, BUTTON_A, 10, 140);
    pbf_press_button(context, BUTTON_HOME, 80ms, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
}


void pbf_controller_state(
    ProControllerContext& context,
    Button button,
    DpadPosition position,
    uint8_t left_x, uint8_t left_y,
    uint8_t right_x, uint8_t right_y,
    uint16_t ticks
){
    context->issue_full_controller_state(
        &context,
        ticks*8ms,
        button, position,
        left_x, left_y,
        right_x, right_y
    );
}
void pbf_controller_state(
    ProControllerContext& context,
    Button button,
    DpadPosition position,
    uint8_t left_x, uint8_t left_y,
    uint8_t right_x, uint8_t right_y,
    Milliseconds duration
){
    context->issue_full_controller_state(
        &context,
        duration,
        button, position,
        left_x, left_y,
        right_x, right_y
    );
}




void pbf_wait(JoyconContext& context, Milliseconds duration){
    ssf_do_nothing(context, duration);
}
void pbf_press_button(JoyconContext& context, Button button, Milliseconds hold, Milliseconds release){
    ssf_press_button(context, button, hold + release, hold, 0ms);
}
void pbf_move_joystick(JoyconContext& context, uint8_t x, uint8_t y, Milliseconds hold, Milliseconds release){
    ssf_press_joystick(context, x, y, hold + release, hold, 0ms);
}
void pbf_mash_button(JoyconContext& context, Button button, Milliseconds duration){
    ssf_mash1_button(context, button, duration);
}

void pbf_controller_state(
    JoyconContext& context,
    Button button,
    uint8_t x, uint8_t y,
    Milliseconds duration
){
    context->issue_full_controller_state(
        &context,
        button,
        x, y,
        duration
    );
}



























}
}


