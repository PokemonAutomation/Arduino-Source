/*  Serial Port (PABotBase)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Controllers_SerialPABotBase_H
#define PokemonAutomation_Controllers_SerialPABotBase_H

#include <stdint.h>
#include <string>
#include <set>

namespace PokemonAutomation{
namespace SerialPABotBase{


//  Supported Controllers
extern const char NintendoSwitch_Basic[];


//  Feature List
enum class Features{
    TickPrecise,
    NintendoSwitch_Basic,
    NintendoSwitch_SSF,
    NintendoSwitch_Macros,
    NintendoSwitch_DateSkip,
};
const char* to_string(Features feature);


//  Defaults
extern const std::pair<std::string, std::set<std::string>> OLD_NINTENDO_SWITCH_DEFAULT_REQUIREMENTS;


//  Internal Parsing
std::set<std::string> program_id_to_features(uint8_t id);






}
}
#endif
