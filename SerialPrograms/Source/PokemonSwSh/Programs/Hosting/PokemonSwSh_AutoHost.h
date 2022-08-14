/*  Auto-Hosting
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_AutoHost_H
#define PokemonAutomation_PokemonSwSh_AutoHost_H

#include "Common/Cpp/Options/RandomCodeOption.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "PokemonSwSh/Options/PokemonSwSh_Catchability.h"
#include "PokemonSwSh/Options/PokemonSwSh_AutoHostNotification.h"
#include "PokemonSwSh/Inference/Dens/PokemonSwSh_DenMonReader.h"

namespace PokemonAutomation{
    class BotBaseContext;
    struct ProgramInfo;
    class ProgramEnvironment;
    class StatsTracker;
namespace NintendoSwitch{
namespace PokemonSwSh{


bool connect_to_internet(
    ConsoleHandle& console, BotBaseContext& context,
    bool host_online,
    uint16_t connect_to_internet_delay
);

void send_raid_notification(
    const ProgramEnvironment& env,
    ConsoleHandle& console,
    AutoHostNotificationOption& settings,
    bool has_code, uint8_t code[8],
    const ImageViewRGB32& screenshot,
    const DenMonReadResults& results,
    const StatsTracker& stats_tracker
);


void run_autohost(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
    Catchability catchability, uint8_t skips,
    const RandomCodeOption* raid_code, uint16_t lobby_wait_delay,
    bool host_online, uint8_t accept_FR_slot,
    uint8_t move_slot, bool dynamax, uint8_t troll_hosting,
    AutoHostNotificationOption& notifications,
    uint16_t connect_to_internet_delay,
    uint16_t enter_online_den_delay,
    uint16_t open_online_den_lobby_delay,
    uint16_t raid_start_to_exit_delay,
    uint16_t delay_to_select_move
);


}
}
}
#endif
