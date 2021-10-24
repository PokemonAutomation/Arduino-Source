/*  Program Notifications
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ProgramNotifications_H
#define PokemonAutomation_ProgramNotifications_H

#include <vector>
#include <QString>
#include <QImage>
#include "CommonFramework/Tools/Logger.h"
#include "MessageAttachment.h"
#include "EventNotificationOption.h"

namespace PokemonAutomation{


void send_program_notification(
    Logger& logger, EventNotificationOption& settings,
    QColor color,
    const QString& program_name,
    const QString& title,
    const std::vector<std::pair<QString, QString>>& messages,
    const QImage& image = QImage(), bool keep_file = false
);

void send_program_telemetry(
    Logger& logger, bool is_error, QColor color,
    const QString& program_name,
    const QString& title,
    const std::vector<std::pair<QString, QString>>& messages,
    const QString& file
);

void send_program_status_notification(
    Logger& logger, EventNotificationOption& settings,
    const QString& program_name,
    const QString& message = "",
    const std::string& stats = "",
    const QImage& image = QImage(), bool keep_file = false
);
void send_program_finished_notification(
    Logger& logger, EventNotificationOption& settings,
    const QString& program_name,
    const QString& message,
    const std::string& stats,
    const QImage& image = QImage(), bool keep_file = false
);
void send_program_error_notification(
    Logger& logger, EventNotificationOption& settings,
    const QString& program_name,
    const QString& message,
    const std::string& stats = "",
    const QImage& image = QImage(), bool keep_file = false
);


}
#endif
