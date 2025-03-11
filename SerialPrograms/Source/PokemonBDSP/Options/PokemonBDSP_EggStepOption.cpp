/*  Egg Step Count Dropdown
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Pokemon/Resources/Pokemon_EggSteps.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokemonSprites.h"
#include "PokemonBDSP_EggStepOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{





const Pokemon::EggStepDatabase& EGGSTEP_DATABASE(){
    static Pokemon::EggStepDatabase database("PokemonBDSP/EggSteps.json", &PokemonSwSh::ALL_POKEMON_SPRITES());
    return database;
}



EggStepCountOption::EggStepCountOption()
    : StringSelectOption(
        "<b>Step Count:</b>",
        EGGSTEP_DATABASE().database(),
        LockMode::LOCK_WHILE_RUNNING,
        "turtwig"
    )
{}

EggStepCountOption::operator uint16_t() const{
    return (uint16_t)EGGSTEP_DATABASE().step_count(this->slug());
}



}
}
}
