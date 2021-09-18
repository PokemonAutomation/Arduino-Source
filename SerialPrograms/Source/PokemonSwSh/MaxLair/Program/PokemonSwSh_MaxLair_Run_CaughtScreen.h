/*  Max Lair Run Caught Screen
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Run_CaughtScreen_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Run_CaughtScreen_H

#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateMachine.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


StateMachineAction run_caught_screen(
    MaxLairRuntime& runtime,
    ProgramEnvironment& env,
    ConsoleHandle& console, bool is_host,
    const QImage& entrance
);



}
}
}
}
#endif
