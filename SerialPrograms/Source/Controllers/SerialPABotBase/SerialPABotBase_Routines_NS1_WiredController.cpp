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
        PABB_MSG_REPORT_NS1_WIRED_CONTROLLER,
        [](const std::string& body){
            //  Disable this by default since it's very spammy.
            if (!GlobalSettings::instance().LOG_EVERYTHING){
                return std::string();
            }
            std::ostringstream ss;
            ss << "PABB_MSG_REPORT_NS1_WIRED_CONTROLLER: ";
            if (body.size() != sizeof(pabb_Message_Report_NS1_WiredController)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_Message_Report_NS1_WiredController*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;

            uint16_t buttons = params->report.buttons0;
            buttons |= (uint16_t)params->report.buttons1 << 8;
            ss << ", buttons = " << buttons << " (" << button_to_string((NintendoSwitch::Button)buttons) << ")";

            ss << ", dpad = " << dpad_to_string((NintendoSwitch::DpadPosition)params->report.dpad);
            ss << ", LJ = (" << (int)params->report.left_joystick_x << "," << (int)params->report.left_joystick_y << ")";
            ss << ", RJ = (" << (int)params->report.right_joystick_x << "," << (int)params->report.right_joystick_y << ")";
            ss << ", milliseconds = " << params->milliseconds;
            return ss.str();
        }
    );
    return 0;
}
int init_PushButtonFramework = register_message_converters_push_button_framework();



}
}
