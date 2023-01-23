/*  Shiny Hunt Autonomous - Swords Of Justice
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_StartBattleDetector.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh/Programs/PokemonSwSh_EncounterHandler.h"
#include "PokemonSwSh_ShinyHuntAutonomous-SwordsOfJustice.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntAutonomousSwordsOfJustice_Descriptor::ShinyHuntAutonomousSwordsOfJustice_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:ShinyHuntAutonomousSwordsOfJustice",
        STRING_POKEMON + " SwSh", "Shiny Hunt Autonomous - Swords Of Justice",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/ShinyHuntAutonomous-SwordsOfJustice.md",
        "Automatically hunt for shiny Sword of Justice using video feedback.",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
std::unique_ptr<StatsTracker> ShinyHuntAutonomousSwordsOfJustice_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(
        new ShinyHuntTracker(
            true,
            {{"Timeouts", "Errors"}}
        )
    );
}



ShinyHuntAutonomousSwordsOfJustice::ShinyHuntAutonomousSwordsOfJustice()
    : GO_HOME_WHEN_DONE(false)
    , AIRPLANE_MODE(
        "<b>Airplane Mode:</b><br>Enable if airplane mode is on.",
        LockWhileRunning::LOCKED,
        false
    )
    , ENCOUNTER_BOT_OPTIONS(true, false)
    , NOTIFICATIONS({
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_NONSHINY,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_SHINY,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , EXIT_BATTLE_TIMEOUT(
        "<b>Exit Battle Timeout:</b><br>After running, wait this long to return to overworld.",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "10 * TICKS_PER_SECOND"
    )
    , POST_BATTLE_MASH_TIME(
        "<b>Post-Battle Mash:</b><br>After each battle, mash B for this long before entering the camp.",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "1 * TICKS_PER_SECOND"
    )
    , ENTER_CAMP_DELAY(
        "<b>Enter Camp Delay:</b>",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "8 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(START_LOCATION);
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



void ShinyHuntAutonomousSwordsOfJustice::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_no_interact(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
    }

    const uint32_t PERIOD = (uint32_t)TIME_ROLLBACK_HOURS * 3600 * TICKS_PER_SECOND;
    uint32_t last_touch = system_clock(context);

    ShinyHuntTracker& stats = env.current_stats<ShinyHuntTracker>();
    env.update_stats();

    StandardEncounterHandler handler(
        env, env.console, context,
        LANGUAGE,
        ENCOUNTER_BOT_OPTIONS,
        stats
    );

    while (true){
        //  Touch the date.
        if (TIME_ROLLBACK_HOURS > 0 && system_clock(context) - last_touch >= PERIOD){
            pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE);
            rollback_hours_from_home(context, TIME_ROLLBACK_HOURS, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY);
            resume_game_no_interact(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
            last_touch += PERIOD;
        }

        //  Trigger encounter.
        pbf_mash_button(context, BUTTON_B, POST_BATTLE_MASH_TIME);
        pbf_press_button(context, BUTTON_X, 10, GameSettings::instance().OVERWORLD_TO_MENU_DELAY);
        pbf_press_button(context, BUTTON_A, 10, ENTER_CAMP_DELAY);
        if (AIRPLANE_MODE){
            pbf_press_button(context, BUTTON_A, 10, 100);
            pbf_press_button(context, BUTTON_A, 10, 100);
        }
        pbf_press_button(context, BUTTON_X, 10, 50);
        pbf_press_dpad(context, DPAD_LEFT, 10, 10);
        env.log("Starting Encounter: " + tostr_u_commas(stats.encounters() + 1));
        pbf_press_button(context, BUTTON_A, 10, 0);
        context.wait_for_all_requests();

        {
            //  Wait for start of battle.
            StandardBattleMenuWatcher battle_menu_detector(false);
            StartBattleWatcher start_back_detector;
            wait_until(
                env.console, context,
                std::chrono::seconds(30),
                {
                    {battle_menu_detector},
                    {start_back_detector},
                }
            );
        }

        //  Detect shiny.
        ShinyDetectionResult result = detect_shiny_battle(
            env.console, context,
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
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}





}
}
}
