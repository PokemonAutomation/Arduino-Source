/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonBDSP_EncounterFilterEnums.h"
#include "PokemonBDSP_EncounterFilterOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{




EncounterFilterOption2::EncounterFilterOption2(bool enable_overrides)
    : BatchOption(LockMode::UNLOCK_WHILE_RUNNING)
    , SHINY_FILTER(
        "<b>Stop on:</b>",
        ShinyFilter_Database(),
        LockMode::UNLOCK_WHILE_RUNNING,
        ShinyFilter::SHINY
    )
{
    PA_ADD_OPTION(SHINY_FILTER);
    if (enable_overrides){
        PA_ADD_OPTION(FILTER_TABLE);
    }
}





}
}
}
