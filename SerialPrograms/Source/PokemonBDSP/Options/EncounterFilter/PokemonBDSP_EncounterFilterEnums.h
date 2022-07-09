/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_EncounterFilterEnums_H
#define PokemonAutomation_PokemonBDSP_EncounterFilterEnums_H

#include <string>
#include <vector>
#include <map>
#include "PokemonBDSP_EncounterFilterOverride.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


extern const std::vector<std::string> ShinyFilter_NAMES;
extern const std::map<std::string, ShinyFilter> ShinyFilter_MAP;

extern const std::vector<std::string> EncounterAction_NAMES;
extern const std::map<std::string, EncounterAction> EncounterAction_MAP;



}
}
}
#endif
