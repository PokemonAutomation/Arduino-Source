/*  Egg Step Count Dropdown
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Pokemon/Resources/Pokemon_EggSteps.h"
#include "PokemonBDSP_EggStepOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{





const Pokemon::EggStepDatabase& EGGSTEP_DATABASE(){
    static Pokemon::EggStepDatabase database("PokemonBDSP/EggSteps.json");
    return database;
}



EggStepCountOption::EggStepCountOption()
    : StringSelectOption(
        "<b>Step Count:</b>",
        EGGSTEP_DATABASE().database(),
        LockWhileRunning::LOCKED,
        "grookey"
    )
{}

EggStepCountOption::operator uint16_t() const{
    return (uint16_t)EGGSTEP_DATABASE().step_count(this->slug());
}



}
}
}
