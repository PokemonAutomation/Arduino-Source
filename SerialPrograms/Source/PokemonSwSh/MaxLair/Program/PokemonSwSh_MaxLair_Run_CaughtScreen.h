/*  Max Lair Run Caught Screen
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Run_CaughtScreen_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Run_CaughtScreen_H

#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateMachine.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


StateMachineAction run_caught_screen(
    AdventureRuntime& runtime,
    ProgramEnvironment& env, size_t console_index,
    ConsoleHandle& console, ProControllerContext& context,
    GlobalStateTracker& state_tracker,
    const EndBattleDecider& decider,
    const ImageViewRGB32& entrance
);



}
}
}
}
#endif
