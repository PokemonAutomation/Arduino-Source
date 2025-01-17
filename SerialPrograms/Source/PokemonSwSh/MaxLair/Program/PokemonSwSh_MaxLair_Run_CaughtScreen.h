/*  Max Lair Run Caught Screen
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Run_CaughtScreen_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Run_CaughtScreen_H

#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateMachine.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


StateMachineAction run_caught_screen(
    AdventureRuntime& runtime,
    ProgramEnvironment& env, ConsoleHandle& console, ControllerContext& context,
    GlobalStateTracker& state_tracker,
    const EndBattleDecider& decider,
    const ImageViewRGB32& entrance
);



}
}
}
}
#endif
