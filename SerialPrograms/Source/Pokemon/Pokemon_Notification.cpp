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

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace Pokemon{


Color shiny_color(ShinyType shiny_type){
    switch (shiny_type){
    case ShinyType::MAYBE_SHINY:
    case ShinyType::UNKNOWN_SHINY:
    case ShinyType::STAR_SHINY:
        return COLOR_STAR_SHINY;
    case ShinyType::SQUARE_SHINY:
        return COLOR_SQUARE_SHINY;
    default:
        return Color();
    }
}
QString shiny_symbol(ShinyType shiny_type){
    switch (shiny_type){
    case ShinyType::MAYBE_SHINY:
        return ":question:";
    case ShinyType::UNKNOWN_SHINY:
    case ShinyType::STAR_SHINY:
        return QChar(0x2728);
    case ShinyType::SQUARE_SHINY:
        return QChar(0x2733);   //  Green square asterisk
    default:
        return "";
    }
}
QString pokemon_to_string(const EncounterResult& pokemon){
    QString str;

    QString symbol = shiny_symbol(pokemon.shininess);
    if (!symbol.isEmpty()){
        str += symbol + " ";
    }

    if (pokemon.slug_candidates.empty()){
        str = "Unable to detect.";
    }else if (pokemon.slug_candidates.size() == 1){
        str += get_pokemon_name(*pokemon.slug_candidates.begin()).display_name();
    }else{
        str += "Ambiguous: ";
        bool first1 = true;
        for (const std::string& slug : pokemon.slug_candidates){
            if (!first1){
                str += ", ";
            }
            first1 = false;
            str += get_pokemon_name(slug).display_name();
        }
    }
    return str;
}



void send_encounter_notification(
    LoggerQt& logger,
    EventNotificationOption& settings_nonshiny,
    EventNotificationOption& settings_shiny,
    const ProgramInfo& info,
    bool enable_names, bool shiny_detected,
    const std::vector<EncounterResult>& results,
    QImage screenshot,
    const StatsTracker* session_stats,
    const EncounterFrequencies* frequencies,
    const StatsTracker* alltime_stats
){
    ShinyType max_shiny_type = ShinyType::UNKNOWN;
    size_t shiny_count = 0;

    QString names;

    bool first = true;
    for (const EncounterResult& result : results){
        if (!first){
            names += "\n";
        }
        first = false;
        names += pokemon_to_string(result);
        max_shiny_type = max_shiny_type < result.shininess ? result.shininess : max_shiny_type;
        shiny_count += is_confirmed_shiny(result.shininess) ? 1 : 0;
    }
    if (max_shiny_type == ShinyType::MAYBE_SHINY){
        max_shiny_type = ShinyType::UNKNOWN_SHINY;
    }
    Color color = shiny_color(max_shiny_type);
    bool has_shiny = is_likely_shiny(max_shiny_type) || shiny_detected;

    QString shinies;
    if (results.size() == 1){
        QString symbol = shiny_symbol(results[0].shininess);
        switch (results[0].shininess){
        case ShinyType::UNKNOWN:
            shinies = "Unknown";
            break;
        case ShinyType::NOT_SHINY:
            shinies = "Not Shiny";
            break;
        case ShinyType::MAYBE_SHINY:
            shinies = "Maybe Shiny";
            break;
        case ShinyType::UNKNOWN_SHINY:
            shinies = symbol + QString(" Shiny ") + symbol;
            break;
        case ShinyType::STAR_SHINY:
            shinies = symbol + QString(" Star Shiny ") + symbol;
            break;
        case ShinyType::SQUARE_SHINY:
            shinies = symbol + QString(" Square Shiny ") + symbol;
            break;
        }
    }else if (!results.empty()){
        QString symbol = shiny_symbol(max_shiny_type);
        switch (shiny_count){
        case 0:
            if (shiny_detected){
                symbol = shiny_symbol(ShinyType::UNKNOWN_SHINY);
                shinies = symbol + " Found Shiny! " + symbol + " (Unable to determine which.)";
            }else{
                shinies = "No Shinies";
            }
            break;
        case 1:
            shinies = symbol + " Found Shiny! " + symbol;
            break;
        default:
            shinies += symbol + QString(" Multiple Shinies! ") + symbol;
            break;
        }
    }

    std::vector<std::pair<QString, QString>> embeds;
    if (enable_names && !names.isEmpty()){
        embeds.emplace_back("Species", std::move(names));
    }
    if (!shinies.isEmpty()){
        embeds.emplace_back("Shininess", std::move(shinies));
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
            embeds.emplace_back("Session Stats", std::move(session_stats_str));
        }
    }
    if (alltime_stats){
        embeds.emplace_back("All Time Stats", QString::fromStdString(alltime_stats->to_str()));
    }

    if (has_shiny){
        send_program_notification(
            logger, settings_shiny,
            color, info,
            "Encounter Notification",
            embeds,
            std::move(screenshot), true
        );
    }else{
        send_program_notification(
            logger, settings_nonshiny,
            color, info,
            "Encounter Notification",
            embeds,
            std::move(screenshot), false
        );
    }
}



void send_catch_notification(
    LoggerQt& logger,
    EventNotificationOption& settings_catch_success,
    EventNotificationOption& settings_catch_failed,
    const ProgramInfo& info,
    const std::set<std::string>* pokemon_slugs,
    const std::string& ball_slug, int balls_used,
    bool success
){
    Color color = success ? COLOR_GREEN : COLOR_ORANGE;

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
            color, info,
            STRING_POKEMON + " Caught",
            embeds
        );
    }else{
        send_program_notification(
            logger, settings_catch_failed,
            color, info,
            "Catch Failed",
            embeds
        );
    }
}




}
}
