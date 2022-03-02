/*  Region Navigation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_RegionNavigation_H
#define PokemonAutomation_PokemonLA_RegionNavigation_H

#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
#include "PokemonLA/Inference/PokemonLA_SelectedRegionDetector.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
    class ConsoleHandle;
namespace NintendoSwitch{
namespace PokemonLA{


enum class Camp{
    FIELDLANDS_FIELDLANDS,
    FIELDLANDS_HEIGHTS,
    MIRELANDS_MIRELANDS,
    MIRELANDS_BOGBOUND,
    COASTLANDS_BEACHSIDE,
    COASTLANDS_COASTLANDS,
    HIGHLANDS_HIGHLANDS,
    HIGHLANDS_MOUNTAIN,
    HIGHLANDS_SUMMIT,
    ICELANDS_SNOWFIELDS,
    ICELANDS_ICEPEAK,
};
void goto_professor(ConsoleHandle& console, Camp camp);
void from_professor_return_to_jubilife(ProgramEnvironment& env, ConsoleHandle& console);


void mash_A_to_change_region(ProgramEnvironment& env, ConsoleHandle& console);
void goto_camp_from_jubilife(ProgramEnvironment& env, ConsoleHandle& console, Camp camp);


void goto_camp_from_overworld(
    ProgramEnvironment& env, ConsoleHandle& console,
    ShinyDetectedActionOption& options
);



}
}
}
#endif
