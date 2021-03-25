/*  ShinyHuntAutonomous-IoATrade
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
#include "PokemonSwSh/Inference/PokemonSwSh_SummaryShinySymbolDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_StartGame.h"
#include "PokemonSwSh_ShinyHuntAutonomous-IoATrade.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

ShinyHuntAutonomousIoATrade::ShinyHuntAutonomousIoATrade()
    : SingleSwitchProgram(
        FeedbackType::REQUIRED, PABotBaseLevel::PABOTBASE_12KB,
        "Shiny Hunt Autonomous - IoATrade",
        "SerialPrograms/ShinyHuntAutonomous-IoATrade.md",
        "Hunt for shiny Isle of Armor trade using video feedback."
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , MASH_TO_TRADE_DELAY(
        "<b>Mash to Trade Delay:</b><br>Time to perform the trade.",
        "29 * TICKS_PER_SECOND"
    )
    , TOUCH_DATE_INTERVAL(
        "<b>Rollover Prevention:</b><br>Prevent a den from rolling over by periodically touching the date. If set to zero, this feature is disabled.",
        "4 * 3600 * TICKS_PER_SECOND"
    )
{
    m_options.emplace_back(&m_advanced_options, "");
    m_options.emplace_back(&MASH_TO_TRADE_DELAY, "MASH_TO_TRADE_DELAY");
    m_options.emplace_back(&TOUCH_DATE_INTERVAL, "TOUCH_DATE_INTERVAL");
}



std::string ShinyHuntAutonomousIoATrade::Stats::stats() const{
    std::string str;
    str += "Trades: " + tostr_u_commas(m_encounters);
    str += " - Errors: " + tostr_u_commas(m_errors);
    str += str_shinies();
    return str;
}


void ShinyHuntAutonomousIoATrade::program(SingleSwitchProgramEnvironment& env) const{
    grip_menu_connect_go_home();
    resume_game_back_out(TOLERATE_SYSTEM_UPDATE_MENU_FAST, 500);

    uint32_t last_touch = system_clock() - TOUCH_DATE_INTERVAL;

    Stats stats;

    while (true){
        stats.log_stats(env, env.logger);

        pbf_press_button(BUTTON_A, 10, 100);
        pbf_press_button(BUTTON_A, 10, 50);
        pbf_press_button(BUTTON_A, 10, 100);
        pbf_press_button(BUTTON_A, 10, 50);
        pbf_press_button(BUTTON_A, 10, POKEMON_TO_BOX_DELAY);
        pbf_press_dpad(DPAD_LEFT, 10, 10);
        pbf_mash_button(BUTTON_A, MASH_TO_TRADE_DELAY);

        //  Enter box system.
        pbf_press_button(BUTTON_X, 10, OVERWORLD_TO_MENU_DELAY);
        pbf_press_dpad(DPAD_RIGHT, 10, 10);
        pbf_press_button(BUTTON_A, 10, MENU_TO_POKEMON_DELAY);

        //  View summary.
        pbf_press_button(BUTTON_A, 10, 100);
        pbf_press_button(BUTTON_A, 10, 0);
        env.console.botbase().wait_for_all_requests();

        SummaryShinySymbolDetector::Detection detection;
        {
            SummaryShinySymbolDetector detector(env.console, env.logger);
            detection = detector.wait_for_detection(env);
        }
        switch (detection){
        case SummaryShinySymbolDetector::NO_DETECTION:
            stats.m_errors++;
            break;
        case SummaryShinySymbolDetector::NOT_SHINY:
            stats.add_non_shiny();
            break;
        case SummaryShinySymbolDetector::SHINY:
            stats.add_shiny();
            pbf_wait(1 * TICKS_PER_SECOND);
            pbf_press_button(BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);
            goto StopProgram;
        }

        pbf_press_button(BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
        if (TOUCH_DATE_INTERVAL > 0 && system_clock() - last_touch >= TOUCH_DATE_INTERVAL){
            env.logger.log("Touching date to prevent rollover.");
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

