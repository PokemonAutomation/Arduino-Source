/*  Shiny Hunt Autonomous - IoA Trade
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
#include "PokemonSwSh/ShinyHuntTracker.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SummaryShinySymbolDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_StartGame.h"
#include "PokemonSwSh_ShinyHuntAutonomous-IoATrade.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

ShinyHuntAutonomousIoATrade::ShinyHuntAutonomousIoATrade()
    : SingleSwitchProgram(
        FeedbackType::REQUIRED, PABotBaseLevel::PABOTBASE_12KB,
        "Shiny Hunt Autonomous - IoA Trade",
        "SerialPrograms/ShinyHuntAutonomous-IoATrade.md",
        "Hunt for shiny Isle of Armor trade using video feedback."
    )
    , GO_HOME_WHEN_DONE(
        "<b>Go Home when Done:</b><br>After finding a shiny, go to the Switch Home menu to idle. (turn this off for unattended streaming)",
        false
    )
    , TOUCH_DATE_INTERVAL(
        "<b>Rollover Prevention:</b><br>Prevent a den from rolling over by periodically touching the date. If set to zero, this feature is disabled.",
        "4 * 3600 * TICKS_PER_SECOND"
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , MASH_TO_TRADE_DELAY(
        "<b>Mash to Trade Delay:</b><br>Time to perform the trade.",
        "29 * TICKS_PER_SECOND"
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
    m_options.emplace_back(&TOUCH_DATE_INTERVAL, "TOUCH_DATE_INTERVAL");
    m_options.emplace_back(&m_advanced_options, "");
    m_options.emplace_back(&MASH_TO_TRADE_DELAY, "MASH_TO_TRADE_DELAY");
    if (settings.developer_mode){
        m_options.emplace_back(&VIDEO_ON_SHINY, "VIDEO_ON_SHINY");
        m_options.emplace_back(&RUN_FROM_EVERYTHING, "RUN_FROM_EVERYTHING");
    }
}




struct ShinyHuntAutonomousIoATrade::Stats : public ShinyHuntTracker{
    Stats()
        : ShinyHuntTracker(false)
        , m_errors(m_stats["Errors"])
    {
        m_display_order.insert(m_display_order.begin() + 1, Stat("Errors"));
    }
    uint64_t& m_errors;
};
std::unique_ptr<StatsTracker> ShinyHuntAutonomousIoATrade::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}




void ShinyHuntAutonomousIoATrade::program(SingleSwitchProgramEnvironment& env) const{
    grip_menu_connect_go_home(env.console);
    resume_game_back_out(env.console, TOLERATE_SYSTEM_UPDATE_MENU_FAST, 500);

    uint32_t last_touch = system_clock(env.console) - TOUCH_DATE_INTERVAL;

    Stats& stats = env.stats<Stats>();

    while (true){
        env.update_stats();

        pbf_press_button(env.console, BUTTON_A, 10, 100);
        pbf_press_button(env.console, BUTTON_A, 10, 60);
        pbf_press_button(env.console, BUTTON_A, 10, 100);
        pbf_press_button(env.console, BUTTON_A, 10, 50);
        pbf_press_button(env.console, BUTTON_A, 10, POKEMON_TO_BOX_DELAY);
        pbf_press_dpad(env.console, DPAD_LEFT, 10, 10);
        pbf_mash_button(env.console, BUTTON_A, MASH_TO_TRADE_DELAY);

        //  Enter box system.
        pbf_press_button(env.console, BUTTON_X, 10, OVERWORLD_TO_MENU_DELAY);
        pbf_press_dpad(env.console, DPAD_RIGHT, 10, 10);
        pbf_press_button(env.console, BUTTON_A, 10, MENU_TO_POKEMON_DELAY);

        //  View summary.
        pbf_press_button(env.console, BUTTON_A, 10, 100);
        pbf_press_button(env.console, BUTTON_A, 10, 0);
        env.console.botbase().wait_for_all_requests();

        SummaryShinySymbolDetector::Detection detection;
        {
            SummaryShinySymbolDetector detector(env.console, env.logger());
            detection = detector.wait_for_detection(env);
//            detection = SummaryShinySymbolDetector::SHINY;
        }
        switch (detection){
        case SummaryShinySymbolDetector::NO_DETECTION:
            stats.m_errors++;
            break;
        case SummaryShinySymbolDetector::NOT_SHINY:
            stats.add_non_shiny();
            break;
        case SummaryShinySymbolDetector::SHINY:
            stats.add_unknown_shiny();
            if (VIDEO_ON_SHINY){
                pbf_wait(env.console, 1 * TICKS_PER_SECOND);
                pbf_press_button(env.console, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);
            }
            if (!RUN_FROM_EVERYTHING){
                goto StopProgram;
            }
        }

        pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
        if (TOUCH_DATE_INTERVAL > 0 && system_clock(env.console) - last_touch >= TOUCH_DATE_INTERVAL){
            env.log("Touching date to prevent rollover.");
            touch_date_from_home(env.console, SETTINGS_TO_HOME_DELAY);
            last_touch += TOUCH_DATE_INTERVAL;
        }
        reset_game_from_home_with_inference(
            env, env.console,
            TOLERATE_SYSTEM_UPDATE_MENU_FAST
        );
    }

StopProgram:
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

