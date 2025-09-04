/*  ShinyHuntUnattended-StrongSpawn
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh_ShinyHuntTools.h"
#include "PokemonSwSh_ShinyHuntUnattended-StrongSpawn.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


ShinyHuntUnattendedStrongSpawn_Descriptor::ShinyHuntUnattendedStrongSpawn_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:ShinyHuntUnattendedStrongSpawn",
        STRING_POKEMON + " SwSh", "Shiny Hunt Unattended - Strong Spawn",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/ShinyHuntUnattended-StrongSpawn.md",
        "Hunt for shiny strong spawns. Stop when a shiny is found.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        true
    )
{}



ShinyHuntUnattendedStrongSpawn::ShinyHuntUnattendedStrongSpawn()
    : ENTER_GAME_TO_RUN_DELAY0(
        "<b>Enter Game to Run Delay:</b><br>This needs to be carefully calibrated.",
        LockMode::LOCK_WHILE_RUNNING,
        "18240 ms"
    )
    , START_GAME_WAIT_DELAY0(
        "<b>Start Game Wait Delay:</b><br>Decrease this if your game starts quickly.",
        LockMode::LOCK_WHILE_RUNNING,
        "20 s"
    )
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(TIME_ROLLBACK_HOURS);

    PA_ADD_OPTION(ENTER_GAME_TO_RUN_DELAY0);
    PA_ADD_OPTION(START_GAME_WAIT_DELAY0);
}



void ShinyHuntUnattendedStrongSpawn::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StartProgramChecks::check_performance_class_wired_or_wireless(context);

    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
        ssf_press_button(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_FAST0, 160ms);
    }

    WallDuration PERIOD = std::chrono::hours(TIME_ROLLBACK_HOURS);
    WallClock last_touch = current_time();
    for (uint32_t c = 0; ; c++){

        //  If the update menu isn't there, these will get swallowed by the opening
        //  animation for the select user menu.
        if (ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST){
            pbf_press_button(context, BUTTON_A, 5, 35); //  Choose game
            pbf_press_dpad(context, DPAD_UP, 5, 0);     //  Skip the update window.
        }

        pbf_press_button(context, BUTTON_A, 10, 180);   //  Enter select user menu.
        pbf_press_button(context, BUTTON_A, 10, 10);    //  Enter game

        //  Switch to mashing ZR instead of A to get into the game.
        //  Mash your way into the game.
        Milliseconds duration = GameSettings::instance().START_GAME_MASH0;
        if (ConsoleSettings::instance().START_GAME_REQUIRES_INTERNET){
            //  Need to wait a bit longer for the internet check.
            duration += ConsoleSettings::instance().START_GAME_INTERNET_CHECK_DELAY0;
        }
        pbf_mash_button(context, BUTTON_ZR, duration);

        //  Wait for game to start.
        pbf_wait(context, START_GAME_WAIT_DELAY0);

        //  Enter game.
        env.log("Starting Encounter: " + tostr_u_commas(c + 1));
        pbf_press_button(context, BUTTON_A, 80ms, ENTER_GAME_TO_RUN_DELAY0);

        //  Run away.
        run_away_with_lights(context);

        //  Enter Pokemon menu if shiny.
        enter_summary(context, false);

        //  Touch the date and conditional close game.
        if (TIME_ROLLBACK_HOURS > 0 && current_time() - last_touch >= PERIOD){
            last_touch += PERIOD;
            close_game_if_overworld(env.console, context, false, TIME_ROLLBACK_HOURS);
        }else{
            close_game_if_overworld(env.console, context, false, 0);
        }

    }

//    pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE);
}


}
}
}
