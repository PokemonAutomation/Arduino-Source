/*  Shiny Hunt Autonomous - IoA Trade
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/PokemonSwSh/PokemonSwShDateSpam.h"
#include "CommonFramework/PersistentSettings.h"
#include "Pokemon/Pokemon_Notification.h"
#include "PokemonSwSh/ShinyHuntTracker.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SummaryShinySymbolDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_StartGame.h"
#include "PokemonSwSh_ShinyHuntAutonomous-IoATrade.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntAutonomousIoATrade_Descriptor::ShinyHuntAutonomousIoATrade_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:ShinyHuntAutonomousIoATrade",
        "Shiny Hunt Autonomous - IoA Trade",
        "SwSh-Arduino/wiki/Advanced:-ShinyHuntAutonomous-IoATrade",
        "Hunt for shiny Isle of Armor trade using video feedback.",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



ShinyHuntAutonomousIoATrade::ShinyHuntAutonomousIoATrade(const ShinyHuntAutonomousIoATrade_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , MASH_TO_TRADE_DELAY(
        "<b>Mash to Trade Delay:</b><br>Time to perform the trade.",
        "30 * TICKS_PER_SECOND"
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
    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(TOUCH_DATE_INTERVAL);

    PA_ADD_OPTION(NOTIFICATION_LEVEL);
    PA_ADD_OPTION(NOTIFICATION_SCREENSHOT);

    PA_ADD_OPTION(m_advanced_options);
    PA_ADD_OPTION(MASH_TO_TRADE_DELAY);
    if (PERSISTENT_SETTINGS().developer_mode){
        PA_ADD_OPTION(VIDEO_ON_SHINY);
        PA_ADD_OPTION(RUN_FROM_EVERYTHING);
    }
}



std::unique_ptr<StatsTracker> ShinyHuntAutonomousIoATrade::make_stats() const{
    return std::unique_ptr<StatsTracker>(new ShinyHuntTracker(false));
}




void ShinyHuntAutonomousIoATrade::program(SingleSwitchProgramEnvironment& env){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
        resume_game_back_out(env.console, TOLERATE_SYSTEM_UPDATE_MENU_FAST, 500);
    }else{
        pbf_press_button(env.console, BUTTON_B, 5, 5);
    }

    uint32_t last_touch = system_clock(env.console) - TOUCH_DATE_INTERVAL;

    ShinyHuntTracker& stats = env.stats<ShinyHuntTracker>();

    EncounterNotificationSender notification_sender(NOTIFICATION_LEVEL);

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
            SummaryShinySymbolDetector detector(env.console, env.console);
            detection = detector.wait_for_detection(env, env.console);
//            detection = SummaryShinySymbolDetector::SHINY;
        }
        switch (detection){
        case SummaryShinySymbolDetector::NO_DETECTION:
            stats.add_error();
            break;
        case SummaryShinySymbolDetector::NOT_SHINY:
            stats.add_non_shiny();
            notification_sender.send_notification(
                env.console,
                m_descriptor.display_name(),
                nullptr,
                ShinyDetectionResult{ShinyType::NOT_SHINY, QImage()},
                ScreenshotMode::NO_SCREENSHOT,
                &stats
            );
            break;
        case SummaryShinySymbolDetector::SHINY:
            stats.add_unknown_shiny();
            notification_sender.send_notification(
                env.console,
                m_descriptor.display_name(),
                nullptr,
                ShinyDetectionResult{ShinyType::UNKNOWN_SHINY, QImage()},
                ScreenshotMode::NO_SCREENSHOT,
                &stats
            );
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

