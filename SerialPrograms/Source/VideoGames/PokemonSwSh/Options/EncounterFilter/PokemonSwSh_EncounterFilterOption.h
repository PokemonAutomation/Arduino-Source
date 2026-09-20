/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EncounterFilterOption_H
#define PokemonAutomation_PokemonSwSh_EncounterFilterOption_H

#include "Common/Cpp/Options/BatchOption.h"
#include "PokemonSwSh_EncounterFilterOverride.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{




class EncounterFilterOption2 : public BatchOption{
public:
    EncounterFilterOption2(bool rare_stars, bool enable_overrides);

public:
    EnumDropdownOption<ShinyFilter> SHINY_FILTER;
    EncounterFilterTable FILTER_TABLE;
};





}
}
}
#endif
