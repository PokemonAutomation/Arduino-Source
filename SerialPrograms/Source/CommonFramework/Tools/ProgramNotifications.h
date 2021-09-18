/*  Program Notifications
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ProgramNotifications_H
#define PokemonAutomation_ProgramNotifications_H

#include <vector>
#include <QString>
#include "CommonFramework/Tools/Logger.h"

namespace PokemonAutomation{


void send_program_notification(
    Logger& logger,
    bool should_ping, QColor color,
    const QString& program_name,
    const QString& title,
    const std::vector<std::pair<QString, QString>>& messages
);

void send_program_status_notification(
    Logger& logger, bool should_ping,
    const QString& program_name,
    const QString& message = "",
    const std::string& stats = ""
);

void send_program_finished_notification(
    Logger& logger, bool should_ping,
    const QString& program_name,
    const QString& message = "",
    const std::string& stats = ""
);

void send_program_error_notification(
    Logger& logger,
    const QString& program_name,
    const QString& message,
    const std::string& stats = ""
);


}
#endif
