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
#include "CommonFramework/Logging/LoggerQt.h"
#include "ProgramInfo.h"
#include "MessageAttachment.h"
#include "EventNotificationOption.h"

namespace PokemonAutomation{


void send_program_notification(
    LoggerQt& logger, EventNotificationOption& settings,
    Color color,
    const ProgramInfo& info,
    const QString& title,
    const std::vector<std::pair<QString, QString>>& messages,
    const QImage& image = QImage(), bool keep_file = false
);

void send_program_telemetry(
    LoggerQt& logger, bool is_error, Color color,
    const ProgramInfo& info,
    const QString& title,
    const std::vector<std::pair<QString, QString>>& messages,
    const QString& file
);

void send_program_status_notification(
    LoggerQt& logger, EventNotificationOption& settings,
    const ProgramInfo& info,
    const QString& message = "",
    const std::string& stats = "",
    const QImage& image = QImage(), bool keep_file = false
);
void send_program_finished_notification(
    LoggerQt& logger, EventNotificationOption& settings,
    const ProgramInfo& info,
    const QString& message,
    const std::string& stats,
    const QImage& image = QImage(), bool keep_file = false
);
void send_program_recoverable_error_notification(
    LoggerQt& logger, EventNotificationOption& settings,
    const ProgramInfo& info,
    const QString& message,
    const std::string& stats = "",
    const QImage& image = QImage(), bool keep_file = false
);
void send_program_fatal_error_notification(
    LoggerQt& logger, EventNotificationOption& settings,
    const ProgramInfo& info,
    const QString& message,
    const std::string& stats = "",
    const QImage& image = QImage(), bool keep_file = false
);


}
#endif
