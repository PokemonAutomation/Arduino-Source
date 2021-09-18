/*  Program Notifications
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonArray>
#include <QJsonObject>
#include "CommonFramework/Globals.h"
#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/Tools/DiscordWebHook.h"
#include "ProgramNotifications.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

void send_program_notification(
    Logger& logger,
    bool should_ping, QColor color,
    const QString& program_name,
    const QString& title,
    const std::vector<std::pair<QString, QString>>& messages
){
    QJsonArray embeds;
    {
        QJsonObject embed;
        {
            QString str = title;
            const QString& instance_name = PERSISTENT_SETTINGS().discord_settings.instance_name;
            if (!instance_name.isEmpty()){
                str += ": ";
                str += instance_name;
            }
            embed["title"] = str;
        }
        if (color.isValid()){
            embed["color"] = (int)(color.rgb() & 0xffffff);
        }
        embed["author"] = QJsonObject();
        embed["image"] = QJsonObject();
        embed["thumbnail"] = QJsonObject();
        embed["footer"] = QJsonObject();

        QJsonArray fields;
        {
            QJsonObject field;
            field["name"] = PERSISTENT_SETTINGS().developer_mode
                ? STRING_POKEMON + " Automation (" + PROGRAM_VERSION + "-dev)"
                : STRING_POKEMON + " Automation (" + PROGRAM_VERSION + ")";
            field["value"] = program_name;
            fields.append(field);
        }
        for (const auto& item : messages){
            QJsonObject field;
            field["name"] = item.first;
            field["value"] = item.second;
            if (!item.first.isEmpty() && !item.second.isEmpty()){
                fields.append(field);
            }
        }
        embed["fields"] = fields;
        embeds.append(embed);
    }
    DiscordWebHook::send_message(logger, should_ping, "", embeds);
}




void send_program_status_notification(
    Logger& logger, bool should_ping,
    const QString& program_name,
    const QString& message,
    const std::string& stats
){
    send_program_notification(
        logger, should_ping, QColor(),
        program_name,
        "Program Status",
        {
            {"Message", message},
            {"Session Stats", QString::fromStdString(stats)},
        }
    );
}

void send_program_finished_notification(
    Logger& logger, bool should_ping,
    const QString& program_name,
    const QString& message,
    const std::string& stats
){
    send_program_notification(
        logger, should_ping, Qt::green,
        program_name,
        "Program Finished",
        {
            {"Message", message},
            {"Session Stats", QString::fromStdString(stats)},
        }
    );
}



void send_program_error_notification(
    Logger& logger,
    const QString& program_name,
    const QString& message,
    const std::string& stats
){
    send_program_notification(
        logger, true, Qt::red,
        program_name,
        "Program Stopped (Error)",
        {
            {"Message", message},
            {"Session Stats", QString::fromStdString(stats)},
        }
    );
#if 0
    QJsonArray embeds;
    {
        QJsonObject embed;
        {
            QString title = "Program Stopped (Error)";
            const QString& instance_name = PERSISTENT_SETTINGS().INSTANCE_NAME;
            if (!instance_name.isEmpty()){
                title += ": ";
                title += instance_name;
            }
            embed["title"] = title;
        }
        embed["color"] = 0xff0000;
        embed["author"] = QJsonObject();
        embed["image"] = QJsonObject();
        embed["thumbnail"] = QJsonObject();
        embed["footer"] = QJsonObject();

        QJsonArray fields;
        {
            QJsonObject field;
            field["name"] = "Program (" + PROGRAM_VERSION + ")";
            field["value"] = program_name;
            fields.append(field);
        }
        {
            QJsonObject field;
            field["name"] = "Error Message";
            field["value"] = message;
            fields.append(field);
        }
        embed["fields"] = fields;
        embeds.append(embed);
    }
    DiscordWebHook::send_message(logger, true, "", embeds);
#endif
}




}
