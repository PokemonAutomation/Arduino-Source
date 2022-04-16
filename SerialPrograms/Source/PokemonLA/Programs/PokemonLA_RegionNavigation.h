/*  Region Navigation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_RegionNavigation_H
#define PokemonAutomation_PokemonLA_RegionNavigation_H

#include "PokemonLA/PokemonLA_Locations.h"
#include "PokemonLA/PokemonLA_TravelLocations.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
#include "PokemonLA/Inference/PokemonLA_SelectedRegionDetector.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
    class ConsoleHandle;
namespace NintendoSwitch{
namespace PokemonLA{


void goto_professor(Logger& logger, BotBaseContext& context, Camp camp);
void from_professor_return_to_jubilife(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context
);


void mash_A_to_enter_sub_area(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context
);
void mash_A_to_change_region(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context
);
void goto_camp_from_jubilife(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
    const TravelLocation& location
);


void goto_camp_from_overworld(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
    ShinyDetectedActionOption& options,
    ShinyStatIncrementer& shiny_stat_incrementer
);

void goto_any_camp_from_overworld(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
    const TravelLocation& location
);



}
}
}
#endif
