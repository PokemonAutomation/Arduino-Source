/*  Shiny Notification
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonArray>
#include <QJsonObject>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/Tools/DiscordWebHook.h"
#include "CommonFramework/Tools/ProgramNotifications.h"
#include "Pokemon/Resources/Pokemon_PokeballNames.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "Pokemon_Notification.h"

namespace PokemonAutomation{
namespace Pokemon{


void send_encounter_notification(
    Logger& logger,
    const QString& program,
    const std::set<std::string>* slugs,
    const ShinyDetectionResult& result, ScreenshotMode screenshot,
    const StatsTracker* session_stats,
    const EncounterFrequencies* frequencies,
    const StatsTracker* alltime_stats
){
    QColor color;
    switch (result.shiny_type){
    case ShinyType::UNKNOWN_SHINY:
    case ShinyType::STAR_SHINY:
        color = 0xffff99;
        break;
    case ShinyType::SQUARE_SHINY:
        color = 0xb266ff;
        break;
    default:;
    }

    std::vector<std::pair<QString, QString>> embeds;

    if (slugs){
        QString str;
        if (slugs->empty()){
            str = "None - Unable to detect.";
        }else if (slugs->size() == 1){
            str += get_pokemon_name(*slugs->begin()).display_name();
        }else{
            str += "Ambiguous: ";
            bool first = true;
            for (const std::string& slug : *slugs){
                if (!first){
                    str += ", ";
                }
                first = false;
                str += get_pokemon_name(slug).display_name();
            }
        }
        embeds.emplace_back("Species", std::move(str));
    }
    {
        QString str;
        switch (result.shiny_type){
        case ShinyType::UNKNOWN:
            str = "Unknown";
            break;
        case ShinyType::NOT_SHINY:
            str = "Not Shiny";
            break;
        case ShinyType::UNKNOWN_SHINY:
            str = QChar(0x2728) + QString(" Shiny ") + QChar(0x2728);
            break;
        case ShinyType::STAR_SHINY:
            str = QChar(0x2728) + QString(" Star Shiny ") + QChar(0x2728);
            break;
        case ShinyType::SQUARE_SHINY:
            str = QChar(0x2728) + QString(" Square Shiny ") + QChar(0x2728);
            break;
        }
        embeds.emplace_back("Shininess", std::move(str));
    }
    {
        QString session_stats_str;
        if (session_stats){
            session_stats_str += QString::fromStdString(session_stats->to_str());
        }
        if (frequencies && !frequencies->empty()){
            if (!session_stats_str.isEmpty()){
                session_stats_str += "\n";
            }
            session_stats_str += frequencies->dump_sorted_map("");
        }
        if (!session_stats_str.isEmpty()){
            QJsonObject field;
            embeds.emplace_back("Session Stats", std::move(session_stats_str));
        }
    }
    if (alltime_stats){
        QJsonObject field;
        embeds.emplace_back("All Time Stats", QString::fromStdString(alltime_stats->to_str()));
    }

    bool is_shiny = result.shiny_type != ShinyType::NOT_SHINY;
    send_program_notification(
        logger,
        is_shiny, color,
        program,
        "Encounter Notification",
        embeds
    );
    if (is_shiny){
        DiscordWebHook::send_screenshot(logger, result.best_screenshot, screenshot, true);
    }
}

EncounterNotificationSender::EncounterNotificationSender(
    EncounterBotNotificationLevel notification_level,
    std::chrono::seconds period
)
    : m_notification_level(notification_level)
    , m_notification_period(period)
    , m_last_notification(std::chrono::system_clock::now() - period)
{}
void EncounterNotificationSender::send_notification(
    Logger& logger,
    const QString& program,
    const std::set<std::string>* slugs,
    const ShinyDetectionResult& result, ScreenshotMode screenshot,
    const StatsTracker* session_stats,
    const EncounterFrequencies* frequencies,
    const StatsTracker* alltime_stats
){
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

    if (m_notification_level == EncounterBotNotificationLevel::NO_NOTIFICATIONS){
//        cout << "no notifications" << endl;
        return;
    }

    do{
        if (m_notification_level == EncounterBotNotificationLevel::EVERYTHING){
//            cout << "everything" << endl;
            break;
        }

        //  Always notify on shiny.
        if (result.shiny_type != ShinyType::NOT_SHINY){
//            cout << "shiny" << endl;
            break;
        }

        if (m_notification_level == EncounterBotNotificationLevel::SHINY_ONLY){
//            cout << "notify on shiny only" << endl;
            return;
        }

        //  Not enough time elapsed.
        if (now < m_last_notification + m_notification_period){
//            cout << "not enough time" << endl;
            return;
        }

    }while (false);

    m_last_notification = now;

    send_encounter_notification(
        logger,
        program,
        slugs,
        result, screenshot,
        session_stats,
        frequencies,
        alltime_stats
    );

}



void send_catch_notification(
    Logger& logger,
    const QString& program,
    const std::set<std::string>* pokemon_slugs,
    const std::string& ball_slug, int balls_used,
    bool success, bool ping
){
    QColor color = success ? 0x00ff00 : 0xffa500;

    std::vector<std::pair<QString, QString>> embeds;

    if (pokemon_slugs){
        QString str;
        if (pokemon_slugs->empty()){
            str = "None - Unable to detect.";
        }else if (pokemon_slugs->size() == 1){
            str += get_pokemon_name(*pokemon_slugs->begin()).display_name();
        }else{
            str += "Ambiguous: ";
            bool first = true;
            for (const std::string& slug : *pokemon_slugs){
                if (!first){
                    str += ", ";
                }
                first = false;
                str += get_pokemon_name(slug).display_name();
            }
        }
        embeds.emplace_back("Species", std::move(str));
    }
    {
        QString str;
        if (balls_used >= 0){
            str += QString::number(balls_used);
        }
        if (!ball_slug.empty()){
            if (!str.isEmpty()){
                str += " x ";
            }
            str += get_pokeball_name(ball_slug).display_name();
        }
        if (!str.isEmpty()){
            embeds.emplace_back("Balls Used", std::move(str));
        }
    }

    send_program_notification(
        logger,
        ping, color,
        program,
        success ? STRING_POKEMON + " Caught" : "Catch Failed",
        embeds
    );
}





}
}
