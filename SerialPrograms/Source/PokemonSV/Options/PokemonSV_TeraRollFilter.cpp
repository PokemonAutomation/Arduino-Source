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


TeraRollFilter::TeraRollFilter()
    : GroupOption("Opponent Filter", LockWhileRunning::UNLOCKED)
    , EVENT_CHECK_MODE(
        "<b>Event Tera Raid Action:</b><br>Choose how the program interacts with event/non-event raids."
        "<br>Check only non-event can be further sped up if you exclude 6 star from your filters.",
        {
            {EventCheckMode::CHECK_ALL,             "check_all",        "Check everything. Don't filter by event."},
            {EventCheckMode::CHECK_ONLY_EVENT,      "check_event",      "Check only event raids."},
            {EventCheckMode::CHECK_ONLY_NONEVENT,   "check_nonevent",   "Check only non-event raids."},
        },
        LockWhileRunning::UNLOCKED,
        EventCheckMode::CHECK_ALL
    )
    , MIN_STARS(
        "<b>Min Stars:</b><br>Skip raids with less than this many stars.",
        LockWhileRunning::UNLOCKED,
        1, 1, 7
    )
    , MAX_STARS(
        "<b>Max Stars:</b><br>Skip raids with more than this many stars.",
        LockWhileRunning::UNLOCKED,
        4, 1, 7
    )
{
    PA_ADD_OPTION(EVENT_CHECK_MODE);
    PA_ADD_OPTION(MIN_STARS);
    PA_ADD_OPTION(MAX_STARS);
}

void TeraRollFilter::start_program_check(Logger& logger) const{
    if (MIN_STARS > MAX_STARS){
        throw UserSetupError(logger, "Error in the settings, \"Min Stars\" is bigger than \"Max Stars\".");
    }
}

TeraRollFilter::FilterResult TeraRollFilter::run_filter(
    const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    std::string* pokemon_slug, size_t* stars
){
    if (pokemon_slug != nullptr){
        pokemon_slug->clear();
    }
    if (stars != nullptr){
        *stars = 0;
    }

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
    size_t read_stars = reader.stars(screen);
    if (stars != nullptr){
        *stars = read_stars;
    }
    if (read_stars == 0){
        dump_image(console, info, "ReadStarsFailed", *screen.frame);
    }

    switch (event_check_mode){
    case EventCheckMode::CHECK_ALL:
        break;
    case EventCheckMode::CHECK_ONLY_EVENT:
        // only sparkling raids at this point
        // skip 6 star raids
        if (read_stars == 6){
            console.log("Detected non-event 6 star raid, skipping...", COLOR_ORANGE);
            close_raid(info, console, context);
            return FilterResult::NO_RAID;
        }
        break;
    case EventCheckMode::CHECK_ONLY_NONEVENT:
        // skip sparkling raids unless 6 stars
        if (sparkling_raid && read_stars != 6){
            console.log("Detected event raid, skipping...", COLOR_ORANGE);
            close_raid(info, console, context);
            return FilterResult::NO_RAID;
        }
        break;
    }
    context.wait_for_all_requests();


    VideoOverlaySet overlay_set(console);

    std::string silhouette = reader.pokemon_slug(console, info, screen);
    if (silhouette.empty()){
        silhouette = "unknown " + Pokemon::STRING_POKEMON;
    }

    std::string tera_type = reader.tera_type(console, info, screen);
    if (tera_type.empty()){
        tera_type = "unknown tera type";
    }

    std::string log = "Detected a " + std::to_string(read_stars) + "* " + tera_type + " " + silhouette;
    console.overlay().add_log(log, COLOR_GREEN);
    console.log(log);

    if (read_stars < min_stars || read_stars > max_stars){
        close_raid(info, console, context);
        return FilterResult::FAILED;
    }

    // TODO: Add species filter

    return FilterResult::PASSED;
}




}
}
}
