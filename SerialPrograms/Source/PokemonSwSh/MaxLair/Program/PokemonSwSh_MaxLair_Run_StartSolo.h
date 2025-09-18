/*  Max Lair Run Start (Solo)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Run_StartSolo_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Run_StartSolo_H

#include <memory>
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "PokemonSwSh/Inference/PokemonSwSh_QuantityReader.h"
#include "PokemonSwSh/MaxLair/Options/PokemonSwSh_MaxLair_Options_Hosting.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_Stats.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateTracker.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


bool wait_for_a_player(
    VideoStream& stream, ProControllerContext& context,
    const ImageViewRGB32& entrance,
    WallClock time_limit
);
bool wait_for_lobby_ready(
    VideoStream& stream, ProControllerContext& context,
    const ImageViewRGB32& entrance,
    size_t min_players,
    size_t start_players,
    WallClock time_limit
);
bool start_adventure(
    VideoStream& stream, ProControllerContext& context,
    size_t consoles
);


bool start_raid_self_solo(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    GlobalStateTracker& state_tracker,
    std::shared_ptr<const ImageRGB32>& entrance, size_t boss_slot,
    ReadableQuantity999& ore
);

bool start_raid_host_solo(
    ProgramEnvironment& env, ConsoleHandle& console, ProControllerContext& context,
    GlobalStateTracker& state_tracker,
    std::shared_ptr<const ImageRGB32>& entrance, size_t boss_slot,
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
