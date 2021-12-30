/*  Max Lair Notifications
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Notifications_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Notifications_H

#include <QImage>
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/Framework/NintendoSwitch_MultiSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_AutoHostNotification.h"
#include "PokemonSwSh_MaxLair_Stats.h"
#include "PokemonSwSh_MaxLair_StateMachine.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


QString pokemon_name(const std::string& slug, const QString& empty_string);



void send_status_notification(
    MultiSwitchProgramEnvironment& env,
    AdventureRuntime& runtime
);

void send_raid_notification(
    const ProgramInfo& program_info,
    ConsoleHandle& console,
    AutoHostNotificationOption& settings,
    bool has_code, uint8_t code[8],
    const std::string& slug,
    const PathStats& path_stats,
    const StatsTracker& session_stats
);

void send_shiny_notification(
    Logger& logger, EventNotificationOption& settings,
    const ProgramInfo& program_info,
    size_t console_index, size_t shinies,
    const std::set<std::string>* slugs,
    const PathStats& path_stats,
    const Stats& session_stats,
    const QImage& image
);


}
}
}
}
#endif
