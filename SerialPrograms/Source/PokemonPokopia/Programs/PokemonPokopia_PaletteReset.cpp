/*  Palette Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include "CommonFramework/Logging/Logger.h"
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_HomeToDateTime.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonPokopia/Inference/PokemonPokopia_ButtonDetector.h"
#include "PokemonPokopia/Inference/PokemonPokopia_MovesDetection.h"
#include "PokemonPokopia/Inference/PokemonPokopia_PCDetection.h"
#include "PokemonPokopia/Inference/PokemonPokopia_SettingsScreenDetector.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "PokemonPokopia/PokemonPokopia_Settings.h"

#include "PokemonPokopia/Programs/PokemonPokopia_PCNavigation.h"
#include "PokemonPokopia/Programs/PokemonPokopia_PaletteReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonPokopia{

using namespace Pokemon;


PaletteReset_Descriptor::PaletteReset_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonPokopia:PaletteReset",
        STRING_POKEMON + " Pokopia", "Palette Reset",
        "Programs/PokemonPokopia/PaletteReset.html",
        "Reset Palette Town for Mew stamps and recipes.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}


PaletteReset::PaletteReset()
    : SKIPS(
          "<b>Number of Attempts:</b>",
          LockMode::UNLOCK_WHILE_RUNNING,
          1500
          )
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATIONS({
          &NOTIFICATION_PROGRAM_FINISH,
          &NOTIFICATION_ERROR_FATAL,
      })
{
    PA_ADD_OPTION(SKIPS);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}



void PaletteReset::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    //start in game facing a PC, with the A button visible,
    for (uint32_t c = 0; c < SKIPS; c++){
        env.log("Resets: " + tostr_u_commas(c));

        wait_for_overworld(env.console, context);


        access_pc_from_overworld(env.console, context, true);
        env.console.log("Opened Cloud Island PC menu");
        pbf_wait(context, 2000ms);

        Stamp todays_stamp = get_stamp(env.console, context, TODAYS_STAMP_BOX);
        if (todays_stamp == Stamp::MEW){
            env.log("Mew Stamp found.");
            GO_HOME_WHEN_DONE.run_end_of_program(context);
            send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
            break;
        }

        exit_pc(env.console, context);
    
        //  Date skip
        pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY0);
        home_to_date_time(env.console, context, true);

        pbf_press_button(context, BUTTON_A, 160ms, 840ms);
        pbf_press_button(context, BUTTON_A, 48ms, 24ms);//month
        pbf_move_left_joystick(context, {0,-1}, 48ms, 24ms);
        pbf_press_button(context, BUTTON_A, 48ms, 24ms);//day
        pbf_press_button(context, BUTTON_A, 48ms, 24ms);//year
        pbf_press_button(context, BUTTON_A, 48ms, 24ms);//hour
        pbf_press_button(context, BUTTON_A, 48ms, 24ms);//minute
        pbf_press_button(context, BUTTON_A, 48ms, 24ms);//am-pm
        pbf_press_button(context, BUTTON_A, 160ms, 840ms);
        
        pbf_press_button(context, BUTTON_HOME, 160ms, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
        //resume game
        resume_game_from_home(env.console, context);

        //  Date skip
        pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY0);
        home_to_date_time(env.console, context, true);

        pbf_press_button(context, BUTTON_A, 160ms, 840ms);
        pbf_press_button(context, BUTTON_A, 48ms, 24ms);//month
        pbf_move_left_joystick(context, {0,+1}, 48ms, 24ms);
        pbf_press_button(context, BUTTON_A, 48ms, 24ms);//day
        pbf_press_button(context, BUTTON_A, 48ms, 24ms);//year
        pbf_press_button(context, BUTTON_A, 48ms, 24ms);//hour
        pbf_press_button(context, BUTTON_A, 48ms, 24ms);//minute
        pbf_press_button(context, BUTTON_A, 48ms, 24ms);//am-pm
        pbf_press_button(context, BUTTON_A, 160ms, 840ms);

        pbf_press_button(context, BUTTON_HOME, 160ms, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
        //resume game
        resume_game_from_home(env.console, context);
        
        pbf_wait(context, 2000ms);

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}



}
}
}
}