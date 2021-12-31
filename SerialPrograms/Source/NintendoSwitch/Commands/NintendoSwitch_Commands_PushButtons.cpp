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
#include "NintendoSwitch_Messages_PushButtons.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void pbf_wait(const BotBaseContext& context, uint16_t ticks){
    context.issue_request(
        DeviceRequest_pbf_wait(ticks)
    );
}
void pbf_press_button(const BotBaseContext& context, Button button, uint16_t hold_ticks, uint16_t release_ticks){
    context.issue_request(
        DeviceRequest_pbf_press_button(button, hold_ticks, release_ticks)
    );
}
void pbf_press_dpad(const BotBaseContext& context, DpadPosition position, uint16_t hold_ticks, uint16_t release_ticks){
    context.issue_request(
        DeviceRequest_pbf_press_dpad(position, hold_ticks, release_ticks)
    );
}
void pbf_move_left_joystick(const BotBaseContext& context, uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks){
    context.issue_request(
        DeviceRequest_pbf_move_left_joystick(x, y, hold_ticks, release_ticks)
    );
}
void pbf_move_right_joystick(const BotBaseContext& context, uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks){
    context.issue_request(
        DeviceRequest_pbf_move_right_joystick(x, y, hold_ticks, release_ticks)
    );
}
void pbf_mash_button(const BotBaseContext& context, Button button, uint16_t ticks){
    context.issue_request(
        DeviceRequest_pbf_mash_button(button, ticks)
    );
}

void start_program_flash(const BotBaseContext& context, uint16_t ticks){
    for (uint16_t c = 0; c < ticks; c += 50){
        set_leds(context, true);
        pbf_wait(context, 25);
        set_leds(context, false);
        pbf_wait(context, 25);
    }
}
void grip_menu_connect_go_home(const BotBaseContext& context){
    pbf_press_button(context, BUTTON_L | BUTTON_R, 10, 40);
    pbf_press_button(context, BUTTON_A, 10, 140);
    pbf_press_button(context, BUTTON_HOME, 10, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY);
}

void pbf_controller_state(
    const BotBaseContext& context,
    Button button,
    DpadPosition position,
    uint8_t left_x, uint8_t left_y,
    uint8_t right_x, uint8_t right_y,
    uint8_t ticks
){
    context.issue_request(
        DeviceRequest_controller_state(button, position, left_x, left_y, right_x, right_y, ticks)
    );
}


int register_message_converters_push_button_framework(){
    register_message_converter(
        PABB_MSG_COMMAND_PBF_WAIT,
        [](const std::string& body){
            std::stringstream ss;
            ss << "pbf_wait() - ";
            if (body.size() != sizeof(pabb_pbf_wait)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_pbf_wait*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", ticks = " << params->ticks;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_PBF_PRESS_BUTTON,
        [](const std::string& body){
            std::stringstream ss;
            ss << "pbf_press_button() - ";
            if (body.size() != sizeof(pabb_pbf_press_button)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_pbf_press_button*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", button = " << params->button;
            ss << ", hold_ticks = " << params->hold_ticks;
            ss << ", release_ticks = " << params->release_ticks;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_PBF_PRESS_DPAD,
        [](const std::string& body){
            std::stringstream ss;
            ss << "pbf_press_dpad() - ";
            if (body.size() != sizeof(pabb_pbf_press_dpad)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_pbf_press_dpad*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", dpad = " << (unsigned)params->dpad;
            ss << ", hold_ticks = " << params->hold_ticks;
            ss << ", release_ticks = " << params->release_ticks;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_PBF_MOVE_JOYSTICK_L,
        [](const std::string& body){
            std::stringstream ss;
            ss << "pbf_move_left_joystick() - ";
            if (body.size() != sizeof(pabb_pbf_move_joystick)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_pbf_move_joystick*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", x = " << (unsigned)params->x;
            ss << ", y = " << (unsigned)params->y;
            ss << ", hold_ticks = " << params->hold_ticks;
            ss << ", release_ticks = " << params->release_ticks;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_PBF_MOVE_JOYSTICK_R,
        [](const std::string& body){
            std::stringstream ss;
            ss << "pbf_move_right_joystick() - ";
            if (body.size() != sizeof(pabb_pbf_move_joystick)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_pbf_move_joystick*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", x = " << (unsigned)params->x;
            ss << ", y = " << (unsigned)params->y;
            ss << ", hold_ticks = " << params->hold_ticks;
            ss << ", release_ticks = " << params->release_ticks;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_MASH_BUTTON,
        [](const std::string& body){
            std::stringstream ss;
            ss << "pbf_mash_button() - ";
            if (body.size() != sizeof(pabb_pbf_mash_button)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_pbf_mash_button*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", button = " << params->button;
            ss << ", ticks = " << params->ticks;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_CONTROLLER_STATE,
        [](const std::string& body){
            std::stringstream ss;
            ss << "controller_state() - ";
            if (body.size() != sizeof(pabb_controller_state)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_controller_state*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", button = " << params->button;
            ss << ", dpad = " << (int)params->dpad;
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


