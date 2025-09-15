/*  Region Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_RegionNavigation_H
#define PokemonAutomation_PokemonLA_RegionNavigation_H

#include "CommonFramework/Tools/VideoStream.h"
#include "PokemonLA/PokemonLA_Locations.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
namespace NintendoSwitch{
namespace PokemonLA{

struct TravelLocation;

void goto_professor(Logger& logger, ProControllerContext& context, Camp camp);

void goto_professor(Logger& logger, ProControllerContext& context, const TravelLocation& location);

void from_professor_return_to_jubilife(
    ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context
);


void mash_A_to_enter_sub_area(
    ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context
);
void mash_A_to_change_region(
    ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context
);

// Start from Jubilife Village gate, with camera facing towards the village, run towards the gate
// and travel to a location.
void goto_camp_from_jubilife(
    ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context,
    const TravelLocation& location,
    bool fresh_from_reset
);


// From overworld, open map, press X to open the fast travel popup menu and fast travel to
// the first camp from the menu.
void goto_camp_from_overworld(
    ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context
);

// From overworld, open map, press X to open the fast travel popup menu and go to the specified
// travel location.
void goto_any_camp_from_overworld(
    ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context,
    const TravelLocation& location
);

// From a camp location, run to Mai in that camp.
void goto_Mai_from_camp(
    Logger& logger, ProControllerContext& context, Camp camp
);

// From the program starting position at Jubilife Village, move backwards and talk to guard to open the map.
// Return after the map is detected.
// Throw an error if map is not found.
void open_travel_map_from_jubilife(
    ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context,
    bool fresh_from_reset
);


}
}
}
#endif
