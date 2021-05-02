/*  Shiny Hunt Autonomous - Swords Of Justice
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Clientside/PrettyPrint.h"
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/PokemonSwSh/PokemonSwShDateSpam.h"
#include "CommonFramework/PersistentSettings.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"
#include "PokemonSwSh_EncounterTracker.h"
#include "PokemonSwSh_ShinyHuntAutonomous-SwordsOfJustice.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntAutonomousSwordsOfJustice::ShinyHuntAutonomousSwordsOfJustice()
    : SingleSwitchProgram(
        FeedbackType::REQUIRED, PABotBaseLevel::PABOTBASE_12KB,
        "Shiny Hunt Autonomous - Swords Of Justice",
        "SerialPrograms/ShinyHuntAutonomous-SwordsOfJustice.md",
        "Automatically hunt for shiny Sword of Justice using video feedback."
    )
    , GO_HOME_WHEN_DONE(
        "<b>Go Home when Done:</b><br>After finding a shiny, go to the Switch Home menu to idle. (turn this off for unattended streaming)",
        false
    )
    , AIRPLANE_MODE(
        "<b>Airplane Mode:</b><br>Enable if airplane mode is on.",
        false
    )
    , TIME_ROLLBACK_HOURS(
        "<b>Time Rollback (in hours):</b><br>Periodically roll back the time to keep the weather the same. If set to zero, this feature is disabled.",
        1, 0, 11
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , EXIT_BATTLE_MASH_TIME(
        "<b>Exit Battle Time:</b><br>After running, wait this long to return to overworld.",
        "6 * TICKS_PER_SECOND"
    )
    , ENTER_CAMP_DELAY(
        "<b>Enter Camp Delay:</b>",
        "8 * TICKS_PER_SECOND"
    )
    , VIDEO_ON_SHINY(
        "<b>Video Capture:</b><br>Take a video of the encounter if it is shiny.",
        true
    )
    , RUN_FROM_EVERYTHING(
        "<b>Run from Everything:</b><br>Run from everything - even if it is shiny. (For testing only.)",
        false
    )
{
    m_options.emplace_back(&GO_HOME_WHEN_DONE, "GO_HOME_WHEN_DONE");
    m_options.emplace_back(&AIRPLANE_MODE, "AIRPLANE_MODE");
    m_options.emplace_back(&TIME_ROLLBACK_HOURS, "TIME_ROLLBACK_HOURS");
    m_options.emplace_back(&m_advanced_options, "");
    m_options.emplace_back(&EXIT_BATTLE_MASH_TIME, "EXIT_BATTLE_MASH_TIME");
    m_options.emplace_back(&ENTER_CAMP_DELAY, "ENTER_CAMP_DELAY");
    if (settings.developer_mode){
        m_options.emplace_back(&VIDEO_ON_SHINY, "VIDEO_ON_SHINY");
        m_options.emplace_back(&RUN_FROM_EVERYTHING, "RUN_FROM_EVERYTHING");
    }
}



struct ShinyHuntAutonomousSwordsOfJustice::Stats : public ShinyHuntTracker{
    Stats()
        : ShinyHuntTracker(true)
        , m_timeouts(m_stats["Timeouts"])
    {
        m_display_order.insert(m_display_order.begin() + 1, Stat("Timeouts"));
    }
    uint64_t& m_timeouts;
};
std::unique_ptr<StatsTracker> ShinyHuntAutonomousSwordsOfJustice::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}




void ShinyHuntAutonomousSwordsOfJustice::program(SingleSwitchProgramEnvironment& env) const{
    grip_menu_connect_go_home(env.console);
    resume_game_no_interact(env.console, TOLERATE_SYSTEM_UPDATE_MENU_FAST);

    const uint32_t PERIOD = (uint32_t)TIME_ROLLBACK_HOURS * 3600 * TICKS_PER_SECOND;
    uint32_t last_touch = system_clock(env.console);

    Stats& stats = env.stats<Stats>();
    StandardEncounterTracker tracker(
        stats, env.console,
        false,
        EXIT_BATTLE_MASH_TIME,
        VIDEO_ON_SHINY,
        RUN_FROM_EVERYTHING
    );

    while (true){
        env.update_stats();

        //  Touch the date.
        if (TIME_ROLLBACK_HOURS > 0 && system_clock(env.console) - last_touch >= PERIOD){
            pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
            rollback_hours_from_home(env.console, TIME_ROLLBACK_HOURS, SETTINGS_TO_HOME_DELAY);
            resume_game_no_interact(env.console, TOLERATE_SYSTEM_UPDATE_MENU_FAST);
            last_touch += PERIOD;
        }

        //  Trigger encounter.
        pbf_press_button(env.console, BUTTON_X, 10, OVERWORLD_TO_MENU_DELAY);
        pbf_press_button(env.console, BUTTON_A, 10, ENTER_CAMP_DELAY);
        if (AIRPLANE_MODE){
            pbf_press_button(env.console, BUTTON_A, 10, 100);
            pbf_press_button(env.console, BUTTON_A, 10, 100);
        }
        pbf_press_button(env.console, BUTTON_X, 10, 50);
        pbf_press_dpad(env.console, DPAD_LEFT, 10, 10);
        env.log("Starting Encounter: " + tostr_u_commas(stats.encounters() + 1));
        pbf_press_button(env.console, BUTTON_A, 10, 0);
        env.console.botbase().wait_for_all_requests();

        //  Detect shiny.
        ShinyDetection detection = detect_shiny_battle(
            env, env.console,
            SHINY_BATTLE_REGULAR,
            std::chrono::seconds(30)
        );

        if (tracker.process_result(detection)){
            break;
        }
        if (detection == ShinyDetection::NO_BATTLE_MENU){
            stats.m_timeouts++;
            pbf_mash_button(env.console, BUTTON_B, TICKS_PER_SECOND);
            tracker.run_away();
        }
    }

    env.update_stats();

    if (GO_HOME_WHEN_DONE){
        pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
    }

    end_program_callback(env.console);
    end_program_loop(env.console);
}





}
}
}
