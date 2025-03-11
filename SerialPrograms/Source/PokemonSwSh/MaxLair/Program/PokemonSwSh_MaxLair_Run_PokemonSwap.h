/*  Max Lair Run Pokemon Swap
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Run_PokemonSwap_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Run_PokemonSwap_H

#include "PokemonSwSh/MaxLair/Options/PokemonSwSh_MaxLair_Options_Consoles.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateTracker.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateMachine.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void run_swap_pokemon(
    size_t console_index,
    AdventureRuntime& runtime,
    VideoStream& stream, ProControllerContext& context,
    GlobalStateTracker& state_tracker,
    const ConsoleSpecificOptions& settings
);



}
}
}
}
#endif
