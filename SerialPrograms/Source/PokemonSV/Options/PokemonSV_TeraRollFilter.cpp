/*  Tera Roll Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
//#include "CommonFramework/Tools/DebugDumper.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraCardDetector.h"
#include "PokemonSV/Programs/TeraRaids/PokemonSV_TeraRoutines.h"
#include "PokemonSV_TeraRollFilter.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


TeraRollFilter::TeraRollFilter(uint8_t default_max_stars, bool enable_herb_filter)
    : GroupOption("Opponent Filter", LockMode::UNLOCK_WHILE_RUNNING)
    , EVENT_CHECK_MODE(
        "<b>Event Tera Raid Action:</b><br>Choose how the program interacts with event/non-event raids."
        "<br>Check only non-event can be further sped up if you exclude 6 star from your filters.",
        {
            {EventCheckMode::CHECK_ALL,             "check_all",        "Check everything. Don't filter by event."},
            {EventCheckMode::CHECK_ONLY_EVENT,      "check_event",      "Check only event raids."},
            {EventCheckMode::CHECK_ONLY_NONEVENT,   "check_nonevent",   "Check only non-event raids."},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        EventCheckMode::CHECK_ALL
    )
    , MIN_STARS(
        "<b>Min Stars:</b><br>Skip raids with less than this many stars.",
        LockMode::UNLOCK_WHILE_RUNNING,
        1, 1, 7
    )
    , MAX_STARS(
        "<b>Max Stars:</b><br>Skip raids with more than this many stars.",
        LockMode::UNLOCK_WHILE_RUNNING,
        default_max_stars, 1, 7
    )
    , SKIP_NON_HERBA(
        "<b>Skip Non-Herba Raids:</b><br>"
        "Skip raids that don't have the possibility to reward all types of Herba Mystica. Enable this if you are searching for an herba raid.",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
{
    PA_ADD_OPTION(EVENT_CHECK_MODE);
    PA_ADD_OPTION(MIN_STARS);
    PA_ADD_OPTION(MAX_STARS);
    if (enable_herb_filter){
        PA_ADD_OPTION(SKIP_NON_HERBA);
    }
}

std::string TeraRollFilter::check_validity() const{
    if (MIN_STARS > MAX_STARS){
        return "\"Min Stars\" is bigger than \"Max Stars\".";
    }
    if (SKIP_NON_HERBA && MAX_STARS < 5){
        return
            "Setting \"Max Stars\" below 5 and \"Skip Herba\" to "
            "true will never yield results because all herb raids are 5-star or higher.";
    }
    if (SKIP_NON_HERBA && EVENT_CHECK_MODE == EventCheckMode::CHECK_ONLY_EVENT){
        return "\"Check only event raids\" and \"Skip Non-Herba Raids\" is incompatible because only non-event raids can have all herbs.";
    }
    return "";
}

TeraRollFilter::FilterResult TeraRollFilter::run_filter(
    const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    TeraRaidData& data
) const{
    uint8_t min_stars = MIN_STARS;
    uint8_t max_stars = MAX_STARS;
    EventCheckMode event_check_mode = EVENT_CHECK_MODE;

    bool sparkling_raid = false;

    switch (event_check_mode){
    case EventCheckMode::CHECK_ALL:
        break;
    case EventCheckMode::CHECK_ONLY_EVENT:
        // this makes sure that we only check sparkling raids
        // and that includes event & 6 star raids
        // a later star check will be performed to exclude 6 star raids
        if (!is_sparkling_raid(console, context)){
            console.log("No sparkling raid detected, skipping...", COLOR_ORANGE);
//            data.event_type = TeraRaidData::EventType::NORMAL;
            return FilterResult::NO_RAID;
        }
        break;
    case EventCheckMode::CHECK_ONLY_NONEVENT:
        if (is_sparkling_raid(console, context)){
            // if the user excluded 6 star raids, skip sparkling raids
            if (min_stars > 6 || max_stars < 6){
                console.log("Sparkling raid detected, skipping...", COLOR_ORANGE);
                return FilterResult::FAILED;
            }
            // if the user included 6 star raids, defer skip decision
            sparkling_raid = true;
        }
        break;
    }

    if (!open_raid(console, context)){
        return FilterResult::NO_RAID;
    }
    context.wait_for(std::chrono::milliseconds(500));

    VideoSnapshot screen = console.video().snapshot();
    TeraCardReader reader(COLOR_RED);

    read_card(info, console, screen, reader, data);

    switch (event_check_mode){
    case EventCheckMode::CHECK_ALL:
        break;
    case EventCheckMode::CHECK_ONLY_EVENT:
        // only sparkling raids at this point
        // skip 6 star raids
        if (data.stars == 6){
            console.log("Detected non-event 6 star raid, skipping...", COLOR_ORANGE);
            close_raid(info, console, context);
            return FilterResult::NO_RAID;
        }
        break;
    case EventCheckMode::CHECK_ONLY_NONEVENT:
        // skip sparkling raids unless 6 stars
        if (sparkling_raid && data.stars != 6){
            console.log("Detected event raid, skipping...", COLOR_ORANGE);
            close_raid(info, console, context);
            return FilterResult::NO_RAID;
        }
        break;
    }

    if (data.stars < min_stars || data.stars > max_stars){
        console.log("Raid stars is out of range. Skipping...");
        close_raid(info, console, context);
        return FilterResult::FAILED;
    }

    // TODO: Add species filter

    if (!check_herba(data.species)){
        console.log("Raid cannot have all herbas. Skipping...");
        close_raid(info, console, context);
        return FilterResult::FAILED;
    }

    return FilterResult::PASSED;
}


void TeraRollFilter::read_card(
    const ProgramInfo& info, ConsoleHandle& console, const ImageViewRGB32& screen,
    TeraCardReader& reader, TeraRaidData& data
) const{
    data.stars = reader.stars(console, info, screen);
    data.tera_type = reader.tera_type(console, info, screen);
    data.species = reader.pokemon_slug(console, info, screen);

    std::string stars = data.stars == 0
        ? "?"
        : std::to_string(data.stars);
    std::string tera_type = data.tera_type.empty()
        ? "? tera"
        : data.tera_type;
    std::string pokemon = data.species.empty()
        ? "? " + Pokemon::STRING_POKEMON
        : data.species;

    console.overlay().add_log(
        stars + "* " + tera_type + " " + pokemon,
        COLOR_GREEN
    );
    std::string log = "Detected a " + stars + "* " + tera_type + " " + pokemon;
    console.log(log);
}
bool TeraRollFilter::check_herba(const std::string& pokemon_slug) const{
    if (!SKIP_NON_HERBA){
        return true;
    }

    static const std::set<std::string> fivestar{
        "gengar", "glalie", "amoonguss", "dondozo", "palafin", "finizen", "blissey", "eelektross", "drifblim", "cetitan",
        "snorlax", "dusknoir", "mandibuzz", "basculegion"
    };
    static const std::set<std::string> sixstar{
        "blissey", "vaporeon", "amoonguss", "farigiraf", "cetitan", "dondozo",
        "poliwrath", "snorlax", "basculegion"
    };

    if (fivestar.find(pokemon_slug) != fivestar.end()){
        return true;
    }
    if (sixstar.find(pokemon_slug) != sixstar.end()){
        return true;
    }

    return false;
}




}
}
}
