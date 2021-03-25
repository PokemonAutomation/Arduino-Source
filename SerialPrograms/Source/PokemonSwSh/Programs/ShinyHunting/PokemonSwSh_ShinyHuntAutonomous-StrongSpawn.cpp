/*  ShinyHuntAutonomous-StrongSpawn
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
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , EXIT_BATTLE_MASH_TIME(
        "<b>Exit Battle Time:</b><br>After running, wait this long to return to overworld.",
        "6 * TICKS_PER_SECOND"
    )
    , TIME_ROLLBACK_HOURS(
        "<b>Time Rollback (in hours):</b><br>Periodically roll back the time to keep the weather the same. If set to zero, this feature is disabled.",
        1, 0, 11
    )
{
    m_options.emplace_back(&m_advanced_options, "");
    m_options.emplace_back(&EXIT_BATTLE_MASH_TIME, "EXIT_BATTLE_MASH_TIME");
    m_options.emplace_back(&TIME_ROLLBACK_HOURS, "TIME_ROLLBACK_HOURS");
}


std::string ShinyHuntAutonomousStrongSpawn::Stats::stats() const{
    std::string str;
    str += str_encounters();
    str += " - Timeouts: " + tostr_u_commas(m_timeouts);
    str += str_shinies();
    return str;
}

ShinyHuntAutonomousStrongSpawn::Tracker::Tracker(EncounterStats& stats, ConsoleHandle& console)
    : StandardEncounterTracker(stats, console, false, 0)
{}
bool ShinyHuntAutonomousStrongSpawn::Tracker::run_away(){
    pbf_press_button(BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
    return true;
}

void ShinyHuntAutonomousStrongSpawn::program(SingleSwitchProgramEnvironment& env) const{
    grip_menu_connect_go_home();
//    resume_game_no_interact(TOLERATE_SYSTEM_UPDATE_MENU_FAST);

    const uint32_t PERIOD = (uint32_t)TIME_ROLLBACK_HOURS * 3600 * TICKS_PER_SECOND;
    uint32_t last_touch = system_clock();

    Stats stats;
    Tracker tracker(stats, env.console);

    while (true){
        stats.log_stats(env, env.logger);

        uint32_t now = system_clock();
        if (TIME_ROLLBACK_HOURS > 0 && now - last_touch >= PERIOD){
            rollback_hours_from_home(TIME_ROLLBACK_HOURS, SETTINGS_TO_HOME_DELAY);
            last_touch += PERIOD;
        }
        reset_game_from_home_with_inference(
            env, env.logger, env.console,
            TOLERATE_SYSTEM_UPDATE_MENU_FAST
        );
        env.logger.log("Starting Encounter: " + tostr_u_commas(stats.encounters() + 1));
        env.console.botbase().wait_for_all_requests();

        //  Detect shiny.
        ShinyEncounterDetector::Detection detection;
        {
            ShinyEncounterDetector detector(
                env.console, env.logger,
                ShinyEncounterDetector::REGULAR_BATTLE,
                std::chrono::seconds(60)
            );
            detection = detector.detect(env);
        }

        if (tracker.process_result(detection)){
            break;
        }
        if (detection == ShinyEncounterDetector::NO_BATTLE_MENU){
            stats.m_timeouts++;
            tracker.run_away();
        }
    }

    stats.log_stats(env, env.logger);

    pbf_press_button(BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);

    end_program_callback();
    end_program_loop();
}



}
}
}

