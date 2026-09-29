/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_EncounterFilterOption_H
#define PokemonAutomation_PokemonBDSP_EncounterFilterOption_H

//#include <atomic>
#include "Common/Cpp/Options/BatchOption.h"
#include "PokemonBDSP_EncounterFilterOverride.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{



class EncounterFilterOption2 : public BatchOption{
public:
    EncounterFilterOption2(bool enable_overrides);

public:
    EnumDropdownOption<ShinyFilter> SHINY_FILTER;
    EncounterFilterTable FILTER_TABLE;
};


}
}
}
#endif
