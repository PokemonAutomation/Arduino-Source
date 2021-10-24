/*  Auto-Hosting
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_AutoHost_H
#define PokemonAutomation_PokemonSwSh_AutoHost_H

#include "CommonFramework/Options/RandomCodeOption.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "PokemonSwSh/Options/PokemonSwSh_AutoHostNotification.h"
#include "PokemonSwSh/Inference/Dens/PokemonSwSh_DenMonReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


bool connect_to_internet(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    bool host_online,
    uint16_t connect_to_internet_delay
);

void send_raid_notification(
    const QString& program_name,
    ConsoleHandle& console,
    AutoHostNotificationOption& settings,
    bool has_code, uint8_t code[8],
    const DenMonReader& reader,
    const StatsTracker& stats_tracker
);


void run_autohost(
    ProgramEnvironment& env, ConsoleHandle& console,
    const QString& program_name,
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
