/*  Max Lair Notifications
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Notifications_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Notifications_H

#include <QImage>
#include "CommonFramework/Tools/Logger.h"
#include "PokemonSwSh_MaxLair_Stats.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void send_shiny_notification(
    Logger& logger,
    const QString& program,
    const std::set<std::string>* slugs,
    const QImage& screenshot,
    const Stats& stats
);


}
}
}
}
#endif
