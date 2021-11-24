/*  Shiny Hunt Autonomous - Swords Of Justice
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
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
#include "PokemonSwSh_ShinyHuntAutonomous-SwordsOfJustice.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntAutonomousSwordsOfJustice_Descriptor::ShinyHuntAutonomousSwordsOfJustice_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:ShinyHuntAutonomousSwordsOfJustice",
        "Shiny Hunt Autonomous - Swords Of Justice",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/ShinyHuntAutonomous-SwordsOfJustice.md",
        "Automatically hunt for shiny Sword of Justice using video feedback.",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



ShinyHuntAutonomousSwordsOfJustice::ShinyHuntAutonomousSwordsOfJustice(const ShinyHuntAutonomousSwordsOfJustice_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , AIRPLANE_MODE(
        "<b>Airplane Mode:</b><br>Enable if airplane mode is on.",
        false
    )
    , ENCOUNTER_BOT_OPTIONS(true, false)
    , NOTIFICATION_PROGRAM_FINISH("Program Finished", true, true)
    , NOTIFICATIONS({
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_NONSHINY,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_SHINY,
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
    , POST_BATTLE_MASH_TIME(
        "<b>Post-Battle Mash:</b><br>After each battle, mash B for this long before entering the camp.",
        "1 * TICKS_PER_SECOND"
    )
    , ENTER_CAMP_DELAY(
        "<b>Enter Camp Delay:</b>",
        "8 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(TIME_ROLLBACK_HOURS);

    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(AIRPLANE_MODE);
    PA_ADD_OPTION(ENCOUNTER_BOT_OPTIONS);
    PA_ADD_OPTION(NOTIFICATIONS);

    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(EXIT_BATTLE_TIMEOUT);
    PA_ADD_OPTION(POST_BATTLE_MASH_TIME);
    PA_ADD_OPTION(ENTER_CAMP_DELAY);
}



std::unique_ptr<StatsTracker> ShinyHuntAutonomousSwordsOfJustice::make_stats() const{
    return std::unique_ptr<StatsTracker>(
        new ShinyHuntTracker(
            true,
            {{"Timeouts", "Errors"}}
        )
    );
}




void ShinyHuntAutonomousSwordsOfJustice::program(SingleSwitchProgramEnvironment& env){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
        resume_game_no_interact(env.console, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
    }else{
        pbf_press_button(env.console, BUTTON_B, 5, 5);
    }

    const uint32_t PERIOD = (uint32_t)TIME_ROLLBACK_HOURS * 3600 * TICKS_PER_SECOND;
    uint32_t last_touch = system_clock(env.console);

    ShinyHuntTracker& stats = env.stats<ShinyHuntTracker>();
    env.update_stats();

    StandardEncounterHandler handler(
        env, env.console,
        LANGUAGE,
        ENCOUNTER_BOT_OPTIONS,
        stats
    );

    while (true){
        //  Touch the date.
        if (TIME_ROLLBACK_HOURS > 0 && system_clock(env.console) - last_touch >= PERIOD){
            pbf_press_button(env.console, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE);
            rollback_hours_from_home(env.console, TIME_ROLLBACK_HOURS, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY);
            resume_game_no_interact(env.console, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
            last_touch += PERIOD;
        }

        //  Trigger encounter.
        pbf_mash_button(env.console, BUTTON_B, POST_BATTLE_MASH_TIME);
        pbf_press_button(env.console, BUTTON_X, 10, GameSettings::instance().OVERWORLD_TO_MENU_DELAY);
        pbf_press_button(env.console, BUTTON_A, 10, ENTER_CAMP_DELAY);
        if (AIRPLANE_MODE){
            pbf_press_button(env.console, BUTTON_A, 10, 100);
            pbf_press_button(env.console, BUTTON_A, 10, 100);
        }
        pbf_press_button(env.console, BUTTON_X, 10, 50);
        pbf_press_dpad(env.console, DPAD_LEFT, 10, 10);
        env.log("Starting Encounter: " + tostr_u_commas(stats.encounters() + 1));
        pbf_press_button(env.console, BUTTON_A, 10, 0);
        env.console.botbase().wait_for_all_requests();

        {
            //  Wait for start of battle.
            StandardBattleMenuDetector battle_menu_detector(false);
            StartBattleDetector start_back_detector(env.console);
            wait_until(
                env, env.console,
                std::chrono::seconds(30),
                {
                    &battle_menu_detector,
                    &start_back_detector,
                }
            );
        }

        //  Detect shiny.
        ShinyDetectionResult result = detect_shiny_battle(
            env.console,
            env, env.console, env.console,
            SHINY_BATTLE_REGULAR,
            std::chrono::seconds(30)
        );
//        shininess = ShinyDetection::SQUARE_SHINY;

        bool stop = handler.handle_standard_encounter_end_battle(result, EXIT_BATTLE_TIMEOUT);
        if (stop){
            break;
        }
    }

    env.update_stats();

    if (GO_HOME_WHEN_DONE){
        pbf_press_button(env.console, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE);
    }

    send_program_finished_notification(
        env.logger(), NOTIFICATION_PROGRAM_FINISH,
        env.program_info(),
        "",
        stats.to_str()
    );

    end_program_callback(env.console);
    end_program_loop(env.console);
}





}
}
}
