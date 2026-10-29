/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonSwSh_EncounterFilterEnums.h"
#include "PokemonSwSh_EncounterFilterOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{





EncounterFilterOption2::EncounterFilterOption2(bool rare_stars, bool enable_overrides)
    : BatchOption(LockMode::UNLOCK_WHILE_RUNNING)
    , SHINY_FILTER(
        "<b>Stop on:</b>",
        rare_stars ? ShinyFilter_StopRareStars_Database() : ShinyFilter_Normal_Database(),
        LockMode::UNLOCK_WHILE_RUNNING,
        ShinyFilter::ANY_SHINY
    )
    , FILTER_TABLE(rare_stars)
{
    PA_ADD_OPTION(SHINY_FILTER);
    if (enable_overrides){
        PA_ADD_OPTION(FILTER_TABLE);
    }
}





















}
}
}
