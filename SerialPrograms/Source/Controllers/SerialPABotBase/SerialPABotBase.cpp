/*  Serial Port (PABotBase)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/PokemonSwSh/PokemonProgramIDs.h"
#include "SerialPABotBase.h"

namespace PokemonAutomation{
namespace SerialPABotBase{



const char NintendoSwitch_Basic[] = "NintendoSwitch-SerialPABotBase";


//  Feature List
const char* to_string(Features feature){
    switch (feature){
    case Features::TickPrecise:             return "TickPrecise";
    case Features::NintendoSwitch_Basic:    return "NintendoSwitch_Basic";
    case Features::NintendoSwitch_SSF:      return "NintendoSwitch_SSF";
    case Features::NintendoSwitch_Macros:   return "NintendoSwitch_Macros";
    case Features::NintendoSwitch_DateSkip: return "NintendoSwitch_DateSkip";
    }
    return nullptr;
}


//  Defaults
const std::pair<std::string, std::set<std::string>> OLD_NINTENDO_SWITCH_DEFAULT_REQUIREMENTS{
    NintendoSwitch_Basic,
    {
        to_string(Features::TickPrecise),
        to_string(Features::NintendoSwitch_Basic),
//        to_string(Features::NintendoSwitch_SSF),
//        to_string(Features::NintendoSwitch_Macros),
    }
};


//  Internal Parsing
std::set<std::string> program_id_to_features(uint8_t id){
    switch (id){
    case PABB_PID_PABOTBASE_12KB:
        return {
            to_string(Features::TickPrecise),
            to_string(Features::NintendoSwitch_Basic),
            to_string(Features::NintendoSwitch_SSF),
            to_string(Features::NintendoSwitch_Macros),
//            to_string(Features::NintendoSwitch_DateSkip),
        };
    case PABB_PID_PABOTBASE_31KB:
        return {
            to_string(Features::TickPrecise),
            to_string(Features::NintendoSwitch_Basic),
            to_string(Features::NintendoSwitch_SSF),
            to_string(Features::NintendoSwitch_Macros),
            to_string(Features::NintendoSwitch_DateSkip),
        };
    }
    return {};
}









}
}
