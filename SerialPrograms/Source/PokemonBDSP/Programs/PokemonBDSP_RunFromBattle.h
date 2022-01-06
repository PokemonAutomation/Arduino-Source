/*  Run from Battle
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_RunFromBattle_H
#define PokemonAutomation_PokemonBDSP_RunFromBattle_H

#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


//  The cursor must be over the "Run" button.
bool run_from_battle(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    uint16_t exit_battle_time
);


}
}
}
#endif
