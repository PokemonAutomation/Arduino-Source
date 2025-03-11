/*  Max Lair Run Battle
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Run_Battle_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Run_Battle_H

#include "CommonFramework/Language.h"
#include "PokemonSwSh/MaxLair/Options/PokemonSwSh_MaxLair_Options.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateMachine.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateTracker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


StateMachineAction run_move_select(
    ProgramEnvironment& env, size_t console_index,
    VideoStream& stream, ProControllerContext& context,
    OcrFailureWatchdog& ocr_watchdog,
    GlobalStateTracker& state_tracker,
    const ConsoleSpecificOptions& settings,
    bool currently_dmaxed, bool cheer_only
);

StateMachineAction throw_balls(
    AdventureRuntime& runtime,
    ProgramEnvironment& env, size_t console_index,
    VideoStream& stream, ProControllerContext& context,
    Language language,
    OcrFailureWatchdog& ocr_watchdog,
    GlobalStateTracker& state_tracker,
    const EndBattleDecider& decider
);



}
}
}
}
#endif
