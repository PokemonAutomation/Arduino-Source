/*  Max Lair Run Start
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Run_Start_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Run_Start_H

#include <memory>
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "NintendoSwitch/NintendoSwitch_MultiSwitchProgram.h"
#include "PokemonSwSh/MaxLair/Options/PokemonSwSh_MaxLair_Options_Hosting.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateTracker.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateMachine.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


//  Returns true if adventure started. False if you errored and are back at the entrance.
bool start_adventure(
    MultiSwitchProgramEnvironment& env, CancellableScope& scope,
    GlobalStateTracker& state_tracker,
    std::shared_ptr<const ImageRGB32> entrance[4],
    ConsoleHandle& host, size_t boss_slot,
    HostingSettings& settings,
    const PathStats& path_stats,
    const StatsTracker& session_stats,
    ConsoleRuntime console_stats[4]
);



}
}
}
}
#endif
