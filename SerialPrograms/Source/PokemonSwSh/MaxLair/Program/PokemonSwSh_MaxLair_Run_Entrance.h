/*  Max Lair Run Entrance
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Run_Entrance_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Run_Entrance_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateTracker.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateMachine.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void run_entrance(
    AdventureRuntime& runtime,
    ProgramEnvironment& env, size_t console_index,
    VideoStream& stream, ProControllerContext& context,
    bool save_path,
    GlobalStateTracker& state_tracker
);



}
}
}
}
#endif
