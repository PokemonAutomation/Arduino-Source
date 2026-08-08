/*  BDSP Summary Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_SummaryNavigation_H
#define PokemonAutomation_PokemonBDSP_SummaryNavigation_H

#include "CommonFramework/Language.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Pokemon_StatsCalculation.h"
#include "PokemonBDSP_RngCalibration.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


bool open_menu(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

bool open_starter_summary(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

bool navigate_to_summary(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

bool read_observed_pokemon(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    Language language,
    const Pokemon::BaseStats& base_stats,
    uint8_t level,
    bool shiny,
    bool shiny_known,
    BdspObservedStarter& observed
);


}
}
}
#endif
