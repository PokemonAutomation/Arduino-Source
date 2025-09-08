/*  Program Notifications
 *
 *  From: https://github.com/PokemonAutomation/
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
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "Integrations/DiscordSettingsOption.h"
#include "Integrations/DiscordWebhook.h"
#include "Integrations/DppIntegration/DppClient.h"
#include "ProgramNotifications.h"

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
        text += " ([GitHub](" + GITHUB_LINK_URL + "))";
    }else{
        text += " ([GitHub](" + GITHUB_LINK_URL + ")/[Discord](" + DISCORD_LINK_URL_EMBED + "))";
    }
    field["value"] = std::move(text);
    return field;
}
std::pair<std::string, std::string> make_session_field(
    const ProgramInfo& info,
    const StatsTracker* current_stats, const std::string& current_stats_addendum
){
    const std::string& instance_name = GlobalSettings::instance().DISCORD->message.instance_name;
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
    std::shared_ptr<PendingFileSend> file = std::make_shared<PendingFileSend>(logger, image);
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

#ifdef PA_DPP
    Integration::DppClient::Client::instance().send_embed_dpp(
        should_ping, color, tags, std::move(embed),
        hasFile ? file : nullptr
    );
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
    std::shared_ptr<PendingFileSend> file = std::make_shared<PendingFileSend>(filepath, true);
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

#ifdef PA_DPP
    Integration::DppClient::Client::instance().send_embed_dpp(
        should_ping, color, tags, std::move(embed),
        hasFile ? file : nullptr
    );
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
bool send_program_notification(
    ProgramEnvironment& env, EventNotificationOption& settings,
    Color color,
    const std::string& title,
    std::vector<std::pair<std::string, std::string>> messages,
    const std::string& current_stats_addendum,
    const ImageViewRGB32& image, bool keep_file
){
    if (!settings.ok_to_send_now(env.logger())){
        return false;
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

    return true;
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








}
