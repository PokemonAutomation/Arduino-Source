/*  Shiny Hunt Autonomous - IoA Trade
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Pokemon_Notification.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/ShinyHuntTracker.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SummaryShinySymbolDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh_ShinyHuntAutonomous-IoATrade.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntAutonomousIoATrade_Descriptor::ShinyHuntAutonomousIoATrade_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:ShinyHuntAutonomousIoATrade",
        STRING_POKEMON + " SwSh", "Shiny Hunt Autonomous - IoA Trade",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/ShinyHuntAutonomous-IoATrade.md",
        "Hunt for shiny Isle of Armor trade using video feedback.",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
std::unique_ptr<StatsTracker> ShinyHuntAutonomousIoATrade_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new ShinyHuntTracker(false));
}



ShinyHuntAutonomousIoATrade::ShinyHuntAutonomousIoATrade()
    : GO_HOME_WHEN_DONE(false)
    , VIDEO_ON_SHINY(
        "<b>Video Capture:</b><br>Take a video of the encounter if it is shiny.",
        LockWhileRunning::LOCKED,
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
        &NOTIFICATION_ERROR_FATAL,
    })
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , MASH_TO_TRADE_DELAY(
        "<b>Mash to Trade Delay:</b><br>Time to perform the trade.",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "30 * TICKS_PER_SECOND"
    )
    , RUN_FROM_EVERYTHING(
        "<b>Run from Everything:</b><br>Run from everything - even if it is shiny. (For testing only.)",
        LockWhileRunning::LOCKED,
        false
    )
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(TOUCH_DATE_INTERVAL);

    PA_ADD_OPTION(VIDEO_ON_SHINY);
    PA_ADD_OPTION(NOTIFICATIONS);

    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(MASH_TO_TRADE_DELAY);
    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(RUN_FROM_EVERYTHING);
    }
}


void ShinyHuntAutonomousIoATrade::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_back_out(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 500);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
    }

    ShinyHuntTracker& stats = env.current_stats<ShinyHuntTracker>();

    while (true){
        env.update_stats();

        pbf_press_button(context, BUTTON_A, 10, 100);
        pbf_press_button(context, BUTTON_A, 10, 60);
        pbf_press_button(context, BUTTON_A, 10, 100);
        pbf_press_button(context, BUTTON_A, 10, 50);
        pbf_press_button(context, BUTTON_A, 10, GameSettings::instance().POKEMON_TO_BOX_DELAY);
        pbf_press_dpad(context, DPAD_LEFT, 10, 10);
        pbf_mash_button(context, BUTTON_A, MASH_TO_TRADE_DELAY);

        //  Enter box system.
        pbf_press_button(context, BUTTON_X, 10, GameSettings::instance().OVERWORLD_TO_MENU_DELAY);
        pbf_press_dpad(context, DPAD_RIGHT, 10, 10);
        pbf_press_button(context, BUTTON_A, 10, GameSettings::instance().MENU_TO_POKEMON_DELAY);

        //  View summary.
        pbf_press_button(context, BUTTON_A, 10, 100);
        pbf_press_button(context, BUTTON_A, 10, 0);
        context.wait_for_all_requests();

        SummaryShinySymbolDetector::Detection detection;
        {
            SummaryShinySymbolDetector detector(env.console, env.console);
            detection = detector.wait_for_detection(context, env.console);
//            detection = SummaryShinySymbolDetector::SHINY;
        }
        switch (detection){
        case SummaryShinySymbolDetector::NO_DETECTION:
            stats.add_error();
            break;
        case SummaryShinySymbolDetector::NOT_SHINY:
            stats.add_non_shiny();
            send_encounter_notification(
                env,
                NOTIFICATION_NONSHINY,
                NOTIFICATION_SHINY,
                false, false, {{{}, ShinyType::NOT_SHINY}}, std::nan(""),
                ImageViewRGB32()
            );
            break;
        case SummaryShinySymbolDetector::SHINY:
            stats.add_unknown_shiny();
            send_encounter_notification(
                env,
                NOTIFICATION_NONSHINY,
                NOTIFICATION_SHINY,
                false, true, {{{}, ShinyType::UNKNOWN_SHINY}}, std::nan(""),
                env.console.video().snapshot()
            );
            if (VIDEO_ON_SHINY){
                pbf_wait(context, 1 * TICKS_PER_SECOND);
                pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);
            }
            if (!RUN_FROM_EVERYTHING){
                goto StopProgram;
            }
        }

        pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE);
        TOUCH_DATE_INTERVAL.touch_now_from_home_if_needed(context);
        reset_game_from_home_with_inference(
            env.console, context,
            ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST
        );
    }

StopProgram:
    env.update_stats();
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}


}
}
}

