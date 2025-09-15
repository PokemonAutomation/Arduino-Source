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
            ss << ", milliseconds = " << params->milliseconds;

            //  Do not log the contents of the command due to privacy concerns.
            //  (people entering passwords)
#if 0
            ss << std::hex;
            ss << ", 0x" << (int)params->report.key[0];
            ss << ", 0x" << (int)params->report.key[1];
            ss << ", 0x" << (int)params->report.key[2];
            ss << ", 0x" << (int)params->report.key[3];
            ss << ", 0x" << (int)params->report.key[4];
            ss << ", 0x" << (int)params->report.key[5];
            ss << std::dec;
#endif

            return ss.str();
        }
    );
    return 0;
}
int init_PushButtonFramework = register_message_converters_HID_Keyboard();



}
}
