/*  Max Lair Run Start
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Run_Start_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Run_Start_H

#include "NintendoSwitch/Framework/MultiSwitchProgram.h"
#include "PokemonSwSh/Programs/MaxLair/PokemonSwSh_MaxLair_State.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void start_raid(
    MultiSwitchProgramEnvironment& env,
    GlobalStateTracker& state_tracker,
    ConsoleHandle& host, size_t boss_slot,
    const uint8_t code[8]
);


}
}
}
}
#endif
