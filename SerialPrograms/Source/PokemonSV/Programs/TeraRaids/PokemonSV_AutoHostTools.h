/*  Auto Host Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoHostTools_H
#define PokemonAutomation_PokemonSV_AutoHostTools_H

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



void send_host_announcement(
    ProgramEnvironment& env, ConsoleHandle& host_console,
    const std::string& lobby_code, bool show_raid_code,
    const std::string& description,
    EventNotificationOption& NOTIFICATION_RAID_POST
);



}
}
}
#endif
