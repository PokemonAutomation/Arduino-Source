/*  Max Lair State Machine
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_StateMachine_H
#define PokemonAutomation_PokemonSwSh_MaxLair_StateMachine_H

#include "CommonFramework/Options/ScreenshotFormatOption.h"
#include "NintendoSwitch/Framework/MultiSwitchProgram.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_Stats.h"
#include "PokemonSwSh_MaxLair_Options.h"
#include "PokemonSwSh_MaxLair_StateTracker.h"

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


enum class CaughtScreenAction{
    ALWAYS_STOP,
    TAKE_NON_BOSS_STOP_ON_NOTHING,
    TAKE_NON_BOSS_STOP_ON_SHINY_BOSS,
    RESET_HOST_IF_NON_SHINY_BOSS,
};


enum class StateMachineAction{
    KEEP_GOING,
    DONE_WITH_ADVENTURE,
    STOP_PROGRAM,
};


struct MaxLairRuntime{
    QString program_name;
    const MaxLairPlayerOptions* player_settings[4];
    CaughtScreenAction action;
    ScreenshotMode screenshot;
    Stats& stats;
};


//  Return true if done.
StateMachineAction run_state_iteration(
    MaxLairRuntime& runtime, size_t index,
    ProgramEnvironment& env,
    ConsoleHandle& console, bool is_host,
    GlobalStateTracker& state_tracker,
    const QImage& entrance
);



}
}
}
}
#endif
