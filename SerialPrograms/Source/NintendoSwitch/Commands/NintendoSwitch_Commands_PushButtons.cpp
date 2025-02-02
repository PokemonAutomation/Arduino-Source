/*  Push Button Framework
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include <sstream>
#include "ClientSource/Libraries/MessageConverter.h"
#include "NintendoSwitch_Commands_Device.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch_Messages_PushButtons.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void pbf_wait(SwitchControllerContext& context, uint16_t ticks){
    ssf_do_nothing(context, ticks);
}
void pbf_wait(SwitchControllerContext& context, Milliseconds duration){
    ssf_do_nothing(context, duration);
}
void pbf_press_button(SwitchControllerContext& context, Button button, uint16_t hold_ticks, uint16_t release_ticks){
    uint32_t delay = (uint32_t)hold_ticks + release_ticks;
    if ((uint16_t)delay == delay){
        ssf_press_button(context, button, (uint16_t)delay, hold_ticks, 0);
    }else{
        ssf_press_button(context, button, hold_ticks, hold_ticks, 0);
        ssf_do_nothing(context, release_ticks);
    }
}
void pbf_press_button(SwitchControllerContext& context, Button button, Milliseconds hold, Milliseconds release){
    ssf_press_button(context, button, hold + release, hold, 0ms);
}
void pbf_press_dpad(SwitchControllerContext& context, DpadPosition position, uint16_t hold_ticks, uint16_t release_ticks){
    uint32_t delay = (uint32_t)hold_ticks + release_ticks;
    if ((uint16_t)delay == delay){
        ssf_press_dpad(context, position, (uint16_t)delay, hold_ticks, 0);
    }else{
        ssf_press_dpad(context, position, hold_ticks, hold_ticks, 0);
        ssf_do_nothing(context, release_ticks);
    }
}
void pbf_press_dpad(SwitchControllerContext& context, DpadPosition position, Milliseconds hold, Milliseconds release){
    ssf_press_dpad(context, position, hold + release, hold, 0ms);
}
void pbf_move_left_joystick(SwitchControllerContext& context, uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks){
    uint32_t delay = (uint32_t)hold_ticks + release_ticks;
    if ((uint16_t)delay == delay){
        ssf_press_left_joystick(context, x, y, (uint16_t)delay, hold_ticks, 0);
    }else{
        ssf_press_left_joystick(context, x, y, hold_ticks, hold_ticks, 0);
        ssf_do_nothing(context, release_ticks);
    }
}
void pbf_move_left_joystick (SwitchControllerContext& context, uint8_t x, uint8_t y, Milliseconds hold, Milliseconds release){
    ssf_press_left_joystick(context, x, y, hold + release, hold, 0ms);
}
void pbf_move_right_joystick(SwitchControllerContext& context, uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks){
    uint32_t delay = (uint32_t)hold_ticks + release_ticks;
    if ((uint16_t)delay == delay){
        ssf_press_right_joystick(context, x, y, (uint16_t)delay, hold_ticks, 0);
    }else{
        ssf_press_right_joystick(context, x, y, hold_ticks, hold_ticks, 0);
        ssf_do_nothing(context, release_ticks);
    }
}
void pbf_move_right_joystick (SwitchControllerContext& context, uint8_t x, uint8_t y, Milliseconds hold, Milliseconds release){
    ssf_press_right_joystick(context, x, y, hold + release, hold, 0ms);
}
void pbf_mash_button(SwitchControllerContext& context, Button button, uint16_t ticks){
    ssf_mash1_button(context, button, ticks);
}
void pbf_mash_button(SwitchControllerContext& context, Button button, Milliseconds duration){
    ssf_mash1_button(context, button, duration);
}

void grip_menu_connect_go_home(SwitchControllerContext& context){
    pbf_press_button(context, BUTTON_L | BUTTON_R, 10, 40);
    pbf_press_button(context, BUTTON_A, 10, 140);
    pbf_press_button(context, BUTTON_HOME, 80ms, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
}


void pbf_controller_state(
    SwitchControllerContext& context,
    Button button,
    DpadPosition position,
    uint8_t left_x, uint8_t left_y,
    uint8_t right_x, uint8_t right_y,
    uint16_t ticks
){
    context->issue_controller_state(
        &context,
        button, position,
        left_x, left_y,
        right_x, right_y,
        ticks*8ms
    );
}
void pbf_controller_state(
    SwitchControllerContext& context,
    Button button,
    DpadPosition position,
    uint8_t left_x, uint8_t left_y,
    uint8_t right_x, uint8_t right_y,
    Milliseconds duration
){
    context->issue_controller_state(
        &context,
        button, position,
        left_x, left_y,
        right_x, right_y,
        duration
    );
}

#if 0
static std::string button_to_string(Button button){
    std::string str;
    if (button & BUTTON_Y) str += " BUTTON_Y ";
    if (button & BUTTON_B) str += " BUTTON_B ";
    if (button & BUTTON_A) str += " BUTTON_A ";
    if (button & BUTTON_X) str += " BUTTON_X ";
    if (button & BUTTON_L) str += " BUTTON_L ";
    if (button & BUTTON_R) str += " BUTTON_R ";
    if (button & BUTTON_ZL) str += " BUTTON_ZL ";
    if (button & BUTTON_ZR) str += " BUTTON_ZR ";
    if (button & BUTTON_MINUS) str += " BUTTON_MINUS ";
    if (button & BUTTON_PLUS) str += " BUTTON_PLUS ";
    if (button & BUTTON_LCLICK) str += " BUTTON_LCLICK ";
    if (button & BUTTON_RCLICK) str += " BUTTON_RCLICK ";
    if (button & BUTTON_HOME) str += " BUTTON_HOME ";
    if (button & BUTTON_CAPTURE) str += " BUTTON_CAPTURE ";
    return str;
}
static std::string dpad_to_string(Button dpad){
    switch (dpad){
    case DPAD_UP            : return "DPAD_UP";
    case DPAD_UP_RIGHT      : return "DPAD_UP_RIGHT";
    case DPAD_RIGHT         : return "DPAD_RIGHT";
    case DPAD_DOWN_RIGHT    : return "DPAD_DOWN_RIGHT";
    case DPAD_DOWN          : return "DPAD_DOWN";
    case DPAD_DOWN_LEFT     : return "DPAD_DOWN_LEFT";
    case DPAD_LEFT          : return "DPAD_LEFT";
    case DPAD_UP_LEFT       : return "DPAD_UP_LEFT";
    case DPAD_NONE          : return "DPAD_NONE";
    }
    return "UNKNOWN_DPAD";
}
#endif

int register_message_converters_push_button_framework(){
    register_message_converter(
        PABB_MSG_CONTROLLER_STATE,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "controller_state() - ";
            if (body.size() != sizeof(pabb_controller_state)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_controller_state*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", button = " << params->button << "(" << button_to_string(params->button) << ")";
            ss << ", dpad = " << dpad_to_string(params->dpad);
            ss << ", LJ = (" << (int)params->left_joystick_x << "," << (int)params->left_joystick_y << ")";
            ss << ", RJ = (" << (int)params->right_joystick_x << "," << (int)params->right_joystick_y << ")";
            ss << ", ticks = " << (int)params->ticks;
            return ss.str();
        }
    );
    return 0;
}
int init_PushButtonFramework = register_message_converters_push_button_framework();






}
}


