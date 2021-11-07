/*  Shiny Hunt Autonomous - Berry Tree
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Device.h"
#include "NintendoSwitch/Commands/NintendoSwitch_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_StartBattleDetector.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_EncounterHandler.h"
#include "PokemonSwSh_ShinyHuntAutonomous-BerryTree.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntAutonomousBerryTree_Descriptor::ShinyHuntAutonomousBerryTree_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:ShinyHuntAutonomousBerryTree",
        "Shiny Hunt Autonomous - Berry Tree",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/ShinyHuntAutonomous-BerryTree.md",
        "Automatically hunt for shiny berry tree " + STRING_POKEMON + " using video feedback.",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



ShinyHuntAutonomousBerryTree::ShinyHuntAutonomousBerryTree(const ShinyHuntAutonomousBerryTree_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , ENCOUNTER_BOT_OPTIONS(false, true)
    , NOTIFICATION_PROGRAM_FINISH("Program Finished", true, true)
    , NOTIFICATIONS({
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_NONSHINY,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_SHINY,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_CATCH_SUCCESS,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_CATCH_FAILED,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_PROGRAM_ERROR,
    })
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , EXIT_BATTLE_TIMEOUT(
        "<b>Exit Battle Timeout:</b><br>After running, wait this long to return to overworld.",
        "10 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);

    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(ENCOUNTER_BOT_OPTIONS);
    PA_ADD_OPTION(NOTIFICATIONS);

    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(EXIT_BATTLE_TIMEOUT);
}



std::unique_ptr<StatsTracker> ShinyHuntAutonomousBerryTree::make_stats() const{
    return std::unique_ptr<StatsTracker>(new ShinyHuntTracker(true));
}




void ShinyHuntAutonomousBerryTree::program(SingleSwitchProgramEnvironment& env){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
        resume_game_no_interact(env.console, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
    }else{
        pbf_press_button(env.console, BUTTON_B, 5, 5);
    }

    ShinyHuntTracker& stats = env.stats<ShinyHuntTracker>();
    env.update_stats();

    StandardEncounterHandler handler(
        m_descriptor.display_name(),
        env, env.console,
        LANGUAGE,
        ENCOUNTER_BOT_OPTIONS,
        stats
    );

    uint8_t year = MAX_YEAR;
    while (true){
        pbf_press_button(env.console, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_FAST);
        home_roll_date_enter_game_autorollback(env.console, &year);
//        home_to_date_time(env.console, true, true);
//        neutral_date_skip(env.console);
//        settings_to_enter_game(env.console, true);
        pbf_mash_button(env.console, BUTTON_B, 90);
        env.console.botbase().wait_for_all_requests();

        {
            StandardBattleMenuDetector battle_menu_detector(false);
            StartBattleDetector start_battle_detector(env.console);

            int result = run_until(
                env, env.console,
                [](const BotBaseContext& context){
                    pbf_mash_button(context, BUTTON_A, 60 * TICKS_PER_SECOND);
                },
                {
                    &battle_menu_detector,
                    &start_battle_detector,
                }
            );

            switch (result){
            case 0:
                env.log("Unexpected battle menu.", Qt::red);
                stats.add_error();
                env.update_stats();
                pbf_mash_button(env.console, BUTTON_B, TICKS_PER_SECOND);
                run_away(env, env.console, EXIT_BATTLE_TIMEOUT);
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
            env.console,
            env, env.console, env.console,
            SHINY_BATTLE_REGULAR,
            std::chrono::seconds(30)
        );

        bool stop = handler.handle_standard_encounter_end_battle(result, EXIT_BATTLE_TIMEOUT);
        if (stop){
            break;
        }
//        pbf_mash_button(env.console, BUTTON_B, 10 * TICKS_PER_SECOND);
    }

    pbf_press_button(env.console, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE);

    home_to_date_time(env.console, false, false);
    pbf_press_button(env.console, BUTTON_A, 5, 5);
    pbf_press_button(env.console, BUTTON_A, 5, 10);
    pbf_press_button(env.console, BUTTON_HOME, 10, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY);

    if (!GO_HOME_WHEN_DONE){
        pbf_press_button(env.console, BUTTON_HOME, 10, GameSettings::instance().HOME_TO_GAME_DELAY);
    }

    send_program_finished_notification(
        env.logger(), NOTIFICATION_PROGRAM_FINISH,
        descriptor().display_name(),
        "",
        stats.to_str()
    );

    end_program_callback(env.console);
    end_program_loop(env.console);
}




}
}
}
