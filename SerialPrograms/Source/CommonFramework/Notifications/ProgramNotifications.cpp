/*  Program Notifications
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include "CommonFramework/Globals.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "Integrations/DiscordWebHook.h"
#include "Integrations/SleepyDiscordRunner.h"
#include "ProgramNotifications.h"

#ifdef PA_OFFICIAL
#include "../Internal/SerialPrograms/TelemetryURLs.h"
#endif

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


void send_program_notification(
    Logger& logger,
    QColor color, bool should_ping, const std::vector<QString>& tags,
    const QString& program_name,
    const QString& title,
    const std::vector<std::pair<QString, QString>>& messages,
    const ImageAttachment& image
){
    std::shared_ptr<PendingFileSend> file(new PendingFileSend(logger, image));
    bool hasFile = !file->filepath().isEmpty();

    QJsonObject embed_sleepy;
    QJsonArray embeds;
    {
        QJsonObject embed;
        {
            QString str = title;
            const QString& instance_name = GlobalSettings::instance().DISCORD.message.instance_name;
            if (!instance_name.isEmpty()){
                str += ": ";
                str += instance_name;
            }
            embed["title"] = str;
        }

        if (color.isValid()){
            embed["color"] = (int)(color.rgb() & 0xffffff);
        }

        QJsonArray fields;
        {
            QJsonObject field;
            field["name"] = GlobalSettings::instance().DEVELOPER_MODE
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

        if (hasFile){
            QJsonObject image;
            {
                image["url"] = "attachment://" + file->filename();
            }
            embed["image"] = image;
        }
        embeds.append(embed);
        embed_sleepy = embed;
    }

    if (!hasFile){
        Integration::DiscordWebHook::send_message(logger, should_ping, tags, "", embeds, nullptr);
#ifdef PA_SLEEPY
        Integration::SleepyDiscordRunner::send_message_sleepy(should_ping, "", embed_sleepy);
#endif
    }else{
        Integration::DiscordWebHook::send_message(logger, should_ping, tags, "", embeds, file);
#ifdef PA_SLEEPY
        Integration::SleepyDiscordRunner::send_screenshot_sleepy(should_ping, "", embed_sleepy, file);
#endif
    }
}
void send_program_notification(
    Logger& logger, EventNotificationOption& settings,
    QColor color,
    const QString& program_name,
    const QString& title,
    const std::vector<std::pair<QString, QString>>& messages,
    const QImage& image, bool keep_file
){
    if (!settings.ok_to_send_now(logger)){
        return;
    }
    send_program_notification(
        logger,
        color,
        settings.ping(), settings.tags(),
        program_name, title,
        messages,
        ImageAttachment(image, settings.screenshot(), keep_file)
    );
}

void send_program_telemetry(
    Logger& logger, bool is_error, QColor color,
    const QString& program_name,
    const QString& title,
    const std::vector<std::pair<QString, QString>>& messages,
    const QString& file
){
#ifdef PA_OFFICIAL
    if (!GlobalSettings::instance().SEND_ERROR_REPORTS){
        return;
    }

    bool hasFile = !file.isEmpty();
    std::shared_ptr<PendingFileSend> pending = !hasFile
            ? nullptr
            : std::shared_ptr<PendingFileSend>(new PendingFileSend(file, GlobalSettings::instance().SAVE_DEBUG_IMAGES));

    QJsonArray embeds;
    {
        QJsonObject embed;
        embed["title"] = title;
        if (color.isValid()){
            embed["color"] = (int)(color.rgb() & 0xffffff);
        }

        QJsonArray fields;
        {
            QJsonObject field;
            field["name"] = GlobalSettings::instance().DEVELOPER_MODE
                ? STRING_POKEMON + " Automation (" + PROGRAM_VERSION + "-dev)"
                : STRING_POKEMON + " Automation (" + PROGRAM_VERSION + ")";
            field["value"] = program_name.isEmpty() ? "(unknown)" : program_name;
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

        if (hasFile){
            QJsonObject image;
            {
                image["url"] = "attachment://" + pending->filename();
            }
            embed["image"] = image;
        }
        embeds.append(embed);
    }

    QJsonObject jsonContent;
//    jsonContent["content"] = "asdf";
    jsonContent["embeds"] = embeds;

    QString url = QString::fromStdString(
        is_error
            ? flip(ERROR_REPORTING_URL, sizeof(ERROR_REPORTING_URL))
            : flip(TELEMETRY_URL, sizeof(TELEMETRY_URL))
    );

    using namespace Integration::DiscordWebHook;

    DiscordWebHookSender& sender = DiscordWebHookSender::instance();
    if (hasFile){
        sender.send_json(logger, url, jsonContent, pending);
    }else{
        sender.send_json(logger, url, jsonContent, nullptr);
    }
#endif
}




void send_program_status_notification(
    Logger& logger, EventNotificationOption& settings,
    const QString& program_name,
    const QString& message,
    const std::string& stats,
    const QImage& image, bool keep_file
){
    send_program_notification(
        logger, settings,
        QColor(),
        program_name,
        "Program Status",
        {
            {"Message", message},
            {"Session Stats", QString::fromStdString(stats)},
        },
        image, keep_file
    );
}
void send_program_finished_notification(
    Logger& logger, EventNotificationOption& settings,
    const QString& program_name,
    const QString& message,
    const std::string& stats,
    const QImage& image, bool keep_file
){
    send_program_notification(
        logger, settings,
        Qt::green, program_name,
        "Program Finished",
        {
            {"Message", message},
            {"Session Stats", QString::fromStdString(stats)},
        },
        image, keep_file
    );
}
void send_program_error_notification(
    Logger& logger, EventNotificationOption& settings,
    const QString& program_name,
    const QString& message,
    const std::string& stats,
    const QImage& image, bool keep_file
){
    send_program_notification(
        logger, settings,
        Qt::red, program_name,
        "Program Stopped (Error)",
        {
            {"Message", message},
            {"Session Stats", QString::fromStdString(stats)},
        },
        image, keep_file
    );
}




}
