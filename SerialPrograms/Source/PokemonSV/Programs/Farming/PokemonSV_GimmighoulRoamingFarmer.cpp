/*  Gimmighoul Roaming Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_HomeToDateTime.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_RollDateForward1.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_RollDateBackwardN.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV_GimmighoulRoamingFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;

GimmighoulRoamingFarmer_Descriptor::GimmighoulRoamingFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:GimmighoulRoamingFarmer",
        STRING_POKEMON + " SV", "Gimmighoul Roaming Farmer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/GimmighoulRoamingFarmer.md",
        "Farm roaming Gimmighoul for coins.",
        ProgramControllerClass::StandardController_PerformanceClassSensitive,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

GimmighoulRoamingFarmer::GimmighoulRoamingFarmer()
    : SKIPS(
        "<b>Number of Attempts:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        1500
    )
    , GO_HOME_WHEN_DONE(false)
    , FIX_TIME_WHEN_DONE(
        "<b>Fix Time when Done:</b><br>Fix the time after the program finishes.",
        LockMode::UNLOCK_WHILE_RUNNING, false
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

void GimmighoulRoamingFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    //  Start in game facing a roaming Gimmighoul somewhere safe. (ex. Pokemon Center since wild Pokemon can't fight you there.)
    uint8_t year = MAX_YEAR;
    for (uint32_t c = 0; c < SKIPS; c++){
        //  Grab coin assuming there is one
        env.log("Fetch Attempts: " + tostr_u_commas(c));
        pbf_mash_button(context, BUTTON_A, 90);
        pbf_wait(context, 2 * TICKS_PER_SECOND);

        //  Save the game
        save_game_from_overworld(env.program_info(), env.console, context);

        //  Date skip
        pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY1);
        home_to_date_time(env.console, context, true);
        if (year >= MAX_YEAR){
            roll_date_backward_N(env.console, context, MAX_YEAR, true);
            year = 0;
        }else{
            roll_date_forward_1(env.console, context, true);
            year++;
        }

        pbf_press_button(context, BUTTON_HOME, 160ms, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);

        //  Reset game
        reset_game_from_home(env.program_info(), env.console, context, 5 * TICKS_PER_SECOND);
    }

    if (FIX_TIME_WHEN_DONE){
        go_home(env.console, context);
        home_to_date_time(env.console, context, false);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_HOME, 160ms, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
        resume_game_from_home(env.console, context);
    }
    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}
}
}

