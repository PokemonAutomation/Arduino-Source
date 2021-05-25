/*  Shiny Hunt Autonomous - Strong Spawn
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
#include "PokemonSwSh/Inference/PokemonSwSh_StartBattleDetector.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_StartGame.h"
#include "PokemonSwSh_ShinyHuntAutonomous-StrongSpawn.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntAutonomousStrongSpawn::ShinyHuntAutonomousStrongSpawn()
    : SingleSwitchProgram(
        FeedbackType::REQUIRED, PABotBaseLevel::PABOTBASE_12KB,
        "Shiny Hunt Autonomous - Strong Spawn",
        "SerialPrograms/ShinyHuntAutonomous-StrongSpawn.md",
        "Automatically hunt for shiny strong spawns using video feedback."
    )
    , GO_HOME_WHEN_DONE(
        "<b>Go Home when Done:</b><br>After finding a shiny, go to the Switch Home menu to idle. (turn this off for unattended streaming)",
        false
    )
    , TIME_ROLLBACK_HOURS(
        "<b>Time Rollback (in hours):</b><br>Periodically roll back the time to keep the weather the same. If set to zero, this feature is disabled.",
        1, 0, 11
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
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
    m_options.emplace_back(&TIME_ROLLBACK_HOURS, "TIME_ROLLBACK_HOURS");
    if (PERSISTENT_SETTINGS().developer_mode){
        m_options.emplace_back(&m_advanced_options, "");
        m_options.emplace_back(&VIDEO_ON_SHINY, "VIDEO_ON_SHINY");
        m_options.emplace_back(&RUN_FROM_EVERYTHING, "RUN_FROM_EVERYTHING");
    }
}




struct ShinyHuntAutonomousStrongSpawn::Stats : public ShinyHuntTracker{
    Stats()
        : ShinyHuntTracker(true)
        , m_timeouts(m_stats["Timeouts"])
    {
        m_display_order.insert(m_display_order.begin() + 1, Stat("Timeouts"));
    }
    uint64_t& m_timeouts;
};
std::unique_ptr<StatsTracker> ShinyHuntAutonomousStrongSpawn::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



ShinyHuntAutonomousStrongSpawn::Tracker::Tracker(
    ShinyHuntTracker& stats,
    ProgramEnvironment& env,
    ConsoleHandle& console,
    bool take_video,
    bool run_from_everything
)
    : StandardEncounterTracker(stats, env, console, false, 0, take_video, run_from_everything)
{}
bool ShinyHuntAutonomousStrongSpawn::Tracker::run_away(){
    pbf_press_button(m_console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
    return true;
}

void ShinyHuntAutonomousStrongSpawn::program(SingleSwitchProgramEnvironment& env) const{
    grip_menu_connect_go_home(env.console);
//    resume_game_no_interact(env.console, TOLERATE_SYSTEM_UPDATE_MENU_FAST);

    const uint32_t PERIOD = (uint32_t)TIME_ROLLBACK_HOURS * 3600 * TICKS_PER_SECOND;
    uint32_t last_touch = system_clock(env.console);

    Stats& stats = env.stats<Stats>();
    Tracker tracker(stats, env, env.console, VIDEO_ON_SHINY, RUN_FROM_EVERYTHING);

    while (true){
        env.update_stats();

        uint32_t now = system_clock(env.console);
        if (TIME_ROLLBACK_HOURS > 0 && now - last_touch >= PERIOD){
            rollback_hours_from_home(env.console, TIME_ROLLBACK_HOURS, SETTINGS_TO_HOME_DELAY);
            last_touch += PERIOD;
        }
        reset_game_from_home_with_inference(
            env, env.console,
            TOLERATE_SYSTEM_UPDATE_MENU_FAST
        );
        env.log("Starting Encounter: " + tostr_u_commas(stats.encounters() + 1));
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

