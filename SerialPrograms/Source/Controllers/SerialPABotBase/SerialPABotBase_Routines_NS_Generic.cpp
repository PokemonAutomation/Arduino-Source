/*  SerialPABotBase AVR8 Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <sstream>
#include "Common/SerialPABotBase/SerialPABotBase_Messages_NS_Generic.h"
#include "ClientSource/Libraries/MessageConverter.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ControllerState.h"

namespace PokemonAutomation{
namespace SerialPABotBase{



int register_message_converters_push_button_framework(){
    register_message_converter(
        PABB_MSG_NS_GENERIC_CONTROLLER_STATE_TICKS,
        [](const std::string& body){
            //  Disable this by default since it's very spammy.
            if (!GlobalSettings::instance().LOG_EVERYTHING){
                return std::string();
            }
            std::ostringstream ss;
            ss << "controller_state_ticks() - ";
            if (body.size() != sizeof(pabb_Message_NS_Generic_ControllerStateTicks)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_Message_NS_Generic_ControllerStateTicks*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", buttons = " << params->buttons << " (" << button_to_string((NintendoSwitch::Button)params->buttons) << ")";
            ss << ", dpad = " << dpad_to_string((NintendoSwitch::DpadPosition)params->dpad);
            ss << ", LJ = (" << (int)params->left_joystick_x << "," << (int)params->left_joystick_y << ")";
            ss << ", RJ = (" << (int)params->right_joystick_x << "," << (int)params->right_joystick_y << ")";
            ss << ", ticks = " << (int)params->ticks;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_NS_GENERIC_CONTROLLER_STATE_MS,
        [](const std::string& body){
            //  Disable this by default since it's very spammy.
            if (!GlobalSettings::instance().LOG_EVERYTHING){
                return std::string();
            }
            std::ostringstream ss;
            ss << "controller_state_ms() - ";
            if (body.size() != sizeof(pabb_Message_NS_Generic_ControllerStateMs)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_Message_NS_Generic_ControllerStateMs*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", buttons = " << params->buttons << " (" << button_to_string((NintendoSwitch::Button)params->buttons) << ")";
            ss << ", dpad = " << dpad_to_string((NintendoSwitch::DpadPosition)params->dpad);
            ss << ", LJ = (" << (int)params->left_joystick_x << "," << (int)params->left_joystick_y << ")";
            ss << ", RJ = (" << (int)params->right_joystick_x << "," << (int)params->right_joystick_y << ")";
            ss << ", milliseconds = " << params->milliseconds;
            return ss.str();
        }
    );
    return 0;
}
int init_PushButtonFramework = register_message_converters_push_button_framework();



}
}
