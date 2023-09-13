/*  Tera Roller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include <set>
#include <sstream>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Inference/VisualDetector.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/Tools/DebugDumper.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Pokemon_Notification.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxShinyDetector.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraCardDetector.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraSilhouetteReader.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraTypeReader.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV/Programs/TeraRaids/PokemonSV_TeraRoutines.h"
#include "PokemonSV_TeraRoller.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


TeraRoller_Descriptor::TeraRoller_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:TeraRoller",
        STRING_POKEMON + " SV", "Tera Roller",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/TeraRoller.md",
        "Roll Tera raids to find shiny " + STRING_POKEMON + ".",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
struct TeraRoller_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_skips(m_stats["Date Skips"])
        , m_raids(m_stats["Raids"])
        , m_skipped(m_stats["Skipped"])
        , m_errors(m_stats["Errors"])
        , m_shinies(m_stats["Shinies"])
    {
        m_display_order.emplace_back("Date Skips");
        m_display_order.emplace_back("Raids");
        m_display_order.emplace_back("Skipped");
        m_display_order.emplace_back("Errors", true);
        m_display_order.emplace_back("Shinies", true);
    }
    std::atomic<uint64_t>& m_skips;
    std::atomic<uint64_t>& m_raids;
    std::atomic<uint64_t>& m_skipped;
    std::atomic<uint64_t>& m_errors;
    std::atomic<uint64_t>& m_shinies;
};
std::unique_ptr<StatsTracker> TeraRoller_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



TeraRollerOpponentFilter::TeraRollerOpponentFilter()
    : GroupOption("Opponent Filter", LockWhileRunning::UNLOCKED)
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
    PA_ADD_OPTION(MIN_STARS);
    PA_ADD_OPTION(MAX_STARS);
}

bool TeraRollerOpponentFilter::should_battle(size_t stars, const std::string& pokemon) const{
    if (stars < MIN_STARS || stars > MAX_STARS){
        return false;
    }

    // TODO: Add species filter

    return true;
}


TeraRoller::TeraRoller()
    : CHECK_ONLY_FIRST(
          "<b>Check Only the First Pokédex Page:</b><br>Reduce time per reset at the expense of not checking repeated encounters.",
          LockWhileRunning::UNLOCKED,
          false
    )
    , EVENT_CHECK_MODE(
        "<b>Event Tera Raid Action:</b><br>Choose how the program interacts with event/non-event raids."
        "<br>Check only non-event can be further sped up if you exclude 6 star from your filters.",
        {
            {EventCheckMode::CHECK_ALL,             "check_all",        "Ongoing event, check all raids / No ongoing event"},
            {EventCheckMode::CHECK_ONLY_EVENT,      "check_event",      "Ongoing event, check only event raids"},
            {EventCheckMode::CHECK_ONLY_NONEVENT,   "check_nonevent",   "Ongoing event, check only non-event raids"},
        },
        LockWhileRunning::LOCKED,
        EventCheckMode::CHECK_ALL
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATION_SHINY(
        "Shiny Encounter",
        true, true, ImageAttachmentMode::JPG,
        {"Notifs", "Showcase"}
    )
    , m_notification_noop("", false, false)
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_SHINY,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(CHECK_ONLY_FIRST);
    PA_ADD_OPTION(EVENT_CHECK_MODE);
    PA_ADD_OPTION(FILTER);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void TeraRoller::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    TeraRoller_Descriptor::Stats& stats = env.current_stats<TeraRoller_Descriptor::Stats>();

    if (FILTER.MIN_STARS > FILTER.MAX_STARS){
        throw UserSetupError(env.console, "Error in the settings, \"Min Stars\" is bigger than \"Max Stars\".");
    }

    //  Connect the controller
    pbf_press_button(context, BUTTON_L, 10, 10);

    bool first = true;

    while (true){
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

        if (!first){
            day_skip_from_overworld(env.console, context);
            pbf_wait(context, GameSettings::instance().RAID_SPAWN_DELAY);
            context.wait_for_all_requests();
            stats.m_skips++;
        }
        first = false;

        bool sparkling_raid = false;

        switch (EVENT_CHECK_MODE){
        case EventCheckMode::CHECK_ALL:
            break;
        case EventCheckMode::CHECK_ONLY_EVENT:
            // this makes sure that we only check sparkling raids
            // and that includes event & 6 star raids
            // a later star check will be performed to exclude 6 star raids
            if (!is_sparkling_raid(env, context)){
                env.log("No sparkling raid detected, skipping...", COLOR_ORANGE);
                continue;
            }
            break;
        case EventCheckMode::CHECK_ONLY_NONEVENT:
            if (is_sparkling_raid(env, context)){
                // if the user excluded 6 star raids, skip sparkling raids
                if (FILTER.MIN_STARS > 6 || FILTER.MAX_STARS < 6){
                    env.log("Sparkling raid detected, skipping...", COLOR_ORANGE);
                    continue;
                }
                // if the user included 6 star raids, defer skip decision
                sparkling_raid = true;
            }
            break;
        }

        if (open_raid(env.console, context)){
            stats.m_raids++;
        }else{
            continue;
        }
        context.wait_for(std::chrono::milliseconds(500));

        VideoSnapshot screen = env.console.video().snapshot();
        TeraCardReader reader(COLOR_RED);
        size_t stars = reader.stars(screen);
        if (stars == 0){
            dump_image(env.logger(), env.program_info(), "ReadStarsFailed", *screen.frame);
        }

        switch (EVENT_CHECK_MODE){
        case EventCheckMode::CHECK_ALL:
            break;
        case EventCheckMode::CHECK_ONLY_EVENT:
            // only sparkling raids at this point
            // skip 6 star raids
            if (stars == 6){
                env.log("Detected non-event 6 star raid, skipping...", COLOR_ORANGE);
                stats.m_skipped++;
                close_raid(env.program_info(), env.console, context);
                continue;
            }
            break;
        case EventCheckMode::CHECK_ONLY_NONEVENT:
            // skip sparkling raids unless 6 stars
            if (sparkling_raid && stars != 6){
                env.log("Detected event raid, skipping...", COLOR_ORANGE);
                stats.m_skipped++;
                close_raid(env.program_info(), env.console, context);
                continue;
            }
            break;
        }
        context.wait_for_all_requests();

        VideoOverlaySet overlay_set(env.console);

        TeraSilhouetteReader silhouette_reader;
        silhouette_reader.make_overlays(overlay_set);
        ImageMatch::ImageMatchResult silhouette = silhouette_reader.read(screen);
        silhouette.log(env.logger(), 100);
        std::string best_silhouette = silhouette.results.empty() ? "UnknownSilhouette" : silhouette.results.begin()->second;
        if (silhouette.results.empty()){
            dump_image(env.logger(), env.program_info(), "ReadSilhouetteFailed", *screen.frame);
        }
        else if (PreloadSettings::debug().IMAGE_TEMPLATE_MATCHING){
            dump_debug_image(env.logger(), "PokemonSV/TeraRoller/" + best_silhouette, "", screen);
        }

        TeraTypeReader type_reader;
        type_reader.make_overlays(overlay_set);
        ImageMatch::ImageMatchResult type = type_reader.read(screen);
        type.log(env.logger(), 100);
        std::string best_type = type.results.empty() ? "UnknownType" : type.results.begin()->second;
        if (type.results.empty()){
            dump_image(env.logger(), env.program_info(), "ReadTypeFailed", *screen.frame);
        }
        else if (PreloadSettings::debug().IMAGE_TEMPLATE_MATCHING){
            dump_debug_image(env.logger(), "PokemonSV/TeraRoller/" + best_type, "", screen);
        }

        {
            std::string log = "Detected a " + std::to_string(stars) + "* " + best_type + " " + best_silhouette;
            env.console.overlay().add_log(log, COLOR_GREEN);
            env.log(log);
        }

        if (!FILTER.should_battle(stars, best_silhouette)) {
            env.log("Skipping raid...", COLOR_ORANGE);
            stats.m_skipped++;
            close_raid(env.program_info(), env.console, context);
            continue;
        }

        // Enter tera raid battle alone
        pbf_press_dpad(context, DPAD_DOWN, 10, 10);
        pbf_mash_button(context, BUTTON_A, 250);
        context.wait_for_all_requests();
        overlay_set.clear();
        env.console.log("Entering tera raid...");
        env.console.overlay().add_log("Entering tera raid...", COLOR_WHITE);

        // Run away from the tera raid battle
        run_from_tera_battle(env.program_info(), env.console, context);
        context.wait_for_all_requests();

        env.console.log("Checking if tera raid is shiny...");
        env.console.overlay().add_log("Checking Pokédex...", COLOR_WHITE);
        open_pokedex_from_overworld(env.program_info(), env.console, context);
        open_recently_battled_from_pokedex(env.program_info(), env.console, context);

        // Since encountering the same species within 5 encounters is possible,
        // loop through all 5 candidates of recently battled pokemon for shinies
        for(int i = 0; i < 5; i++){
            BoxShinyWatcher shiny_detector(COLOR_YELLOW, {0.187, 0.196, 0.028, 0.046});
            context.wait_for_all_requests();

            int ret = wait_until(
                env.console, context,
                std::chrono::seconds(1),
                {shiny_detector}
            );

            if (ret == 0){
                env.console.log("Found a shiny tera raid!", COLOR_GREEN);
                env.console.overlay().add_log("Shiny!", COLOR_GREEN);
                stats.m_shinies += 1;

                pbf_wait(context, 500); // Wait enough time for the Pokémon sprite to load
                context.wait_for_all_requests();
                send_encounter_notification(
                    env,
                    m_notification_noop,
                    NOTIFICATION_SHINY,
                    false, true, {{{}, ShinyType::UNKNOWN_SHINY}}, std::nan(""),
                    env.console.video().snapshot()
                );

                leave_phone_to_overworld(env.program_info(), env.console, context);
                save_game_from_overworld(env.program_info(), env.console, context);

                throw ProgramFinishedException();
            }

            if (CHECK_ONLY_FIRST) { // Check only the first Pokédex page
                break;
            } else if (i < 4){ // Check the remaining four Pokédex pages
                pbf_press_dpad(context, DPAD_RIGHT, 10, 20);
            }
        }

        env.console.log("Not a shiny tera raid...");
        env.console.overlay().add_log("Not shiny", COLOR_WHITE);
        leave_phone_to_overworld(env.program_info(), env.console, context);

        pbf_wait(context, 50);
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}











}
}
}
