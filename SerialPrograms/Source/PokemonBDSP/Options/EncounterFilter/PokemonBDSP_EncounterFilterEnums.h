/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_EncounterFilterEnums_H
#define PokemonAutomation_PokemonBDSP_EncounterFilterEnums_H

#include <vector>
#include <map>
#include <QString>
#include "PokemonBDSP_EncounterFilterOverride.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


extern const std::vector<QString> ShinyFilter_NAMES;
extern const std::map<QString, ShinyFilter> ShinyFilter_MAP;

extern const std::vector<QString> EncounterAction_NAMES;
extern const std::map<QString, EncounterAction> EncounterAction_MAP;



}
}
}
#endif
