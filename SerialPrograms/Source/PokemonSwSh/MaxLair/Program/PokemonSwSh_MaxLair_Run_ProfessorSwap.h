/*  Max Lair Run Professor Swap
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Run_ProfessorSwap_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Run_ProfessorSwap_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateTracker.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateMachine.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void run_professor_swap(
    size_t console_index,
    AdventureRuntime& runtime,
    VideoStream& stream, ProControllerContext& context,
    GlobalStateTracker& state_tracker
);



}
}
}
}
#endif
