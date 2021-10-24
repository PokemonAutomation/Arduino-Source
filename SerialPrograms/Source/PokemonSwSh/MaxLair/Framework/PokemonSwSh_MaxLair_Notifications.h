/*  Max Lair Notifications
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Notifications_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Notifications_H

#include <QImage>
#include "CommonFramework/Tools/Logger.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "PokemonSwSh_MaxLair_Stats.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void send_nonshiny_notification(
    Logger& logger, EventNotificationOption& settings,
    const QString& program,
    const Stats& stats
);
void send_shiny_notification(
    Logger& logger, EventNotificationOption& settings,
    const QString& program,
    const std::set<std::string>* slugs,
    const Stats& stats,
    const QImage& image
);


}
}
}
}
#endif
