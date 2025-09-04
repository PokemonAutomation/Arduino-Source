/*  Shiny Hunt Autonomous - Regi
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh/Programs/PokemonSwSh_EncounterHandler.h"
#include "PokemonSwSh/Programs/ShinyHuntUnattended/PokemonSwSh_ShinyHunt-Regi.h"
#include "PokemonSwSh_ShinyHuntAutonomous-Regi.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntAutonomousRegi_Descriptor::ShinyHuntAutonomousRegi_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:ShinyHuntAutonomousRegi",
        STRING_POKEMON + " SwSh", "Shiny Hunt Autonomous - Regi",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/ShinyHuntAutonomous-Regi.md",
        "Automatically hunt for shiny Regi using video feedback.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
std::unique_ptr<StatsTracker> ShinyHuntAutonomousRegi_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(
        new ShinyHuntTracker(
            true,
            {{"Light Resets", "Errors"}}
        )
    );
}



ShinyHuntAutonomousRegi::ShinyHuntAutonomousRegi()
    : GO_HOME_WHEN_DONE(false)
    , ENCOUNTER_BOT_OPTIONS(false, false)
    , NOTIFICATIONS({
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_NONSHINY,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_SHINY,
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
    , POST_BATTLE_MASH_TIME0(
        "<b>Post-Battle Mash:</b><br>After each battle, mash B for this long to clear the dialogs.",
        LockMode::LOCK_WHILE_RUNNING,
        "1000 ms"
    )
    , TRANSITION_DELAY0(
        "<b>Transition Delay:</b><br>Time to enter/exit the building.",
        LockMode::LOCK_WHILE_RUNNING,
        "5000 ms"
    )
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(TOUCH_DATE_INTERVAL);

    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(REGI_NAME);

    PA_ADD_OPTION(ENCOUNTER_BOT_OPTIONS);
    PA_ADD_OPTION(NOTIFICATIONS);

    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(EXIT_BATTLE_TIMEOUT0);
    PA_ADD_OPTION(POST_BATTLE_MASH_TIME0);
    PA_ADD_OPTION(TRANSITION_DELAY0);
}




void ShinyHuntAutonomousRegi::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_back_out(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 200);
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

    bool error = false;
    while (true){
        pbf_mash_button(context, BUTTON_B, POST_BATTLE_MASH_TIME0);
        move_to_corner(env.console, context, error, TRANSITION_DELAY0);
        if (error){
            env.update_stats();
            error = false;
        }

        //  Touch the date.
        if (TOUCH_DATE_INTERVAL.ok_to_touch_now()){
            env.log("Touching date to prevent rollover.");
            pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0);
            touch_date_from_home(env.console, context, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
            resume_game_no_interact(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
        }

        //  Do the light puzzle.
        run_regi_light_puzzle(env.console, context, REGI_NAME, stats.encounters());

        //  Start the encounter.
        pbf_mash_button(context, BUTTON_A, 5 * TICKS_PER_SECOND);
        context.wait_for_all_requests();

        //  Detect shiny.
        ShinyDetectionResult result = detect_shiny_battle(
            env.console, context,
            SHINY_BATTLE_REGULAR,
            std::chrono::seconds(30)
        );
//        shininess = ShinyDetection::SQUARE_SHINY;
        if (result.shiny_type == ShinyType::UNKNOWN){
            stats.add_error();
            pbf_mash_button(context, BUTTON_B, TICKS_PER_SECOND);
            run_away(env.console, context, EXIT_BATTLE_TIMEOUT0);
            error = true;
            continue;
        }

        bool stop = handler.handle_standard_encounter_end_battle(
            result, EXIT_BATTLE_TIMEOUT0
        );
        if (stop){
            break;
        }
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}




}
}
}


