/*  Shiny Hunt Autonomous - Fishing
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh/ShinyHuntTracker.h"
#include "PokemonSwSh/Inference/PokemonSwSh_FishingDetector.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh/Programs/PokemonSwSh_EncounterHandler.h"
#include "PokemonSwSh_ShinyHuntAutonomous-Fishing.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntAutonomousFishing_Descriptor::ShinyHuntAutonomousFishing_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:ShinyHuntAutonomousFishing",
        STRING_POKEMON + " SwSh", "Shiny Hunt Autonomous - Fishing",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/ShinyHuntAutonomous-Fishing.md",
        "Automatically hunt for shiny fishing " + STRING_POKEMON + " using video feedback.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct ShinyHuntAutonomousFishing_Descriptor::Stats : public ShinyHuntTracker{
    Stats()
        : ShinyHuntTracker(true)
        , m_misses(m_stats["Misses"])
    {
        m_display_order.insert(m_display_order.begin() + 1, Stat("Misses"));
        m_aliases["Timeouts"] = "Errors";
        m_aliases["Unexpected Battles"] = "Errors";
    }
    std::atomic<uint64_t>& m_misses;
};
std::unique_ptr<StatsTracker> ShinyHuntAutonomousFishing_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



ShinyHuntAutonomousFishing::ShinyHuntAutonomousFishing()
    : GO_HOME_WHEN_DONE(false)
    , ENCOUNTER_BOT_OPTIONS(true, true)
    , NOTIFICATIONS({
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_NONSHINY,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_SHINY,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_CATCH_SUCCESS,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_CATCH_FAILED,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , EXIT_BATTLE_TIMEOUT0(
        "<b>Exit Battle Timeout:</b><br>After running, wait this long to return to overworld and for the fish to reappear.",
        LockMode::LOCK_WHILE_RUNNING,
        "10000 ms"
    )
    , FISH_RESPAWN_TIME0(
        "<b>Fish Respawn Time:</b><br>Wait this long for fish to respawn.",
        LockMode::LOCK_WHILE_RUNNING,
        "5000 ms"
    )
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(TIME_ROLLBACK_HOURS);

    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(ENCOUNTER_BOT_OPTIONS);
    PA_ADD_OPTION(NOTIFICATIONS);

    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(EXIT_BATTLE_TIMEOUT0);
    PA_ADD_OPTION(FISH_RESPAWN_TIME0);
}



void ShinyHuntAutonomousFishing::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_no_interact(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
    }

    WallDuration PERIOD = std::chrono::hours(TIME_ROLLBACK_HOURS);
    WallClock last_touch = current_time();

    ShinyHuntAutonomousFishing_Descriptor::Stats& stats = env.current_stats<ShinyHuntAutonomousFishing_Descriptor::Stats>();
    env.update_stats();

    StandardEncounterHandler handler(
        env, env.console, context,
        LANGUAGE,
        ENCOUNTER_BOT_OPTIONS,
        stats
    );

    while (true){
        //  Touch the date.
        if (TIME_ROLLBACK_HOURS > 0 && current_time() - last_touch >= PERIOD){
            pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0);
            rollback_hours_from_home(env.console, context, TIME_ROLLBACK_HOURS, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
            resume_game_no_interact(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
            last_touch += PERIOD;
        }

        pbf_wait(context, FISH_RESPAWN_TIME0);
        context.wait_for_all_requests();

        //  Trigger encounter.
        {
            pbf_press_button(context, BUTTON_A, 10, 10);
            pbf_mash_button(context, BUTTON_B, TICKS_PER_SECOND);
            context.wait_for_all_requests();

            FishingMissDetector miss_detector;
            FishingHookDetector hook_detector(env.console);
            StandardBattleMenuWatcher menu_detector(false);
            int result = wait_until(
                env.console, context,
                std::chrono::seconds(12),
                {
                    {miss_detector},
                    {hook_detector},
                    {menu_detector},
                }
            );
            switch (result){
            case 0:
                env.log("Missed a hook.", COLOR_RED);
                stats.m_misses++;
                pbf_mash_button(context, BUTTON_B, 2000ms);
                continue;
            case 1:
                env.log("Detected hook!", COLOR_PURPLE);
                ssf_press_button_ptv(context, BUTTON_A, 120ms);
//                pbf_mash_button(context, BUTTON_A, TICKS_PER_SECOND);
                break;
            case 2:
                env.log("Unexpected battle menu.", COLOR_RED);
                stats.add_error();
                env.update_stats();
                run_away(env.console, context, EXIT_BATTLE_TIMEOUT0);
                continue;
            default:
                env.log("Timed out.", COLOR_RED);
                stats.add_error();
                env.update_stats();
                pbf_mash_button(context, BUTTON_B, 2000ms);
                continue;
            }
            context.wait_for(std::chrono::seconds(3));
            if (miss_detector.detect(env.console.video().snapshot())){
                env.log("False alarm! We actually missed.", COLOR_RED);
                stats.m_misses++;
                pbf_mash_button(context, BUTTON_B, 2000ms);
                continue;
            }
        }

        //  Detect shiny.
        ShinyDetectionResult result = detect_shiny_battle(
            env.console, context,
            SHINY_BATTLE_REGULAR,
            std::chrono::seconds(30)
        );

        bool stop = handler.handle_standard_encounter_end_battle(result, EXIT_BATTLE_TIMEOUT0);
        if (stop){
            break;
        }
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}



}
}
}

