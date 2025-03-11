/*  Max Lair Notifications
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include "CommonFramework/Globals.h"
//#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Pokemon_Notification.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "PokemonSwSh/Resources/PokemonSwSh_MaxLairDatabase.h"
#include "PokemonSwSh_MaxLair_Notifications.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{
using namespace Pokemon;


std::string pokemon_name(const std::string& slug, const std::string& empty_string){
    if (slug.empty()){
        return empty_string;
    }
    return get_pokemon_name(get_maxlair_slugs(slug).name_slug).display_name();
}


void send_status_notification(
    MultiSwitchProgramEnvironment& env,
    AdventureRuntime& runtime
){
    std::string status_str = "Status:\n";
    std::vector<std::pair<std::string, std::string>> embeds;
    {
        std::string str = pokemon_name(runtime.last_boss, "N/A");
        status_str += "Last Boss: " + str + "\n";
        embeds.emplace_back("Last Boss:", std::move(str));
    }
    {
        std::string str = runtime.path_stats.to_str(StatsTracker::DISPLAY_ON_SCREEN);
        status_str += "Current Path: " + str + "\n";
        embeds.emplace_back("Current Path:", std::move(str));
    }
    for (size_t c = 0; c < env.consoles.size(); c++){
        const ConsoleRuntime& stats = runtime.consoles[c];
        std::string label = "Console " + std::to_string(c) + ":";
        std::string str;
        str += "Ore: " + stats.ore.to_str();
        str += " - Normal Balls: " + stats.normal_balls.to_str();
        str += " - Boss Balls: " + stats.boss_balls.to_str();
        status_str += label + " - " + str + "\n";
        embeds.emplace_back(std::move(label), str);
    }
    env.log(status_str);
    send_program_notification(
        env, runtime.notification_status,
        Color(),
        "Max Lair Status Update",
        embeds, ""
    );
}

void send_raid_notification(
    ProgramEnvironment& env,
    VideoStream& stream,
    AutoHostNotificationOption& settings,
    const std::string& code,
    const std::string& slug,
    const PathStats& path_stats,
    const StatsTracker& session_stats
){
    if (!settings.enabled()){
        return;
    }

    VideoSnapshot screen = stream.video().snapshot();

    std::vector<std::pair<std::string, std::string>> embeds;

    {
        std::string description = settings.DESCRIPTION;
        if (!description.empty()){
            embeds.emplace_back("Description:", std::move(description));
        }
    }

    embeds.emplace_back(
        "Current " + STRING_POKEMON,
        pokemon_name(slug, "Unable to detect.")
    );

    {
        std::string code_str;
        if (code.empty()){
            code_str = "None";
        }
        embeds.emplace_back("Raid Code:", std::move(code_str));
    }

    if (path_stats.runs() > 0){
        embeds.emplace_back("Current Path:", path_stats.to_str(StatsTracker::DISPLAY_ON_SCREEN));
    }

    send_program_notification(
        env, settings.NOTIFICATION,
        Color(),
        "Max Lair Notification",
        embeds, "",
        screen, false
    );

}


void send_shiny_notification(
    ProgramEnvironment& env,
    Logger& logger, EventNotificationOption& settings,
    size_t console_index, size_t shinies,
    const std::set<std::string>* slugs,
    const PathStats& path_stats,
    const Stats& session_stats,
    const ImageViewRGB32& image
){
    std::vector<std::pair<std::string, std::string>> embeds;
    embeds.emplace_back(
        "Adventure Result:",
        shinies == 1
            ? "Shiny on console " + std::to_string(console_index) + "!"
            : std::to_string(shinies) + " shinies on console " + std::to_string(console_index) + "!"
    );
    if (slugs){
        std::string str;
        if (slugs->empty()){
            str = "None - Unable to detect.";
        }else if (slugs->size() == 1){
//                str += get_pokemon_name(*slugs->begin()).display_name();
            const PokemonNames* names = get_pokemon_name_nothrow(*slugs->begin());
            if (names){
                str += names->display_name();
            }else{
                str += *slugs->begin();
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
                    str += slug;
                }
            }
        }
        embeds.emplace_back(STRING_POKEMON + ":", std::move(str));
    }
    if (path_stats.runs() > 0){
        embeds.emplace_back("Current Path:", path_stats.to_str(StatsTracker::DISPLAY_ON_SCREEN));
    }

    send_program_notification(
        env, settings,
        Pokemon::COLOR_STAR_SHINY,
        "Max Lair Shiny Notification",
        embeds, "",
        image, true
    );
}



}
}
}
}
