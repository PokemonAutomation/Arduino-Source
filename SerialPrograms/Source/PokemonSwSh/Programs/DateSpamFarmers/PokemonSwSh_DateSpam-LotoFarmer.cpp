/*  Loto Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "Controllers/ControllerTypes.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh_DateSpam-LotoFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


LotoFarmer_Descriptor::LotoFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:LotoFarmer",
        STRING_POKEMON + " SwSh", "Date Spam - Loto Farmer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/DateSpam-LotoFarmer.md",
        "Farm the Loto ID.",
        ProgramControllerClass::StandardController_PerformanceClassSensitive,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}



LotoFarmer::LotoFarmer()
    : SKIPS(
        "<b>Number of Loto Attempts:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        100000
    )
    , MASH_B_DURATION0(
        "<b>Mash B for this long to exit the dialog:</b><br>(Some languages like German need to increase this.)",
        LockMode::LOCK_WHILE_RUNNING,
        "9000 ms"
    )
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
    })
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(SKIPS);
    PA_ADD_OPTION(MASH_B_DURATION0);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void LotoFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
        ssf_press_button(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_FAST0, 160ms);
    }

    uint8_t year = MAX_YEAR;
    for (uint32_t c = 0; c < SKIPS; c++){
        env.log("Fetch Attempts: " + tostr_u_commas(c));
        home_roll_date_enter_game_autorollback(env.console, context, year);
        if (context->performance_class() == ControllerPerformanceClass::SysbotBase){
            pbf_wait(context, 90);
        }else{
            pbf_mash_button(context, BUTTON_B, 90);
        }

        pbf_press_button(context, BUTTON_A, 10, 90);
        pbf_press_button(context, BUTTON_B, 10, 70);
        ssf_press_dpad_ptv(context, DPAD_DOWN, 120ms);
        pbf_mash_button(context, BUTTON_ZL, 490);
        pbf_mash_button(context, BUTTON_B, MASH_B_DURATION0);

        //  Tap HOME and quickly spam B. The B spamming ensures that we don't
        //  accidentally update the system if the system update window pops up.
        ssf_press_button(context, BUTTON_HOME, 120ms, 160ms);
        pbf_mash_button(context, BUTTON_B, GameSettings::instance().GAME_TO_HOME_DELAY_FAST0.get() - 120ms);
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}
}
}
