/*  SerialPABotBase ESP32 Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <sstream>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/SerialPABotBase/SerialPABotBase_Messages_ESP32.h"
#include "ClientSource/Libraries/MessageConverter.h"
#include "CommonFramework/GlobalSettingsPanel.h"

namespace PokemonAutomation{
namespace SerialPABotBase{



int register_message_converters_ESP32(){
    register_message_converter(
        PABB_MSG_REQUEST_STATUS,
        [](const std::string& body){
            //  Disable this by default since it's very spammy.
            if (!GlobalSettings::instance().LOG_EVERYTHING){
                return std::string();
            }
            std::ostringstream ss;
            ss << "PABB_MSG_REQUEST_STATUS() - ";
            if (body.size() != sizeof(pabb_Message_RequestStatus)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_Message_RequestStatus*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_ESP32_REQUEST_READ_SPI,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "PABB_MSG_ESP32_REQUEST_READ_SPI() - ";
            if (body.size() != sizeof(pabb_Message_ESP32_ReadSpi)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_Message_ESP32_ReadSpi*)body.c_str();
            ss << "seqnum = " << params->seqnum;
            ss << ", controller = " << params->controller_type;
            ss << ", address = 0x" << tostr_hex(params->address);
            ss << ", bytes = " << (size_t)params->bytes;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_ESP32_REQUEST_WRITE_SPI,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "PABB_MSG_ESP32_REQUEST_WRITE_SPI() - ";
            if (body.size() <= sizeof(pabb_Message_ESP32_WriteSpi)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_Message_ESP32_WriteSpi*)body.c_str();
            ss << "seqnum = " << params->seqnum;
            ss << ", controller = " << params->controller_type;
            ss << ", address = 0x" << tostr_hex(params->address);
            ss << ", bytes = " << (size_t)params->bytes;
            return ss.str();
        }
    );
#if 0
    register_message_converter(
        PABB_MSG_ESP32_REQUEST_GET_COLORS,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "PABB_MSG_ESP32_REQUEST_GET_COLORS() - ";
            if (body.size() != sizeof(pabb_Message_ESP32_GetColors)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_Message_ESP32_GetColors*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", controller = " << (uint64_t)params->controller_type;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_ESP32_REQUEST_SET_COLORS,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "PABB_MSG_ESP32_REQUEST_SET_COLORS() - ";
            if (body.size() != sizeof(pabb_Message_ESP32_SetColors)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_Message_ESP32_SetColors*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
#endif
    register_message_converter(
        PABB_MSG_ESP32_CONTROLLER_STATE_BUTTONS,
        [](const std::string& body){
            //  Disable this by default since it's very spammy.
            if (!GlobalSettings::instance().LOG_EVERYTHING){
                return std::string();
            }
            std::ostringstream ss;
            ss << "PABB_MSG_ESP32_CONTROLLER_STATE_BUTTONS() - ";
            if (body.size() != sizeof(pabb_Message_ESP32_CommandButtonState)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_Message_ESP32_CommandButtonState*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", milliseconds = " << params->milliseconds;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_ESP32_CONTROLLER_STATE_FULL,
        [](const std::string& body){
            //  Disable this by default since it's very spammy.
            if (!GlobalSettings::instance().LOG_EVERYTHING){
                return std::string();
            }
            std::ostringstream ss;
            ss << "PABB_MSG_ESP32_CONTROLLER_STATE_FULL() - ";
            if (body.size() != sizeof(pabb_Message_ESP32_CommandFullState)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_Message_ESP32_CommandFullState*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", milliseconds = " << params->milliseconds;
            return ss.str();
        }
    );
#if 0
    register_message_converter(
        PABB_MSG_ESP32_REPORT,
        [](const std::string& body){
            //  Disable this by default since it's very spammy.
            if (!GlobalSettings::instance().LOG_EVERYTHING){
                return std::string();
            }
            std::ostringstream ss;
            ss << "ESP32_controller_state() - ";
            if (body.size() != sizeof(pabb_esp32_report30)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_esp32_report30*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", ticks = " << (int)params->ticks;
            ss << ", active = " << (int)params->active;
            return ss.str();
        }
    );
#endif
    return 0;
}
int init_Messages_ESP32 = register_message_converters_ESP32();



}
}
