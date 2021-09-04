/*  Max Lair State Machine
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_StateMachine_H
#define PokemonAutomation_PokemonSwSh_MaxLair_StateMachine_H

#include "NintendoSwitch/Framework/MultiSwitchProgram.h"
#include "PokemonSwSh_MaxLair_Options.h"
#include "PokemonSwSh_MaxLair_State.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


enum class ConsoleState{
    CONSOLE_DEAD,
    POKEMON_SELECT,
    PATH_SELECT,
    ITEM_SELECT,
    MOVE_SELECT,
    CATCH_SELECT,
    POKEMON_SWAP,
    CAUGHT_MENU,
};

enum class Action{
    RESET_ALL,
    CONTINUE,
    DONE,
};


//  Return true if done.
Action run_state_iteration(
    const MaxLairPlayerOptions& player_settings,
    ProgramEnvironment& env,
    ConsoleHandle& console,
    GlobalStateTracker& state_tracker
);



}
}
}
}
#endif
