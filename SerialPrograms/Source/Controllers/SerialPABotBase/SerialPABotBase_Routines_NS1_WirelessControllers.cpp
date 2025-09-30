/*  SerialPABotBase (Nintendo Switch 1 Wireless Controllers)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <sstream>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/SerialPABotBase/SerialPABotBase_Messages_NS1_WirelessControllers.h"
#include "Controllers/SerialPABotBase/Connection/MessageConverter.h"
#include "CommonFramework/GlobalSettingsPanel.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace SerialPABotBase{



void register_message_converters_NS1_WirelessControllers(){
    register_message_converter(
        PABB_MSG_REQUEST_NS1_WIRELESS_CONTROLLER_READ_SPI,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "PABB_MSG_REQUEST_NS1_WIRELESS_CONTROLLER_READ_SPI() - ";
            if (body.size() != sizeof(pabb_Message_NS1_WirelessController_ReadSpi)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_Message_NS1_WirelessController_ReadSpi*)body.c_str();
            ss << "seqnum = " << params->seqnum;
            ss << ", controller = " << params->controller_type;
            ss << ", address = 0x" << tostr_hex(params->address);
            ss << ", bytes = " << (size_t)params->bytes;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_REQUEST_NS1_WIRELESS_CONTROLLER_WRITE_SPI,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "PABB_MSG_REQUEST_NS1_WIRELESS_CONTROLLER_WRITE_SPI() - ";
            if (body.size() <= sizeof(pabb_Message_NS1_WirelessController_WriteSpi)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_Message_NS1_WirelessController_WriteSpi*)body.c_str();
            ss << "seqnum = " << params->seqnum;
            ss << ", controller = " << params->controller_type;
            ss << ", address = 0x" << tostr_hex(params->address);
            ss << ", bytes = " << (size_t)params->bytes;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_NS1_WIRELESS_CONTROLLER_BUTTONS,
        [](const std::string& body){
            //  Disable this by default since it's very spammy.
            if (!GlobalSettings::instance().LOG_EVERYTHING){
                return std::string();
            }
            std::ostringstream ss;
            ss << "PABB_MSG_COMMAND_NS1_WIRELESS_CONTROLLER_BUTTONS() - ";
            if (body.size() != sizeof(pabb_Message_Command_NS1_WirelessController_Buttons)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_Message_Command_NS1_WirelessController_Buttons*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", milliseconds = " << params->milliseconds;

            //  Do not log the contents of the command due to privacy concerns.
            //  (people entering passwords)

            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_NS1_WIRELESS_CONTROLLER_FULL_STATE,
        [](const std::string& body){
            //  Disable this by default since it's very spammy.
            if (!GlobalSettings::instance().LOG_EVERYTHING){
                return std::string();
            }
            std::ostringstream ss;
            ss << "PABB_MSG_COMMAND_NS1_WIRELESS_CONTROLLER_FULL_STATE() - ";
            if (body.size() != sizeof(pabb_Message_Command_NS1_WirelessController_FullState)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_Message_Command_NS1_WirelessController_FullState*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", milliseconds = " << params->milliseconds;

            //  Do not log the contents of the command due to privacy concerns.
            //  (people entering passwords)

            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_INFO_NS1_WIRELESS_CONTROLLER_RUMBLE,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "PABB_MSG_INFO_NS1_WIRELESS_CONTROLLER_RUMBLE(): ";
            if (body.size() != sizeof(pabb_MsgInfo_NS1_WirelessController_Rumble)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_MsgInfo_NS1_WirelessController_Rumble*)body.c_str();
            if (!GlobalSettings::instance().LOG_EVERYTHING && memcmp(
                    params,
                    &pabb_NintendoSwitch_Rumble_NEUTRAL_STATE,
                    sizeof(pabb_NintendoSwitch_Rumble_NEUTRAL_STATE)
                ) == 0
            ){
                return std::string();
            }
            static const char HEX_DIGITS[] = "0123456789abcdef";
            for (size_t c = 0; c < body.size(); c++){
                uint8_t byte = body[c];
                ss << " " << HEX_DIGITS[(byte >> 4)] << HEX_DIGITS[byte & 15];
            }
            return ss.str();
        }
    );
}



}
}
