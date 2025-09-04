/*  Shiny Hunt Autonomous - Berry Tree
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_HomeToDateTime.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_StartBattleDetector.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh/Programs/PokemonSwSh_EncounterHandler.h"
#include "PokemonSwSh_ShinyHuntAutonomous-BerryTree.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntAutonomousBerryTree_Descriptor::ShinyHuntAutonomousBerryTree_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:ShinyHuntAutonomousBerryTree",
        STRING_POKEMON + " SwSh", "Shiny Hunt Autonomous - Berry Tree",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/ShinyHuntAutonomous-BerryTree.md",
        "Automatically hunt for shiny berry tree " + STRING_POKEMON + " using video feedback.",
        ProgramControllerClass::StandardController_PerformanceClassSensitive,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
std::unique_ptr<StatsTracker> ShinyHuntAutonomousBerryTree_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new ShinyHuntTracker(true));
}



ShinyHuntAutonomousBerryTree::ShinyHuntAutonomousBerryTree()
    : GO_HOME_WHEN_DONE(false)
    , ENCOUNTER_BOT_OPTIONS(false, true)
    , NOTIFICATIONS({
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_NONSHINY,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_SHINY,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_CATCH_SUCCESS,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_CATCH_FAILED,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , EXIT_BATTLE_TIMEOUT0(
        "<b>Exit Battle Timeout:</b><br>After running, wait this long to return to overworld.",
        LockMode::LOCK_WHILE_RUNNING,
        "10 s"
    )
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);

    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(ENCOUNTER_BOT_OPTIONS);
    PA_ADD_OPTION(NOTIFICATIONS);

    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(EXIT_BATTLE_TIMEOUT0);
}




void ShinyHuntAutonomousBerryTree::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_no_interact(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
    }

    ShinyHuntTracker& stats = env.current_stats<ShinyHuntTracker>();
    env.update_stats();

    StandardEncounterHandler handler(
        env, env.console, context,
        LANGUAGE,
        ENCOUNTER_BOT_OPTIONS,
        stats
    );

    uint8_t year = MAX_YEAR;
    while (true){
        ssf_press_button(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_FAST0, 160ms);
        home_roll_date_enter_game_autorollback(env.console, context, year);
//        home_to_date_time(context, true, true);
//        neutral_date_skip(context);
//        settings_to_enter_game(context, true);
        pbf_mash_button(context, BUTTON_B, 90);
        context.wait_for_all_requests();

        {
            StandardBattleMenuWatcher battle_menu_detector(false);
            StartBattleWatcher start_battle_detector;

            int result = run_until<ProControllerContext>(
                env.console, context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 60s);
                },
                {
                    {battle_menu_detector},
                    {start_battle_detector},
                }
            );

            switch (result){
            case 0:
                env.log("Unexpected battle menu.", COLOR_RED);
                stats.add_error();
                env.update_stats();
                pbf_mash_button(context, BUTTON_B, 1000ms);
                run_away(env.console, context, EXIT_BATTLE_TIMEOUT0);
                continue;
            case 1:
                env.log("Battle started!");
                break;
            default:
                stats.add_error();
                env.update_stats();
                env.log("Timed out.");
                continue;
            }
        }

        //  Detect shiny.
        ShinyDetectionResult result = detect_shiny_battle(
            env.console, context,
            SHINY_BATTLE_REGULAR,
            std::chrono::seconds(30)
        );

        bool stop = handler.handle_standard_encounter_end_battle(
            result, EXIT_BATTLE_TIMEOUT0
        );
        if (stop){
            break;
        }
//        pbf_mash_button(context, BUTTON_B, 10s);
    }

    pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0);

    home_to_date_time(env.console, context, false);
    pbf_press_button(context, BUTTON_A, 5, 5);
    pbf_press_button(context, BUTTON_A, 5, 10);
    pbf_press_button(context, BUTTON_HOME, 80ms, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);

    if (!GO_HOME_WHEN_DONE){
        go_home(env.console, context);
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}




}
}
}
