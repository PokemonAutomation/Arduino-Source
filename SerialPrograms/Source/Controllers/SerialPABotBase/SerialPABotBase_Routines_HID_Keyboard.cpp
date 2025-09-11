/*  SerialPABotBase (HID Keyboard)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <sstream>
#include "Common/SerialPABotBase/SerialPABotBase_Messages_HID_Keyboard.h"
#include "ClientSource/Libraries/MessageConverter.h"
#include "CommonFramework/GlobalSettingsPanel.h"

namespace PokemonAutomation{
namespace SerialPABotBase{



int register_message_converters_HID_Keyboard(){
    register_message_converter(
        PABB_MSG_COMMAND_HID_KEYBOARD_STATE,
        [](const std::string& body){
            //  Disable this by default since it's very spammy.
            if (!GlobalSettings::instance().LOG_EVERYTHING){
                return std::string();
            }
            std::ostringstream ss;
            ss << "PABB_MSG_COMMAND_HID_KEYBOARD_STATE: ";
            if (body.size() != sizeof(pabb_Message_Command_HID_Keyboard_State)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_Message_Command_HID_Keyboard_State*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;

            ss << std::hex;
            ss << ", 0x" << params->report.key0;
            ss << ", 0x" << params->report.key1;
            ss << ", 0x" << params->report.key2;
            ss << ", 0x" << params->report.key3;
            ss << ", 0x" << params->report.key4;
            ss << ", 0x" << params->report.key5;

            ss << std::dec;
            ss << ", milliseconds = " << params->milliseconds;
            return ss.str();
        }
    );
    return 0;
}
int init_PushButtonFramework = register_message_converters_HID_Keyboard();



}
}
