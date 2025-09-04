/*  Tera Roller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Pokemon_Notification.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxShinyDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
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
        ProgramControllerClass::StandardController_PerformanceClassSensitive,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct TeraRoller_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_skips(m_stats["Date Skips"])
        , m_resets(m_stats["Resets"])
        , m_raids(m_stats["Raids"])
        , m_skipped(m_stats["Skipped"])
        , m_errors(m_stats["Errors"])
        , m_shinies(m_stats["Shinies"])
    {
        m_display_order.emplace_back("Date Skips");
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Raids");
        m_display_order.emplace_back("Skipped");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Shinies", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& m_skips;
    std::atomic<uint64_t>& m_resets;
    std::atomic<uint64_t>& m_raids;
    std::atomic<uint64_t>& m_skipped;
    std::atomic<uint64_t>& m_errors;
    std::atomic<uint64_t>& m_shinies;
};
std::unique_ptr<StatsTracker> TeraRoller_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}





TeraRoller::TeraRoller()
    : FILTER0(7, false)
    , CHECK_ONLY_FIRST(
          "<b>Check Only the First Pokédex Page:</b><br>Reduce time per reset at the expense of not checking repeated encounters.",
        LockMode::UNLOCK_WHILE_RUNNING,
          false
    )
    , PERIODIC_RESET(
        "<b>Periodic Game Reset:</b><br>Reset the game after this many skips. This clears up the framerate bug.",
        LockMode::UNLOCK_WHILE_RUNNING,
        20, 0, 100
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
    PA_ADD_OPTION(FILTER0);
    PA_ADD_OPTION(CHECK_ONLY_FIRST);
    PA_ADD_OPTION(PERIODIC_RESET);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void TeraRoller::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    TeraRoller_Descriptor::Stats& stats = env.current_stats<TeraRoller_Descriptor::Stats>();

    //  Connect the controller
    pbf_press_button(context, BUTTON_L, 10, 10);

    bool first = true;
    uint32_t skip_counter = 0;

    //  Keep track of when we last reset.
    //  Day skips too soon out of a reset may fail.
    WallClock last_reset = WallClock::min();

    while (true){
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

        if (!first){
            day_skip_from_overworld(env.console, context);

            //  Do it again if we're fresh out of a reset.
            while (last_reset + std::chrono::seconds(20) > current_time()){
                env.log("Fresh out of a reset. Skipping again.");
                day_skip_from_overworld(env.console, context);
            }

            pbf_wait(context, GameSettings::instance().RAID_SPAWN_DELAY0);
            context.wait_for_all_requests();
            stats.m_skips++;
            skip_counter++;
            env.update_stats();
        }
        first = false;

        uint8_t reset_period = PERIODIC_RESET;
        if (reset_period != 0 && skip_counter >= reset_period){
            env.log("Resetting game to clear framerate.");
            save_game_from_overworld(env.program_info(), env.console, context);
            reset_game(env.program_info(), env.console, context);
            last_reset = current_time();
            skip_counter = 0;
            stats.m_resets++;
        }

        TeraRaidData raid_data;
        TeraRollFilter::FilterResult result = FILTER0.run_filter(
            env.program_info(), env.console, context,
            raid_data
        );
        switch (result){
        case TeraRollFilter::FilterResult::NO_RAID:
            continue;
        case TeraRollFilter::FilterResult::FAILED:
            stats.m_raids++;
            stats.m_skipped++;
            continue;
        case TeraRollFilter::FilterResult::PASSED:
            stats.m_raids++;
            break;
        }


        // Enter tera raid battle alone
        pbf_press_dpad(context, DPAD_DOWN, 10, 10);
        pbf_mash_button(context, BUTTON_A, 250);
        context.wait_for_all_requests();
//        overlay_set.clear();
        env.console.log("Entering tera raid...");
        env.console.overlay().add_log("Entering tera raid...", COLOR_WHITE);

        // Run away from the tera raid battle
        run_from_tera_battle(env, env.console, context, &stats.m_errors);
        context.wait_for_all_requests();

        env.console.log("Checking if tera raid is shiny...");
        env.console.overlay().add_log("Checking Pokédex...", COLOR_WHITE);
        open_pokedex_from_overworld(env.program_info(), env.console, context);
        open_recently_battled_from_pokedex(env.program_info(), env.console, context);

        // Since encountering the same species within 5 encounters is possible,
        // loop through all 5 candidates of recently battled pokemon for shinies
        for (int i = 0; i < 5; i++){
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

                pbf_wait(context, 500); // Wait enough time for the Pokemon sprite to load
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
            }else if (i < 4){ // Check the remaining four Pokédex pages
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
