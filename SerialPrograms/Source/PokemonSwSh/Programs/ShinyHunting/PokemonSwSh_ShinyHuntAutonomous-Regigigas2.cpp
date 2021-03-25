/*  ShinyHuntAutonomous-Regigigas2
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
#include "PokemonSwSh/Inference/PokemonSwSh_RaidCatchDetector.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_StartGame.h"
#include "PokemonSwSh_ShinyHuntAutonomous-Regigigas2.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

ShinyHuntAutonomousRegigigas2::ShinyHuntAutonomousRegigigas2()
    : SingleSwitchProgram(
        FeedbackType::REQUIRED, PABotBaseLevel::PABOTBASE_12KB,
        "Shiny Hunt Autonomous - Regigigas2",
        "SerialPrograms/ShinyHuntAutonomous-Regigigas2.md",
        "Automatically hunt for shiny Regigigas using video feedback."
    )
    , REVERSAL_PP(
        "<b>Reversal PP:</b><br>The amount of Reversal PP you are saved with.",
        24
    )
    , REQUIRE_SQUARE(
        "<b>Require Square:</b><br>Stop only for a square shiny. Run from star shinies.",
        false
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , CATCH_TO_OVERWORLD_DELAY(
        "<b>Catch to Overworld Delay:</b>",
        "8 * TICKS_PER_SECOND"
    )
    , TOUCH_DATE_INTERVAL(
        "<b>Rollover Prevention:</b><br>Prevent a den from rolling over by periodically touching the date. If set to zero, this feature is disabled.",
        "4 * 3600 * TICKS_PER_SECOND"
    )
{
    m_options.emplace_back(&REVERSAL_PP, "REVERSAL_PP");
    m_options.emplace_back(&REQUIRE_SQUARE, "REQUIRE_SQUARE");
    m_options.emplace_back(&m_advanced_options, "");
    m_options.emplace_back(&CATCH_TO_OVERWORLD_DELAY, "CATCH_TO_OVERWORLD_DELAY");
    m_options.emplace_back(&TOUCH_DATE_INTERVAL, "TOUCH_DATE_INTERVAL");
}




std::string ShinyHuntAutonomousRegigigas2::Stats::stats() const{
    std::string str;
    str += str_encounters();
    str += " - Timeouts: " + tostr_u_commas(m_timeouts);
    str += str_shinies();
    return str;
}

ShinyHuntAutonomousRegigigas2::Tracker::Tracker(
    EncounterStats& stats,
    ProgramEnvironment& env,
    Logger& logger,
    ConsoleHandle& console,
    bool require_square,
    uint16_t exit_battle_time
)
    : StandardEncounterTracker(stats, console, require_square, exit_battle_time)
    , m_env(env)
    , m_logger(logger)
{}
bool ShinyHuntAutonomousRegigigas2::Tracker::run_away(){
    RaidCatchDetector detector(m_console, std::chrono::seconds(30));
    pbf_mash_button(BUTTON_A, 4 * TICKS_PER_SECOND);

    if (!detector.wait(m_env)){
        m_logger.log("Raid Catch Menu not found.", Qt::red);
        return false;
    }

    pbf_press_dpad(DPAD_DOWN, 10, 0);
    pbf_press_button(BUTTON_A, 10, m_exit_battle_time);
    return true;
}

bool ShinyHuntAutonomousRegigigas2::kill_and_return(SingleSwitchProgramEnvironment& env) const{
    RaidCatchDetector detector(env.console, std::chrono::seconds(30));
    pbf_mash_button(BUTTON_A, 4 * TICKS_PER_SECOND);

    if (!detector.wait(env)){
        env.logger.log("Raid Catch Menu not found.", Qt::red);
        return false;
    }

    pbf_press_dpad(DPAD_DOWN, 10, 0);
    pbf_press_button(BUTTON_A, 10, CATCH_TO_OVERWORLD_DELAY);
    return true;
}
void ShinyHuntAutonomousRegigigas2::program(SingleSwitchProgramEnvironment& env) const{
    grip_menu_connect_go_home();

    uint32_t last_touch = system_clock();
    if (TOUCH_DATE_INTERVAL > 0){
        touch_date_from_home(SETTINGS_TO_HOME_DELAY);
    }

    resume_game_back_out(TOLERATE_SYSTEM_UPDATE_MENU_FAST, 500);

    Stats stats;
    Tracker tracker(stats, env, env.logger, env.console, REQUIRE_SQUARE, 0);

    while (true){
        for (uint8_t pp = REVERSAL_PP; pp > 0; pp--){
            stats.log_stats(env, env.logger);

            env.logger.log("Starting Regigigas Encounter: " + tostr_u_commas(stats.encounters() + 1));

            pbf_mash_button(BUTTON_A, 10 * TICKS_PER_SECOND);
            env.console.botbase().wait_for_all_requests();

            ShinyEncounterDetector::Detection detection;
            {
                ShinyEncounterDetector detector(
                    env.console, env.logger,
                    ShinyEncounterDetector::RAID_BATTLE,
                    std::chrono::seconds(30)
                );
                detection = detector.detect(env);
            }

            if (tracker.process_result(detection)){
                goto StopProgram;
            }
            if (detection == ShinyEncounterDetector::NO_BATTLE_MENU || !tracker.run_away()){
                stats.m_timeouts++;
                break;
            }
        }

        pbf_press_button(BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
        if (TOUCH_DATE_INTERVAL > 0 && system_clock() - last_touch >= TOUCH_DATE_INTERVAL){
            touch_date_from_home(SETTINGS_TO_HOME_DELAY);
            last_touch += TOUCH_DATE_INTERVAL;
        }
        reset_game_from_home_with_inference(
            env, env.logger, env.console,
            TOLERATE_SYSTEM_UPDATE_MENU_FAST
        );
    }


StopProgram:
    stats.log_stats(env, env.logger);

    pbf_press_button(BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);

    end_program_callback();
    end_program_loop();
}



}
}
}


