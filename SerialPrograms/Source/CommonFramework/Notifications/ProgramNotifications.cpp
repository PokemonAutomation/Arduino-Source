/*  Program Notifications
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QFile>
#include <dpp/DPP_SilenceWarnings.h>
#include <Integrations/DppIntegration/DppClient.h>
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

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


void append_body_fields(JsonArray& fields, const std::vector<std::pair<std::string, std::string>>& messages){
    for (const auto& item : messages){
        JsonObject field;
        field["name"] = item.first;
        field["value"] = item.second;
        if (!item.first.empty() && !item.second.empty()){
            fields.push_back(std::move(field));
        }
    }
}
JsonObject make_credits_field(const ProgramInfo& info){
    JsonObject field;
    field["name"] = "Powered By:";
    std::string text = PreloadSettings::instance().DEVELOPER_MODE
        ? PROGRAM_NAME + " CC " + PROGRAM_VERSION + "-dev"
        : PROGRAM_NAME + " CC " + PROGRAM_VERSION + "";
    if (GlobalSettings::instance().HIDE_NOTIF_DISCORD_LINK){
        text += " ([GitHub](" + PROJECT_GITHUB_URL + "About/))";
    }else{
        text += " ([GitHub](" + PROJECT_GITHUB_URL + "About/)/[Discord](" + DISCORD_LINK_URL + "))";
    }
    field["value"] = std::move(text);
    return field;
}
std::pair<std::string, std::string> make_session_field(
    const ProgramInfo& info,
    const StatsTracker* current_stats, const std::string& current_stats_addendum
){
    const std::string& instance_name = GlobalSettings::instance().DISCORD.message.instance_name;
    std::string name = instance_name.empty()
        ? "Current Session:"
        : "Current Session: (" + instance_name + ")";

    std::string text = info.program_name;
    if (info.start_time != WallClock::min()){
        text += "\nUp Time: ";
        text += duration_to_string(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                current_time() - info.start_time
            )
        );
    }
    if (current_stats){
        text += "\n";
        text += current_stats->to_str(StatsTracker::DISPLAY_ON_SCREEN);
        if (!current_stats_addendum.empty()){
            text += "\n";
            text += current_stats_addendum;
        }
    }

    return {std::move(name), std::move(text)};
}


void send_raw_notification(
    Logger& logger,
    Color color, bool should_ping, const std::vector<std::string>& tags,
    const ProgramInfo& info,
    const std::string& title,
    const std::vector<std::pair<std::string, std::string>>& messages,
    const ImageAttachment& image
){
    std::shared_ptr<PendingFileSend> file(new PendingFileSend(logger, image));
    bool hasFile = !file->filepath().empty();

    JsonObject embed;
    JsonArray embeds;
    {
        embed["title"] = title;

        if (color){
            embed["color"] = (int)((uint32_t)color & 0xffffff);
        }

        JsonArray fields;
//        fields.push_back(make_header_field(info));
        append_body_fields(fields, messages);
        fields.push_back(make_credits_field(info));
        embed["fields"] = std::move(fields);

        if (hasFile){
            JsonObject field;
            field["url"] = "attachment://" + file->filename();
            embed["image"] = std::move(field);
        }
        embeds.push_back(embed.clone());
    }

    Integration::DiscordWebhook::send_embed(
        logger, should_ping, tags,
        std::move(embeds),
        hasFile ? file : nullptr
    );

#ifdef PA_SLEEPY
    if (GlobalSettings::instance().DISCORD.integration.library0 == Integration::DiscordIntegrationSettingsOption::Library::SleepyDiscord){
        Integration::SleepyDiscordRunner::send_embed_sleepy(
            should_ping, tags, std::move(embed),
            hasFile ? file : nullptr
        );
    }
#endif

#ifdef PA_DPP
    if (GlobalSettings::instance().DISCORD.integration.library0 == Integration::DiscordIntegrationSettingsOption::Library::DPP){
        Integration::DppClient::Client::instance().send_embed_dpp(
            should_ping, color, tags, std::move(embed),
            hasFile ? file : nullptr
        );
    }
#endif
}
void send_raw_notification(
    Logger& logger,
    Color color, bool should_ping, const std::vector<std::string>& tags,
    const ProgramInfo& info,
    const std::string& title,
    const std::vector<std::pair<std::string, std::string>>& messages,
    const std::string& filepath
){
    std::shared_ptr<PendingFileSend> file(new PendingFileSend(filepath, true));
    bool hasFile = !file->filepath().empty();

    JsonObject embed;
    JsonArray embeds;
    {
        embed["title"] = title;

        if (color){
            embed["color"] = (int)((uint32_t)color & 0xffffff);
        }

        JsonArray fields;
//        fields.push_back(make_header_field(info));
        append_body_fields(fields, messages);
        fields.push_back(make_credits_field(info));
        embed["fields"] = std::move(fields);

        embeds.push_back(embed.clone());
    }

    Integration::DiscordWebhook::send_embed(
        logger, should_ping, tags,
        std::move(embeds),
        hasFile ? file : nullptr
    );

#ifdef PA_SLEEPY
    if (GlobalSettings::instance().DISCORD.integration.library0 == Integration::DiscordIntegrationSettingsOption::Library::SleepyDiscord){
        Integration::SleepyDiscordRunner::send_embed_sleepy(
            should_ping, tags, std::move(embed),
            hasFile ? file : nullptr
        );
    }
#endif

#ifdef PA_DPP
    if (GlobalSettings::instance().DISCORD.integration.library0 == Integration::DiscordIntegrationSettingsOption::Library::DPP){
        Integration::DppClient::Client::instance().send_embed_dpp(
            should_ping, color, tags, std::move(embed),
            hasFile ? file : nullptr
        );
    }
#endif
}



void send_raw_program_notification_with_file(
    Logger& logger, EventNotificationOption& settings,
    Color color,
    const ProgramInfo& info,
    const std::string& title,
    const std::vector<std::pair<std::string, std::string>>& messages,
    const std::string& filepath
){
    if (!settings.ok_to_send_now(logger)){
        return;
    }
    send_raw_notification(
        logger,
        color,
        settings.ping(), settings.tags(),
        info, title,
        messages,
        filepath
    );
}
void send_raw_program_notification(
    Logger& logger, EventNotificationOption& settings,
    Color color,
    const ProgramInfo& info,
    const std::string& title,
    const std::vector<std::pair<std::string, std::string>>& messages,
    const ImageViewRGB32& image, bool keep_file
){
    if (!settings.ok_to_send_now(logger)){
        return;
    }
    send_raw_notification(
        logger,
        color,
        settings.ping(), settings.tags(),
        info, title,
        messages,
        ImageAttachment(image, settings.screenshot(), keep_file)
    );
}



void send_program_notification_with_file(
    ProgramEnvironment& env, EventNotificationOption& settings,
    Color color,
    const std::string& title,
    std::vector<std::pair<std::string, std::string>> messages,
    const std::string& current_stats_addendum,
    const std::string& filepath
){
    if (!settings.ok_to_send_now(env.logger())){
        return;
    }
#if 1
    messages.emplace_back(
        make_session_field(
            env.program_info(),
            env.current_stats(),
            current_stats_addendum
        )
    );
#else
    const StatsTracker* current_stats = env.current_stats();
    if (current_stats){
        std::string str = env.current_stats()->to_str();
        if (!current_stats_addendum.empty()){
            str += "\n";
            str += current_stats_addendum;
        }
        messages.emplace_back("Session Stats:", std::move(str));
    }
#endif
    const StatsTracker* historical_stats = env.historical_stats();
    if (GlobalSettings::instance().ALL_STATS && historical_stats){
        messages.emplace_back(
            "Historical Stats:",
            env.historical_stats()->to_str(StatsTracker::DISPLAY_ON_SCREEN)
        );
    }
    send_raw_notification(
        env.logger(),
        color,
        settings.ping(), settings.tags(),
        env.program_info(),
        title,
        messages,
        filepath
    );
}
void send_program_notification(
    ProgramEnvironment& env, EventNotificationOption& settings,
    Color color,
    const std::string& title,
    std::vector<std::pair<std::string, std::string>> messages,
    const std::string& current_stats_addendum,
    const ImageViewRGB32& image, bool keep_file
){
    if (!settings.ok_to_send_now(env.logger())){
        return;
    }
#if 1
    messages.emplace_back(
        make_session_field(
            env.program_info(),
            env.current_stats(),
            current_stats_addendum
        )
    );
#else
    const StatsTracker* current_stats = env.current_stats();
    if (current_stats){
        std::string str = env.current_stats()->to_str();
        if (!current_stats_addendum.empty()){
            str += "\n";
            str += current_stats_addendum;
        }
        messages.emplace_back("Session Stats:", std::move(str));
    }
#endif
    const StatsTracker* historical_stats = env.historical_stats();
    if (GlobalSettings::instance().ALL_STATS && historical_stats){
        messages.emplace_back(
            "Historical Stats:",
            env.historical_stats()->to_str(StatsTracker::DISPLAY_ON_SCREEN)
        );
    }
    send_raw_notification(
        env.logger(),
        color,
        settings.ping(), settings.tags(),
        env.program_info(),
        title,
        messages,
        ImageAttachment(image, settings.screenshot(), keep_file)
    );
}





void send_program_status_notification(
    ProgramEnvironment& env, EventNotificationOption& settings,
    const std::string& message,
    const ImageViewRGB32& image, bool keep_file
){
    send_program_notification(
        env, settings,
        Color(),
        "Program Status",
        {{"Message:", message}}, "",
        image, keep_file
    );
}
void send_program_finished_notification(
    ProgramEnvironment& env, EventNotificationOption& settings,
    const std::string& message,
    const ImageViewRGB32& image, bool keep_file
){
    send_program_notification(
        env, settings,
        COLOR_GREEN,
        "Program Finished",
        {{"Message:", message}}, "",
        image, keep_file
    );
}
void send_program_recoverable_error_notification(
    ProgramEnvironment& env, EventNotificationOption& settings,
    const std::string& message,
    const ImageViewRGB32& image, bool keep_file
){
    send_program_notification(
        env, settings,
        COLOR_RED,
        "Program Error (Recoverable)",
        {{"Message:", message}}, "",
        image, keep_file
    );
}
void send_program_fatal_error_notification(
    ProgramEnvironment& env, EventNotificationOption& settings,
    const std::string& message,
    const ImageViewRGB32& image, bool keep_file
){
    send_program_notification(
        env, settings,
        COLOR_RED,
        "Program Stopped (Fatal Error)",
        {{"Message:", message}}, "",
        image, keep_file
    );
}












void send_program_telemetry(
    Logger& logger, bool is_error, Color color,
    const ProgramInfo& info,
    const std::string& title,
    const std::vector<std::pair<std::string, std::string>>& messages,
    const std::string& file
){
#ifdef PA_OFFICIAL
    if (!GlobalSettings::instance().SEND_ERROR_REPORTS){
        return;
    }

    //  Rate limit the telemetry to 10/hour.
    static std::mutex lock;
    static std::set<WallClock> sends;
    {
        std::lock_guard<std::mutex> lg(lock);

        WallClock now = current_time();
        WallClock threshold = now - std::chrono::minutes(1);
        while (!sends.empty()){
            auto iter = sends.begin();
            if (*iter > threshold){
                break;
            }
            sends.erase(iter);
        }
        if (sends.size() >= 10){
            logger.log("Error report suppressed due to rate limit.", COLOR_RED);
            return;
        }
        sends.insert(now);
    }


    bool hasFile = !file.empty();
    std::shared_ptr<PendingFileSend> pending = !hasFile
            ? nullptr
            : std::shared_ptr<PendingFileSend>(new PendingFileSend(file, GlobalSettings::instance().SAVE_DEBUG_IMAGES));

    JsonArray embeds;
    {
        JsonObject embed;
        embed["title"] = title;
        if (color){
            embed["color"] = (uint32_t)color & 0xffffff;
        }

        JsonArray fields;
        {
            JsonObject field;
            field["name"] = PreloadSettings::instance().DEVELOPER_MODE
                ? PROGRAM_NAME + " (" + PROGRAM_VERSION + "-dev)"
                : PROGRAM_NAME + " (" + PROGRAM_VERSION + ")";
            field["value"] = info.program_name.empty() ? "(unknown)" : info.program_name;
            fields.push_back(std::move(field));
        }
        for (const auto& item : messages){
            JsonObject field;
            field["name"] = item.first;
            field["value"] = item.second;
            if (!item.first.empty() && !item.second.empty()){
                fields.push_back(std::move(field));
            }
        }
        embed["fields"] = std::move(fields);

        if (hasFile){
            JsonObject image;
            image["url"] = "attachment://" + pending->filename();
            embed["image"] = std::move(image);
        }
        embeds.push_back(std::move(embed));
    }

    JsonObject jsonContent;
//    jsonContent["content"] = "asdf";
    jsonContent["embeds"] = std::move(embeds);

    std::string url = is_error
            ? flip(ERROR_REPORTING_URL, sizeof(ERROR_REPORTING_URL))
            : flip(TELEMETRY_URL, sizeof(TELEMETRY_URL));

    using namespace Integration::DiscordWebhook;

    DiscordWebhookSender& sender = DiscordWebhookSender::instance();
    if (hasFile){
        sender.send_json(logger, QString::fromStdString(url), std::chrono::milliseconds(0), jsonContent, pending);
    }else{
        sender.send_json(logger, QString::fromStdString(url), std::chrono::milliseconds(0), jsonContent, nullptr);
    }
#endif
}



}
