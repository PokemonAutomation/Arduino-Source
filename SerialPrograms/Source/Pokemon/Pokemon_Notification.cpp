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
#include "Pokemon/Resources/Pokemon_PokeballNames.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "Pokemon_Notification.h"

namespace PokemonAutomation{
namespace Pokemon{


void send_encounter_notification(
    Logger& logger,
    const QString& program,
    const std::set<std::string>* slugs,
    const ShinyDetectionResult& result, EncounterBotScreenshot screenshot,
    const StatsTracker* session_stats,
    const EncounterFrequencies* frequencies,
    const StatsTracker* alltime_stats
){
    QJsonArray embeds;
    {
        QJsonObject embed;
        {
            QString title = "Encounter Notification";
            const QString& instance_name = PERSISTENT_SETTINGS().INSTANCE_NAME;
            if (!instance_name.isEmpty()){
                title += ": ";
                title += instance_name;
            }
            embed["title"] = title;
        }
        embed["color"] = result.shiny_type != ShinyType::NOT_SHINY ? 0x00ff00 : 0;
        embed["author"] = QJsonObject();
        embed["image"] = QJsonObject();
        embed["thumbnail"] = QJsonObject();
        embed["footer"] = QJsonObject();

        QJsonArray fields;
        {
            QJsonObject field;
            field["name"] = "Program (" + PROGRAM_VERSION + ")";
            field["value"] = program;
            fields.append(field);
        }
        if (slugs){
            QJsonObject field;
            field["name"] = "Species";
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
            field["value"] = str;
            fields.append(field);
        }
        {
            QJsonObject field;
            field["name"] = "Shininess";
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
            field["value"] = str;
            fields.append(field);
        }
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
            field["name"] = "Session Stats";
            field["value"] = session_stats_str;
            fields.append(field);
        }
        if (alltime_stats){
            QJsonObject field;
            field["name"] = "All Time Stats";
            field["value"] = QString::fromStdString(alltime_stats->to_str());
            fields.append(field);
        }
        embed["fields"] = fields;
        embeds.append(embed);
    }
    if (result.shiny_type == ShinyType::NOT_SHINY){
        DiscordWebHook::send_message(false, "", embeds, &logger);
        return;
    }

    DiscordWebHook::send_message(true, "", embeds, &logger);

    if (screenshot == EncounterBotScreenshot::NO_SCREENSHOT){
        return;
    }

    const QImage& image = result.best_screenshot;
    if (image.isNull()){
        logger.log("Shiny screenshot is null.", "red");
        return;
    }

    QString name = QString::fromStdString(now_to_filestring());
    switch (screenshot){
    case EncounterBotScreenshot::NO_SCREENSHOT:
        return;
    case EncounterBotScreenshot::JPG:
        name += ".jpg";
        break;
    case EncounterBotScreenshot::PNG:
        name += ".png";
        break;
    }

    if (!image.save(name)){
        logger.log("Unable to save shiny screenshot to: " + name, "red");
        return;
    }

    logger.log("Saved shiny screenshot to: " + name, "blue");
    DiscordWebHook::send_file(std::move(name), &logger);
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
    const ShinyDetectionResult& result, EncounterBotScreenshot screenshot,
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
    QJsonArray embeds;
    {
        QJsonObject embed;
        {
            QString title = success
                ? STRING_POKEMON + " Caught"
                : "Catch Failed";
            const QString& instance_name = PERSISTENT_SETTINGS().INSTANCE_NAME;
            if (!instance_name.isEmpty()){
                title += ": ";
                title += instance_name;
            }
            embed["title"] = title;
        }
        embed["color"] = success ? 0x00ff00 : 0xffa500;
        embed["author"] = QJsonObject();
        embed["image"] = QJsonObject();
        embed["thumbnail"] = QJsonObject();
        embed["footer"] = QJsonObject();

        QJsonArray fields;
        {
            QJsonObject field;
            field["name"] = "Program (" + PROGRAM_VERSION + ")";
            field["value"] = program;
            fields.append(field);
        }
        if (pokemon_slugs){
            QJsonObject field;
            field["name"] = "Species";
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
            field["value"] = str;
            fields.append(field);
        }
        {
            QJsonObject field;
            field["name"] = "Balls Used";

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
                field["value"] = std::move(str);
                fields.append(field);
            }
        }
        embed["fields"] = fields;
        embeds.append(embed);
    }
    DiscordWebHook::send_message(ping, "", embeds, &logger);
}





}
}
