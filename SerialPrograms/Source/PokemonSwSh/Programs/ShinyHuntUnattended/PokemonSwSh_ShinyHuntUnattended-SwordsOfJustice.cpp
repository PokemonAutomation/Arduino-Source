/*  ShinyHuntUnattended-SwordsOfJustice
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh_ShinyHuntTools.h"
#include "PokemonSwSh_ShinyHuntUnattended-SwordsOfJustice.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


ShinyHuntUnattendedSwordsOfJustice_Descriptor::ShinyHuntUnattendedSwordsOfJustice_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:ShinyHuntUnattendedSwordsOfJustice",
        STRING_POKEMON + " SwSh", "Shiny Hunt Unattended - Swords Of Justice",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/ShinyHuntUnattended-SwordsOfJustice.md",
        "Hunt for shiny SOJs. Stop when a shiny is found.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        true
    )
{}


ShinyHuntUnattendedSwordsOfJustice::ShinyHuntUnattendedSwordsOfJustice()
    : EXIT_CAMP_TO_RUN_DELAY0(
        "<b>Exit Camp to Run Delay:</b><br>This needs to be carefully calibrated.",
        LockMode::LOCK_WHILE_RUNNING,
        "15120 ms"
    )
    , AIRPLANE_MODE(
        "<b>Airplane Mode:</b><br>Enable if airplane mode is on.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , ENTER_CAMP_DELAY0(
        "<b>Enter Camp Delay:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        "8000 ms"
    )
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(TIME_ROLLBACK_HOURS);

    PA_ADD_OPTION(EXIT_CAMP_TO_RUN_DELAY0);
    PA_ADD_OPTION(AIRPLANE_MODE);
    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(ENTER_CAMP_DELAY0);
}



void ShinyHuntUnattendedSwordsOfJustice::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StartProgramChecks::check_performance_class_wired_or_wireless(context);

    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_no_interact(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
    }

    WallDuration PERIOD = std::chrono::hours(TIME_ROLLBACK_HOURS);
    WallClock last_touch = current_time();
    for (uint32_t c = 0; ; c++){
        //  Touch the date.
        if (TIME_ROLLBACK_HOURS > 0 && current_time() - last_touch >= PERIOD){
            pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0);
            rollback_hours_from_home(env.console, context, TIME_ROLLBACK_HOURS, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
            resume_game_no_interact(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
            last_touch += PERIOD;
        }

        //  Trigger encounter.
        pbf_press_button(context, BUTTON_X, 80ms, GameSettings::instance().OVERWORLD_TO_MENU_DELAY0);
        pbf_press_button(context, BUTTON_A, 80ms, ENTER_CAMP_DELAY0);
        if (AIRPLANE_MODE){
            pbf_press_button(context, BUTTON_A, 10, 100);
            pbf_press_button(context, BUTTON_A, 10, 100);
        }
        pbf_press_button(context, BUTTON_X, 10, 50);
        pbf_press_dpad(context, DPAD_LEFT, 10, 10);
        env.log("Starting Encounter: " + tostr_u_commas(c + 1));
        pbf_press_button(context, BUTTON_A, 80ms, EXIT_CAMP_TO_RUN_DELAY0);

        //  Run away if not shiny.
        run_away_with_lights(context);

        //  Enter Pokemon menu if shiny.
        enter_summary(context, false);
    }

//    pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE);
}



}
}
}
