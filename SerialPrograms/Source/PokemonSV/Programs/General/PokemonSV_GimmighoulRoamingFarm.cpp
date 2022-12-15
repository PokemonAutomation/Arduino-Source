/*  Gimmighoul Roaming Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Routines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV_GimmighoulRoamingFarm.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonSV {

using namespace Pokemon;

GimmighoulRoamingFarm_Descriptor::GimmighoulRoamingFarm_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:GimmighoulRoamingFarm",
        STRING_POKEMON + " SV", "Gimmighoul Roaming Farmer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/GimmighoulRoamingFarmer.md",
        "Farm roaming Gimmighoul for coins.",
        FeedbackType::REQUIRED, true,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}

GimmighoulRoamingFarm::GimmighoulRoamingFarm()
    : SKIPS(
        "<b>Number of Attempts:</b>",
        LockWhileRunning::UNLOCKED,
        1500
    )
    , GO_HOME_WHEN_DONE(false)
    , FIX_TIME_WHEN_DONE(
        "<b>Fix time when done:</b><br>Fix the time after the program finishes.",
        LockWhileRunning::UNLOCKED, false
    )
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
        })
{
    PA_ADD_OPTION(SKIPS);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(FIX_TIME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void GimmighoulRoamingFarm::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) {
    //Start in game facing a roaming Gimmighoul somewhere safe. (ex. Pokemon Center since wild Pokemon can't fight you there.)
    for (uint32_t c = 0; c < SKIPS; c++) {
        //Grab coin assuming there is one
        env.log("Fetch Attempts: " + tostr_u_commas(c));
        pbf_mash_button(context, BUTTON_A, 90);
        pbf_wait(context, 2 * TICKS_PER_SECOND);

        //Save the game then close it
        save_game_from_overworld(env.console, context);
        pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
        context.wait_for_all_requests();
        close_game(context);

        //Date skip
        home_to_date_time(context, true, false);
        roll_date_forward_1(context, false);
        pbf_press_button(context, BUTTON_HOME, 10, 90);

        //Enter game
        reset_game_from_home(env, env.console, context, 5 * TICKS_PER_SECOND);

    }

    if (FIX_TIME_WHEN_DONE) {
        pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY);
        home_to_date_time(context, false, false);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_HOME, 20, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY);
        pbf_press_button(context, BUTTON_HOME, 20, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY);
    }
    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}
}
}

