/*  Max Lair Run Path Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Run_PathSelect_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Run_PathSelect_H

#include "NintendoSwitch/Framework/MultiSwitchProgram.h"
#include "PokemonSwSh/Programs/MaxLair/PokemonSwSh_MaxLair_State.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void run_path_select(
    ConsoleHandle& console,
    GlobalStateTracker& state_tracker
);


}
}
}
}
#endif
