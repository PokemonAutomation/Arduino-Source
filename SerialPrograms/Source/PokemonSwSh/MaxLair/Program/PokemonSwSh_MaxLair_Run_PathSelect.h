/*  Max Lair Run Path Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Run_PathSelect_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Run_PathSelect_H

#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateTracker.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void run_path_select(
    ProgramEnvironment& env, ConsoleHandle& console, SwitchControllerContext& context,
    GlobalStateTracker& state_tracker
);


}
}
}
}
#endif
