/*  Program Notifications
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QFile>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "Integrations/DiscordWebhook.h"
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
    LoggerQt& logger,
    Color color, bool should_ping, const std::vector<QString>& tags,
    const ProgramInfo& info,
    const QString& title,
    const std::vector<std::pair<QString, QString>>& messages,
    const ImageAttachment& image
){
    std::shared_ptr<PendingFileSend> file(new PendingFileSend(logger, image));
    bool hasFile = !file->filepath().isEmpty();

    JsonObject embed_sleepy;
    JsonArray embeds;
    {
        JsonObject embed;
        {
            std::string str = title.toStdString();
            const QString& instance_name = GlobalSettings::instance().DISCORD.message.instance_name;
            if (!instance_name.isEmpty()){
                str += ": ";
                str += instance_name.toStdString();
            }
            embed["title"] = str;
        }

        if (color){
            embed["color"] = (int)((uint32_t)color & 0xffffff);
        }

        JsonArray fields;
        {
            JsonObject field;
            field["name"] = PreloadSettings::instance().DEVELOPER_MODE
                ? UTF8_STRING_POKEMON + " Automation (" + UTF8_PROGRAM_VERSION + "-dev)"
                : UTF8_STRING_POKEMON + " Automation (" + UTF8_PROGRAM_VERSION + ")";
            std::string text = info.program_name.toStdString();
            if (info.start_time != WallClock::min()){
                text += "\nUp Time: ";
                text += duration_to_string(
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        current_time() - info.start_time
                    )
                );
            }
            field["value"] = std::move(text);
            fields.push_back(std::move(field));
        }

        for (const auto& item : messages){
            JsonObject field;
            field["name"] = item.first.toStdString();
            field["value"] = item.second.toStdString();
            if (!item.first.isEmpty() && !item.second.isEmpty()){
                fields.push_back(std::move(field));
            }
        }
        embed["fields"] = std::move(fields);

        if (hasFile){
            JsonObject field;
            field["url"] = "attachment://" + file->filename().toStdString();
            embed["image"] = std::move(field);
        }
        embeds.push_back(std::move(embed));
        embed_sleepy = std::move(embed);
    }

    Integration::DiscordWebhook::send_message(logger, should_ping, tags, "", std::move(embeds), hasFile ? file : nullptr);
#ifdef PA_SLEEPY
    Integration::SleepyDiscordRunner::send_message_sleepy(should_ping, tags, "", embed_sleepy, hasFile ? file : nullptr);
#endif
}
void send_program_notification(
    LoggerQt& logger, EventNotificationOption& settings,
    Color color,
    const ProgramInfo& info,
    const QString& title,
    const std::vector<std::pair<QString, QString>>& messages,
    QImage image, bool keep_file
){
    if (!settings.ok_to_send_now(logger)){
        return;
    }
    send_program_notification(
        logger,
        color,
        settings.ping(), settings.tags(),
        info, title,
        messages,
        ImageAttachment(std::move(image), settings.screenshot(), keep_file)
    );
}

void send_program_telemetry(
    LoggerQt& logger, bool is_error, Color color,
    const ProgramInfo& info,
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

    JsonArray embeds;
    {
        JsonObject embed;
        embed["title"] = title.toStdString();
        if (color){
            embed["color"] = (uint32_t)color & 0xffffff;
        }

        JsonArray fields;
        {
            JsonObject field;
            field["name"] = PreloadSettings::instance().DEVELOPER_MODE
                ? UTF8_STRING_POKEMON + " Automation (" + UTF8_PROGRAM_VERSION + "-dev)"
                : UTF8_STRING_POKEMON + " Automation (" + UTF8_PROGRAM_VERSION + ")";
            field["value"] = info.program_name.isEmpty() ? "(unknown)" : info.program_name.toStdString();
            fields.push_back(std::move(field));
        }
        for (const auto& item : messages){
            JsonObject field;
            field["name"] = item.first.toStdString();
            field["value"] = item.second.toStdString();
            if (!item.first.isEmpty() && !item.second.isEmpty()){
                fields.push_back(std::move(field));
            }
        }
        embed["fields"] = std::move(fields);

        if (hasFile){
            JsonObject image;
            image["url"] = "attachment://" + pending->filename().toStdString();
            embed["image"] = std::move(image);
        }
        embeds.push_back(std::move(embed));
    }

    JsonObject jsonContent;
//    jsonContent["content"] = "asdf";
    jsonContent["embeds"] = std::move(embeds);

    QString url = QString::fromStdString(
        is_error
            ? flip(ERROR_REPORTING_URL, sizeof(ERROR_REPORTING_URL))
            : flip(TELEMETRY_URL, sizeof(TELEMETRY_URL))
    );

    using namespace Integration::DiscordWebhook;

    DiscordWebhookSender& sender = DiscordWebhookSender::instance();
    if (hasFile){
        sender.send_json(logger, url, jsonContent, pending);
    }else{
        sender.send_json(logger, url, jsonContent, nullptr);
    }
#endif
}




#if 0
void send_program_status_notification(
    LoggerQt& logger, EventNotificationOption& settings,
    const ProgramInfo& info,
    const QString& message,
    const StatsTracker* current_stats,
    const StatsTracker* historical_stats,
    QImage image, bool keep_file
){
    std::vector<std::pair<QString, QString>> messages{
        {"Message", message},
    };
    if (current_stats){
        messages.emplace_back("Session Stats", QString::fromStdString(current_stats->to_str()));
    }
    if (GlobalSettings::instance().ALL_STATS && historical_stats){
        messages.emplace_back("Historical Stats", QString::fromStdString(historical_stats->to_str()));
    }
    send_program_notification(
        logger, settings,
        Color(), info,
        "Program Status",
        messages,
        std::move(image), keep_file
    );
}
#endif
void send_program_status_notification(
    ProgramEnvironment& env, EventNotificationOption& settings,
    const QString& message,
    QImage image, bool keep_file
){
    const StatsTracker* current_stats = env.current_stats();
    const StatsTracker* historical_stats = env.historical_stats();
    std::vector<std::pair<QString, QString>> messages{
        {"Message", message},
    };
    if (current_stats){
        messages.emplace_back("Session Stats", QString::fromStdString(env.current_stats()->to_str()));
    }
    if (GlobalSettings::instance().ALL_STATS && historical_stats){
        messages.emplace_back("Historical Stats", QString::fromStdString(env.historical_stats()->to_str()));
    }
    send_program_notification(
        env.logger(), settings,
        Color(), env.program_info(),
        "Program Status",
        messages,
        std::move(image), keep_file
    );
}
void send_program_finished_notification(
    LoggerQt& logger, EventNotificationOption& settings,
    const ProgramInfo& info,
    const QString& message,
    const StatsTracker* current_stats,
    const StatsTracker* historical_stats,
    QImage image, bool keep_file
){
    std::vector<std::pair<QString, QString>> messages{
        {"Message", message},
    };
    if (current_stats){
        messages.emplace_back("Session Stats", QString::fromStdString(current_stats->to_str()));
    }
    if (GlobalSettings::instance().ALL_STATS && historical_stats){
        messages.emplace_back("Historical Stats", QString::fromStdString(historical_stats->to_str()));
    }
    send_program_notification(
        logger, settings,
        COLOR_GREEN, info,
        "Program Finished",
        messages,
        std::move(image), keep_file
    );
}
void send_program_finished_notification(
    ProgramEnvironment& env, EventNotificationOption& settings,
    const QString& message,
    QImage image, bool keep_file
){
    const StatsTracker* current_stats = env.current_stats();
    const StatsTracker* historical_stats = env.historical_stats();
    std::vector<std::pair<QString, QString>> messages{
        {"Message", message},
    };
    if (current_stats){
        messages.emplace_back("Session Stats", QString::fromStdString(env.current_stats()->to_str()));
    }
    if (GlobalSettings::instance().ALL_STATS && historical_stats){
        messages.emplace_back("Historical Stats", QString::fromStdString(env.historical_stats()->to_str()));
    }
    send_program_notification(
        env.logger(), settings,
        COLOR_GREEN, env.program_info(),
        "Program Finished",
        messages,
        std::move(image), keep_file
    );
}
void send_program_recoverable_error_notification(
    ProgramEnvironment& env, EventNotificationOption& settings,
    const QString& message,
    QImage image, bool keep_file
){
    const StatsTracker* current_stats = env.current_stats();
    const StatsTracker* historical_stats = env.historical_stats();
    std::vector<std::pair<QString, QString>> messages{
        {"Message", message},
    };
    if (current_stats){
        messages.emplace_back("Session Stats", QString::fromStdString(env.current_stats()->to_str()));
    }
    if (GlobalSettings::instance().ALL_STATS && historical_stats){
        messages.emplace_back("Historical Stats", QString::fromStdString(env.historical_stats()->to_str()));
    }
    send_program_notification(
        env.logger(), settings,
        COLOR_RED, env.program_info(),
        "Program Error (Recoverable)",
        messages,
        std::move(image), keep_file
    );
}
void send_program_fatal_error_notification(
    LoggerQt& logger, EventNotificationOption& settings,
    const ProgramInfo& info,
    const QString& message,
    const StatsTracker* current_stats,
    const StatsTracker* historical_stats,
    QImage image, bool keep_file
){
    std::vector<std::pair<QString, QString>> messages{
        {"Message", message},
    };
    if (current_stats){
        messages.emplace_back("Session Stats", QString::fromStdString(current_stats->to_str()));
    }
    if (GlobalSettings::instance().ALL_STATS && historical_stats){
        messages.emplace_back("Historical Stats", QString::fromStdString(historical_stats->to_str()));
    }
    send_program_notification(
        logger, settings,
        COLOR_RED, info,
        "Program Stopped (Fatal Error)",
        messages,
        std::move(image), keep_file
    );
}




}
