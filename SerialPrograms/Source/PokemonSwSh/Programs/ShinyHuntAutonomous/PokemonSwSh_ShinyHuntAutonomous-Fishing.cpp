/*  Shiny Hunt Autonomous - Fishing
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Device.h"
#include "NintendoSwitch/Commands/NintendoSwitch_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh/ShinyHuntTracker.h"
#include "PokemonSwSh/Inference/PokemonSwSh_FishingDetector.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_EncounterHandler.h"
#include "PokemonSwSh_ShinyHuntAutonomous-Fishing.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntAutonomousFishing_Descriptor::ShinyHuntAutonomousFishing_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:ShinyHuntAutonomousFishing",
        STRING_POKEMON + " SwSh", "Shiny Hunt Autonomous - Fishing",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/ShinyHuntAutonomous-Fishing.md",
        "Automatically hunt for shiny fishing " + STRING_POKEMON + " using video feedback.",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



ShinyHuntAutonomousFishing::ShinyHuntAutonomousFishing(const ShinyHuntAutonomousFishing_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , ENCOUNTER_BOT_OPTIONS(true, true)
    , NOTIFICATION_PROGRAM_FINISH("Program Finished", true, true)
    , NOTIFICATIONS({
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_NONSHINY,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_SHINY,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_CATCH_SUCCESS,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_CATCH_FAILED,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_PROGRAM_ERROR,
    })
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , EXIT_BATTLE_TIMEOUT(
        "<b>Exit Battle Timeout:</b><br>After running, wait this long to return to overworld and for the fish to reappear.",
        "10 * TICKS_PER_SECOND"
    )
    , FISH_RESPAWN_TIME(
        "<b>Fish Respawn Time:</b><br>Wait this long for fish to respawn.",
        "5 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(TIME_ROLLBACK_HOURS);

    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(ENCOUNTER_BOT_OPTIONS);
    PA_ADD_OPTION(NOTIFICATIONS);

    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(EXIT_BATTLE_TIMEOUT);
    PA_ADD_OPTION(FISH_RESPAWN_TIME);
}



struct ShinyHuntAutonomousFishing::Stats : public ShinyHuntTracker{
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
std::unique_ptr<StatsTracker> ShinyHuntAutonomousFishing::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



void ShinyHuntAutonomousFishing::program(SingleSwitchProgramEnvironment& env){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
        resume_game_no_interact(env.console, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
    }else{
        pbf_press_button(env.console, BUTTON_B, 5, 5);
    }

    const uint32_t PERIOD = (uint32_t)TIME_ROLLBACK_HOURS * 3600 * TICKS_PER_SECOND;
    uint32_t last_touch = system_clock(env.console);

    Stats& stats = env.stats<Stats>();
    env.update_stats();

    StandardEncounterHandler handler(
        env, env.console,
        LANGUAGE,
        ENCOUNTER_BOT_OPTIONS,
        stats
    );

    while (true){
        //  Touch the date.
        if (TIME_ROLLBACK_HOURS > 0 && system_clock(env.console) - last_touch >= PERIOD){
            pbf_press_button(env.console, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE);
            rollback_hours_from_home(env.console, TIME_ROLLBACK_HOURS, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY);
            resume_game_no_interact(env.console, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
            last_touch += PERIOD;
        }

        pbf_wait(env.console, FISH_RESPAWN_TIME);
        env.console.botbase().wait_for_all_requests();

        //  Trigger encounter.
        {
            pbf_press_button(env.console, BUTTON_A, 10, 10);
            pbf_mash_button(env.console, BUTTON_B, TICKS_PER_SECOND);
            env.console.botbase().wait_for_all_requests();

            FishingMissDetector miss_detector;
            FishingHookDetector hook_detector(env.console);
            StandardBattleMenuDetector menu_detector(false);
            int result = wait_until(
                env, env.console,
                std::chrono::seconds(12),
                {
                    &miss_detector,
                    &hook_detector,
                    &menu_detector,
                }
            );
            switch (result){
            case 0:
                env.log("Missed a hook.", "red");
                stats.m_misses++;
                pbf_mash_button(env.console, BUTTON_B, 2 * TICKS_PER_SECOND);
                continue;
            case 1:
                env.log("Detected hook!", "purple");
                pbf_press_button(env.console, BUTTON_A, 10, 0);
                break;
            case 2:
                env.log("Unexpected battle menu.", "red");
                stats.add_error();
                env.update_stats();
                run_away(env, env.console, EXIT_BATTLE_TIMEOUT);
                continue;
            default:
                env.log("Timed out.", "red");
                stats.add_error();
                env.update_stats();
                pbf_mash_button(env.console, BUTTON_B, 2 * TICKS_PER_SECOND);
                continue;
            }
            env.wait_for(std::chrono::seconds(3));
            if (miss_detector.detect(env.console.video().snapshot())){
                env.log("False alarm! We actually missed.", "red");
                stats.m_misses++;
                pbf_mash_button(env.console, BUTTON_B, 2 * TICKS_PER_SECOND);
                continue;
            }
        }

        //  Detect shiny.
        ShinyDetectionResult result = detect_shiny_battle(
            env.console,
            env, env.console, env.console,
            SHINY_BATTLE_REGULAR,
            std::chrono::seconds(30)
        );

        bool stop = handler.handle_standard_encounter_end_battle(result, EXIT_BATTLE_TIMEOUT);
        if (stop){
            break;
        }
    }

    send_program_finished_notification(
        env.logger(), NOTIFICATION_PROGRAM_FINISH,
        env.program_info(),
        "",
        stats.to_str()
    );
    GO_HOME_WHEN_DONE.run_end_of_program(env.console);
}



}
}
}

