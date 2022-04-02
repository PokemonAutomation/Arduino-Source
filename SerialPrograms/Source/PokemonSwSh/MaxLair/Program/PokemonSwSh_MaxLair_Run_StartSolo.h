/*  Max Lair Run Start (Solo)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Run_StartSolo_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Run_StartSolo_H

#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "PokemonSwSh/Inference/PokemonSwSh_QuantityReader.h"
#include "PokemonSwSh/MaxLair/Options/PokemonSwSh_MaxLair_Options_Hosting.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_Stats.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateTracker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


bool wait_for_a_player(
    ProgramEnvironment& env, const BotBaseContext& context, ConsoleHandle& console,
    const QImage& entrance,
    std::chrono::system_clock::time_point time_limit
);
bool wait_for_lobby_ready(
    ProgramEnvironment& env, const BotBaseContext& context, ConsoleHandle& console,
    const QImage& entrance,
    size_t min_players,
    size_t start_players,
    std::chrono::system_clock::time_point time_limit
);
bool start_adventure(
    ProgramEnvironment& env, const BotBaseContext& context, ConsoleHandle& console,
    size_t consoles,
    const QImage& entrance
);


bool start_raid_self_solo(
    ProgramEnvironment& env, const BotBaseContext& context, ConsoleHandle& console,
    GlobalStateTracker& state_tracker,
    QImage& entrance, size_t boss_slot,
    ReadableQuantity999& ore
);

bool start_raid_host_solo(
    ProgramEnvironment& env, const BotBaseContext& context, ConsoleHandle& console,
    GlobalStateTracker& state_tracker,
    QImage& entrance, size_t boss_slot,
    HostingSettings& settings,
    const PathStats& path_stats,
    const StatsTracker& session_stats,
    ReadableQuantity999& ore
);


}
}
}
}
#endif
