/*  Auto-Hosting
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_AutoHost_H
#define PokemonAutomation_PokemonSwSh_AutoHost_H

#include "Common/Cpp/Options/RandomCodeOption.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "PokemonSwSh/Options/PokemonSwSh_Catchability.h"
#include "PokemonSwSh/Options/PokemonSwSh_AutoHostNotification.h"
#include "PokemonSwSh/Inference/Dens/PokemonSwSh_DenMonReader.h"

namespace PokemonAutomation{
    struct ProgramInfo;
    class ProgramEnvironment;
    class StatsTracker;
namespace NintendoSwitch{
namespace PokemonSwSh{


bool connect_to_internet(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    bool host_online,
    Milliseconds connect_to_internet_delay
);

void send_raid_notification(
    const ProgramEnvironment& env,
    VideoStream& stream,
    AutoHostNotificationOption& settings,
    const std::string& code,
    const ImageViewRGB32& screenshot,
    const DenMonReadResults& results,
    const StatsTracker& stats_tracker
);


void run_autohost(
    ProgramEnvironment& env,
    ConsoleHandle& console, ProControllerContext& context,
    Catchability catchability, uint8_t skips,
    const RandomCodeOption* raid_code, Milliseconds lobby_wait_delay,
    bool host_online, uint8_t accept_FR_slot,
    uint8_t move_slot, bool dynamax, uint8_t troll_hosting,
    AutoHostNotificationOption& notifications,
    Milliseconds connect_to_internet_delay,
    Milliseconds enter_online_den_delay,
    Milliseconds open_online_den_lobby_delay,
    Milliseconds raid_start_to_exit_delay,
    Milliseconds delay_to_select_move
);


}
}
}
#endif
