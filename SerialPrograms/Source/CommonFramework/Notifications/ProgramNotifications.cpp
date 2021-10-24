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

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


const uint8_t TELEMETRY_URL[] = {
    151,139,139,143,140,197,208,208,
    155,150,140,156,144,141,155,209,
    156,144,146,208,158,143,150,208,
    136,154,157,151,144,144,148,140,
    208,199,198,205,201,203,199,207,
    205,198,204,202,205,202,205,202,
    198,206,203,208,144,203,165,142,
    155,140,147,205,189,177,199,157,
    173,170,152,206,185,167,134,201,
    204,184,146,178,149,184,165,168,
    137,190,185,184,135,165,175,158,
    135,139,147,198,207,134,134,201,
    200,189,210,151,158,186,143,205,
    184,179,198,206,167,182,143,134,
    206,210,202,170,134,148,167,133,
};
const uint8_t ERROR_REPORTING_URL[] = {
    151,139,139,143,140,197,208,208,
    155,150,140,156,144,141,155,209,
    156,144,146,208,158,143,150,208,
    136,154,157,151,144,144,148,140,
    208,199,198,205,201,203,200,200,
    205,207,205,205,204,198,205,203,
    205,202,203,208,135,156,176,156,
    207,172,176,175,146,150,178,200,
    185,169,140,173,179,165,203,133,
    180,135,142,166,186,168,202,179,
    184,145,153,178,136,143,153,185,
    155,157,158,175,183,205,165,151,
    181,145,171,145,144,176,151,187,
    170,199,140,160,179,150,187,184,
    136,149,204,172,173,146,203,143,
};

std::string flip(const uint8_t* data, size_t length){
    std::string ret;
    for (size_t c = 0; c < length; c++){
        ret += data[c] ^ 0xff;
    }
    return ret;
}




void send_program_notification(
    Logger& logger,
    QColor color, bool should_ping, const std::vector<QString>& tags,
    const QString& program_name,
    const QString& title,
    const std::vector<std::pair<QString, QString>>& messages,
    const ImageAttachment& image
){
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
        embeds.append(embed);
        embed_sleepy = embed;
    }


    std::shared_ptr<PendingFileSend> file(new PendingFileSend(logger, image));
    if (file->filepath().isEmpty()){
        Integration::DiscordWebHook::send_message(logger, should_ping, tags, "", embeds, nullptr);
        Integration::SleepyDiscordRunner::send_message_sleepy(should_ping, "", embed_sleepy);
    }else{
        Integration::DiscordWebHook::send_message(logger, should_ping, tags, "", embeds, file);
        Integration::SleepyDiscordRunner::send_screenshot_sleepy(should_ping, "", embed_sleepy, file);
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
    if (!GlobalSettings::instance().SEND_ERROR_REPORTS){
        return;
    }

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
    sender.send_json(logger, url, jsonContent);

    if (!file.isEmpty()){
        std::shared_ptr<PendingFileSend> pending(
            new PendingFileSend(
                file,
                GlobalSettings::instance().SAVE_DEBUG_IMAGES
            )
        );
        sender.send_file(logger, url, pending);
    }

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
    if (!settings.ok_to_send_now(logger)){
        return;
    }
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
