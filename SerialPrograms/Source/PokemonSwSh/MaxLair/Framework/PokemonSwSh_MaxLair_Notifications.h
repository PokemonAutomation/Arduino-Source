/*  Max Lair Notifications
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Notifications_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Notifications_H

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/NintendoSwitch_MultiSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_AutoHostNotification.h"
#include "PokemonSwSh_MaxLair_Stats.h"
#include "PokemonSwSh_MaxLair_StateMachine.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


std::string pokemon_name(const std::string& slug, const std::string& empty_string);



void send_status_notification(
    MultiSwitchProgramEnvironment& env,
    AdventureRuntime& runtime
);

void send_raid_notification(
    ProgramEnvironment& env,
    VideoStream& stream,
    AutoHostNotificationOption& settings,
    const std::string& code,
    const std::string& slug,
    const PathStats& path_stats,
    const StatsTracker& session_stats
);

void send_shiny_notification(
    ProgramEnvironment& env,
    Logger& logger, EventNotificationOption& settings,
    size_t console_index, size_t shinies,
    const std::set<std::string>* slugs,
    const PathStats& path_stats,
    const Stats& session_stats,
    const ImageViewRGB32& image
);


}
}
}
}
#endif
