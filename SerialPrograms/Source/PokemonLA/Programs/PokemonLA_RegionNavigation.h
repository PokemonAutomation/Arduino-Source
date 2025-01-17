/*  Region Navigation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_RegionNavigation_H
#define PokemonAutomation_PokemonLA_RegionNavigation_H

#include "PokemonLA/PokemonLA_Locations.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
#include "PokemonLA/Inference/Map/PokemonLA_SelectedRegionDetector.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
namespace NintendoSwitch{
    class ConsoleHandle;
namespace PokemonLA{

struct TravelLocation;

void goto_professor(Logger& logger, ControllerContext& context, Camp camp);

void goto_professor(Logger& logger, ControllerContext& context, const TravelLocation& location);

void from_professor_return_to_jubilife(
    ProgramEnvironment& env, ConsoleHandle& console, ControllerContext& context
);


void mash_A_to_enter_sub_area(
    ProgramEnvironment& env, ConsoleHandle& console, ControllerContext& context
);
void mash_A_to_change_region(
    ProgramEnvironment& env, ConsoleHandle& console, ControllerContext& context
);
void goto_camp_from_jubilife(
    ProgramEnvironment& env, ConsoleHandle& console, ControllerContext& context,
    const TravelLocation& location
);


void goto_camp_from_overworld(
    ProgramEnvironment& env, ConsoleHandle& console, ControllerContext& context
);

void goto_any_camp_from_overworld(
    ProgramEnvironment& env, ConsoleHandle& console, ControllerContext& context,
    const TravelLocation& location
);

// From a camp location, run to Mai in that camp.
void goto_Mai_from_camp(
    Logger& logger, ControllerContext& context, Camp camp
);

// From the program starting position at Jubilife Village, move backwards and talk to guard to open the map.
// Return after the map is detected.
// Throw an error if map is not found.
void open_travel_map_from_jubilife(
    ProgramEnvironment& env, ConsoleHandle& console, ControllerContext& context
);


}
}
}
#endif
