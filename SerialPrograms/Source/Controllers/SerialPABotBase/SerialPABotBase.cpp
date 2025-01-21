/*  Serial Port (PABotBase)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "SerialPABotBase.h"

namespace PokemonAutomation{
namespace SerialPABotBase{


const char INTERFACE_NAME[] = "SerialPABotBase";


const char* to_string(Features feature){
    switch (feature){
    case Features::TickPrecise:             return "TickPrecise";
    case Features::NintendoSwitch_Basic:    return "NintendoSwitch_Basic";
    case Features::NintendoSwitch_Macros:   return "NintendoSwitch_Macros";
    case Features::NintendoSwitch_DateSkip: return "NintendoSwitch_DateSkip";
    }
    return nullptr;
}



std::set<std::string> program_id_to_features(uint8_t id){
    switch (id){
    case PABB_PID_PABOTBASE_12KB:
    case PABB_PID_PABOTBASE_31KB:
        return {
            to_string(Features::TickPrecise),
            to_string(Features::NintendoSwitch_Basic),
            to_string(Features::NintendoSwitch_Macros),
            to_string(Features::NintendoSwitch_DateSkip),
        };
    }
    return {};
}


const std::pair<std::string, std::set<std::string>> OLD_SERIAL_DEFAULT{
    SerialPABotBase::INTERFACE_NAME,
    {
        to_string(SerialPABotBase::Features::TickPrecise),
        to_string(SerialPABotBase::Features::NintendoSwitch_Basic),
        to_string(SerialPABotBase::Features::NintendoSwitch_Macros),
        to_string(SerialPABotBase::Features::NintendoSwitch_DateSkip),
    }
};







}
}
