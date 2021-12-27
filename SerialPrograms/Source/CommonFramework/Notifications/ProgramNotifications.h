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


struct ProgramInfo{
    std::string program_id;
    QString program_name;
    std::chrono::system_clock::time_point start_time;

    ProgramInfo(
        const std::string& module = "",
        std::chrono::system_clock::time_point p_start_time = std::chrono::system_clock::time_point::min()
    )
        : program_name(QString::fromStdString(module))
        , start_time(p_start_time)
    {}
    ProgramInfo(
        std::string p_program_id,
        QString category, QString display_name,
        std::chrono::system_clock::time_point p_start_time = std::chrono::system_clock::time_point::min()
    )
        : program_id(std::move(p_program_id))
        , program_name((category.isEmpty() ? "" : category + ": ") + display_name)
        , start_time(p_start_time)
    {}
};


void send_program_notification(
    Logger& logger, EventNotificationOption& settings,
    QColor color,
    const ProgramInfo& info,
    const QString& title,
    const std::vector<std::pair<QString, QString>>& messages,
    const QImage& image = QImage(), bool keep_file = false
);

void send_program_telemetry(
    Logger& logger, bool is_error, QColor color,
    const ProgramInfo& info,
    const QString& title,
    const std::vector<std::pair<QString, QString>>& messages,
    const QString& file
);

void send_program_status_notification(
    Logger& logger, EventNotificationOption& settings,
    const ProgramInfo& info,
    const QString& message = "",
    const std::string& stats = "",
    const QImage& image = QImage(), bool keep_file = false
);
void send_program_finished_notification(
    Logger& logger, EventNotificationOption& settings,
    const ProgramInfo& info,
    const QString& message,
    const std::string& stats,
    const QImage& image = QImage(), bool keep_file = false
);
void send_program_recoverable_error_notification(
    Logger& logger, EventNotificationOption& settings,
    const ProgramInfo& info,
    const QString& message,
    const std::string& stats = "",
    const QImage& image = QImage(), bool keep_file = false
);
void send_program_fatal_error_notification(
    Logger& logger, EventNotificationOption& settings,
    const ProgramInfo& info,
    const QString& message,
    const std::string& stats = "",
    const QImage& image = QImage(), bool keep_file = false
);


}
#endif
