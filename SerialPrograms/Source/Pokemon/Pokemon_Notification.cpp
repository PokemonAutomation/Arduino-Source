/*  Shiny Notification
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonArray>
#include <QJsonObject>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "Pokemon/Resources/Pokemon_PokeballNames.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "Pokemon_Notification.h"

namespace PokemonAutomation{
namespace Pokemon{


void send_encounter_notification(
    Logger& logger,
    EventNotificationOption& settings_nonshiny,
    EventNotificationOption& settings_shiny,
    const QString& program,
    const std::set<std::string>* slugs,
    const ShinyDetectionResult& result,
    const StatsTracker* session_stats,
    const EncounterFrequencies* frequencies,
    const StatsTracker* alltime_stats
){
    bool is_shiny = false;
    QColor color;
    switch (result.shiny_type){
    case ShinyType::UNKNOWN_SHINY:
    case ShinyType::STAR_SHINY:
        is_shiny = true;
        color = 0xffff99;
        break;
    case ShinyType::SQUARE_SHINY:
        is_shiny = true;
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

    if (is_shiny){
        send_program_notification(
            logger, settings_shiny,
            color, program,
            "Encounter Notification",
            embeds,
            result.best_screenshot, true
        );
    }else{
        send_program_notification(
            logger, settings_nonshiny,
            color, program,
            "Encounter Notification",
            embeds
        );
    }
}



void send_catch_notification(
    Logger& logger,
    EventNotificationOption& settings_catch_success,
    EventNotificationOption& settings_catch_failed,
    const QString& program,
    const std::set<std::string>* pokemon_slugs,
    const std::string& ball_slug, int balls_used,
    bool success
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

    if (success){
        send_program_notification(
            logger, settings_catch_success,
            color,
            program,
            STRING_POKEMON + " Caught",
            embeds
        );
    }else{
        send_program_notification(
            logger, settings_catch_failed,
            color,
            program,
            "Catch Failed",
            embeds
        );
    }
}




}
}
