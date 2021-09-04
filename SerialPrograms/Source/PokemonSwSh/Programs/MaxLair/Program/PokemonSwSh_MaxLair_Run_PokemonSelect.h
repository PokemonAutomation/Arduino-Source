/*  Max Lair Run Pokemon Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Run_PokemonSelect_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Run_PokemonSelect_H

#include "NintendoSwitch/Framework/MultiSwitchProgram.h"
#include "PokemonSwSh/Programs/MaxLair/PokemonSwSh_MaxLair_Options.h"
#include "PokemonSwSh/Programs/MaxLair/PokemonSwSh_MaxLair_State.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void run_select_pokemon(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    GlobalStateTracker& state_tracker,
    const MaxLairPlayerOptions& settings
);



}
}
}
}
#endif
