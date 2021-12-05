/*  Shiny Hunt Autonomous - IoA Trade
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Device.h"
#include "NintendoSwitch/Commands/NintendoSwitch_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Pokemon_Notification.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
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
        STRING_POKEMON + " SwSh", "Shiny Hunt Autonomous - IoA Trade",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/ShinyHuntAutonomous-IoATrade.md",
        "Hunt for shiny Isle of Armor trade using video feedback.",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



ShinyHuntAutonomousIoATrade::ShinyHuntAutonomousIoATrade(const ShinyHuntAutonomousIoATrade_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , VIDEO_ON_SHINY(
        "<b>Video Capture:</b><br>Take a video of the encounter if it is shiny.",
        true
    )
    , NOTIFICATION_NONSHINY(
        "Non-Shiny Encounter",
        true, false,
        {"Notifs"},
        std::chrono::seconds(3600)
    )
    , NOTIFICATION_SHINY(
        "Shiny Encounter",
        true, true, ImageAttachmentMode::JPG,
        {"Notifs", "Showcase"}
    )
//    , NOTIFICATION_PROGRAM_FINISH("Program Finished", true, true)
    , NOTIFICATIONS({
        &NOTIFICATION_NONSHINY,
        &NOTIFICATION_SHINY,
//        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_PROGRAM_ERROR,
    })
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , MASH_TO_TRADE_DELAY(
        "<b>Mash to Trade Delay:</b><br>Time to perform the trade.",
        "30 * TICKS_PER_SECOND"
    )
    , RUN_FROM_EVERYTHING(
        "<b>Run from Everything:</b><br>Run from everything - even if it is shiny. (For testing only.)",
        false
    )
{
    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(TOUCH_DATE_INTERVAL);

    PA_ADD_OPTION(VIDEO_ON_SHINY);
    PA_ADD_OPTION(NOTIFICATIONS);

    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(MASH_TO_TRADE_DELAY);
    if (GlobalSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(RUN_FROM_EVERYTHING);
    }
}



std::unique_ptr<StatsTracker> ShinyHuntAutonomousIoATrade::make_stats() const{
    return std::unique_ptr<StatsTracker>(new ShinyHuntTracker(false));
}




void ShinyHuntAutonomousIoATrade::program(SingleSwitchProgramEnvironment& env){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
        resume_game_back_out(env.console, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 500);
    }else{
        pbf_press_button(env.console, BUTTON_B, 5, 5);
    }

    ShinyHuntTracker& stats = env.stats<ShinyHuntTracker>();

    while (true){
        env.update_stats();

        pbf_press_button(env.console, BUTTON_A, 10, 100);
        pbf_press_button(env.console, BUTTON_A, 10, 60);
        pbf_press_button(env.console, BUTTON_A, 10, 100);
        pbf_press_button(env.console, BUTTON_A, 10, 50);
        pbf_press_button(env.console, BUTTON_A, 10, GameSettings::instance().POKEMON_TO_BOX_DELAY);
        pbf_press_dpad(env.console, DPAD_LEFT, 10, 10);
        pbf_mash_button(env.console, BUTTON_A, MASH_TO_TRADE_DELAY);

        //  Enter box system.
        pbf_press_button(env.console, BUTTON_X, 10, GameSettings::instance().OVERWORLD_TO_MENU_DELAY);
        pbf_press_dpad(env.console, DPAD_RIGHT, 10, 10);
        pbf_press_button(env.console, BUTTON_A, 10, GameSettings::instance().MENU_TO_POKEMON_DELAY);

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
            send_encounter_notification(
                env.console,
                NOTIFICATION_NONSHINY,
                NOTIFICATION_SHINY,
                env.program_info(),
                false, false, {{{}, ShinyType::NOT_SHINY}},
                QImage(),
                &stats
            );
            break;
        case SummaryShinySymbolDetector::SHINY:
            stats.add_unknown_shiny();
            send_encounter_notification(
                env.console,
                NOTIFICATION_NONSHINY,
                NOTIFICATION_SHINY,
                env.program_info(),
                false, true, {{{}, ShinyType::UNKNOWN_SHINY}},
                env.console.video().snapshot(),
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

        pbf_press_button(env.console, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE);
        TOUCH_DATE_INTERVAL.touch_now_from_home_if_needed(env.console);
        reset_game_from_home_with_inference(
            env, env.console,
            ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST
        );
    }

StopProgram:
    env.update_stats();
    GO_HOME_WHEN_DONE.run_end_of_program(env.console);
}


}
}
}

