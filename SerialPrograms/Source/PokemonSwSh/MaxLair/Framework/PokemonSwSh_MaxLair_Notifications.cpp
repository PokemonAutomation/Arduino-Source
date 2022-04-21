/*  Max Lair Notifications
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonArray>
#include <QJsonObject>
#include "CommonFramework/Globals.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "Pokemon/Pokemon_Notification.h"
#include "PokemonSwSh/Resources/PokemonSwSh_MaxLairDatabase.h"
#include "PokemonSwSh_MaxLair_Notifications.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{
using namespace Pokemon;


QString pokemon_name(const std::string& slug, const QString& empty_string){
    if (slug.empty()){
        return empty_string;
    }
    return get_pokemon_name(get_maxlair_slugs(slug).name_slug).display_name();
}


void send_status_notification(
    MultiSwitchProgramEnvironment& env,
    AdventureRuntime& runtime
){
    QString status_str = "Status:\n";
    std::vector<std::pair<QString, QString>> embeds;
    {
        QString str = pokemon_name(runtime.last_boss, "N/A");
        status_str += "Last Boss: " + str + "\n";
        embeds.emplace_back("Last Boss", std::move(str));
    }
    {
        QString str = QString::fromStdString(runtime.path_stats.to_str());
        status_str += "Current Path: " + str + "\n";
        embeds.emplace_back("Current Path", std::move(str));
    }
    {
        QString str = QString::fromStdString(runtime.session_stats.to_str());
        status_str += "Session Stats: " + str + "\n";
        embeds.emplace_back("Session Stats", std::move(str));
    }
    for (size_t c = 0; c < env.consoles.size(); c++){
        const ConsoleRuntime& stats = runtime.consoles[c];
        QString label = "Console " + QString::number(c);
        std::string str;
        str += "Ore: " + stats.ore.to_str();
        str += " - Normal Balls: " + stats.normal_balls.to_str();
        str += " - Boss Balls: " + stats.boss_balls.to_str();
        status_str += label + " - " + QString::fromStdString(str) + "\n";
        embeds.emplace_back(std::move(label), QString::fromStdString(str));
    }
    env.log(status_str);
    send_program_notification(
        env.logger(), runtime.notification_status,
        Color(), env.program_info(),
        "Max Lair Status Update",
        embeds
    );
}

void send_raid_notification(
    const ProgramInfo& program_info,
    ConsoleHandle& console,
    AutoHostNotificationOption& settings,
    bool has_code, uint8_t code[8],
    const std::string& slug,
    const PathStats& path_stats,
    const StatsTracker& session_stats
){
    if (!settings.enabled()){
        return;
    }

    QImage screen = console.video().snapshot();

    std::vector<std::pair<QString, QString>> embeds;

    QString description = settings.DESCRIPTION;
    if (!description.isEmpty()){
        embeds.emplace_back("Description", description);
    }

    embeds.emplace_back(
        "Current " + STRING_POKEMON,
        pokemon_name(slug, "Unable to detect.")
    );

    {
        std::string code_str;
        if (has_code){
            size_t c = 0;
            for (; c < 4; c++){
                code_str += code[c] + '0';
            }
            code_str += " ";
            for (; c < 8; c++){
                code_str += code[c] + '0';
            }
        }else{
            code_str += "None";
        }
        embeds.emplace_back("Raid Code", QString::fromStdString(code_str));
    }

    if (path_stats.runs() > 0){
        embeds.emplace_back("Current Path", QString::fromStdString(path_stats.to_str()));
    }
    embeds.emplace_back("Session Stats", QString::fromStdString(session_stats.to_str()));

    send_program_notification(
        console, settings.NOTIFICATION,
        Color(),
        program_info,
        "Max Lair Notification",
        embeds,
        screen, false
    );

}


void send_shiny_notification(
    LoggerQt& logger, EventNotificationOption& settings,
    const ProgramInfo& program_info,
    size_t console_index, size_t shinies,
    const std::set<std::string>* slugs,
    const PathStats& path_stats,
    const Stats& session_stats,
    const QImage& image
){
    std::vector<std::pair<QString, QString>> embeds;
    embeds.emplace_back(
        "Adventure Result",
        shinies == 1
            ? "Shiny on console " + QString::number(console_index) + "!"
            : QString::number(shinies) + " shinies on console " + QString::number(console_index) + "!"
    );
    if (slugs){
        QString str;
        if (slugs->empty()){
            str = "None - Unable to detect.";
        }else if (slugs->size() == 1){
//                str += get_pokemon_name(*slugs->begin()).display_name();
            const PokemonNames* names = get_pokemon_name_nothrow(*slugs->begin());
            if (names){
                str += names->display_name();
            }else{
                str += QString::fromStdString(*slugs->begin());
            }
        }else{
            str += "Ambiguous: ";
            bool first = true;
            for (const std::string& slug : *slugs){
                if (!first){
                    str += ", ";
                }
                first = false;
//                    str += get_pokemon_name(slug).display_name();
                const PokemonNames* names = get_pokemon_name_nothrow(slug);
                if (names){
                    str += names->display_name();
                }else{
                    str += QString::fromStdString(slug);
                }
            }
        }
        embeds.emplace_back(STRING_POKEMON, std::move(str));
    }
    if (path_stats.runs() > 0){
        embeds.emplace_back("Current Path", QString::fromStdString(path_stats.to_str()));
    }
    embeds.emplace_back("Session Stats", QString::fromStdString(session_stats.to_str()));

    send_program_notification(
        logger, settings,
        Pokemon::COLOR_STAR_SHINY, program_info,
        "Max Lair Shiny Notification",
        embeds,
        image, true
    );
}



}
}
}
}
