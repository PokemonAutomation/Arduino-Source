/*  Stow-On-Side Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "Controllers/ControllerTypes.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh_DateSpam-StowOnSideFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


StowOnSideFarmer_Descriptor::StowOnSideFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:StowOnSideFarmer",
        STRING_POKEMON + " SwSh", "Date Spam - Stow-On-Side Farmer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/DateSpam-StowOnSideFarmer.md",
        "Farm the Stow-on-Side items dealer.",
        ProgramControllerClass::StandardController_PerformanceClassSensitive,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}



StowOnSideFarmer::StowOnSideFarmer()
    : SKIPS(
        "<b>Number of Purchase Attempts:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        100000
    )
    , SAVE_ITERATIONS0(
        "<b>Save Every this Many Fetches:</b><br>(zero disables saving): ",
        LockMode::LOCK_WHILE_RUNNING,
        100
    )
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
    })
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(SKIPS);
    PA_ADD_OPTION(SAVE_ITERATIONS0);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void StowOnSideFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
        ssf_press_button(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_FAST0, 160ms);
    }

    uint8_t year = MAX_YEAR;
    uint16_t save_count = 0;
    for (uint32_t c = 0; c < SKIPS; c++){
        env.log("Fetch Attempts: " + tostr_u_commas(c));
        home_roll_date_enter_game_autorollback(env.console, context, year);
        if (context->performance_class() == ControllerPerformanceClass::SysbotBase){
            pbf_wait(context, 90);
        }else{
            pbf_mash_button(context, BUTTON_B, 90);
        }

        ssf_press_button_ptv(context, BUTTON_A, 160ms);
        pbf_mash_button(context, BUTTON_ZL, 385);
        pbf_mash_button(context, BUTTON_B, 700);

        if (SAVE_ITERATIONS0 != 0){
            save_count++;
            if (save_count >= SAVE_ITERATIONS0){
                save_count = 0;
                pbf_mash_button(context, BUTTON_B, 2000ms);
                pbf_press_button(context, BUTTON_X, 160ms, GameSettings::instance().OVERWORLD_TO_MENU_DELAY0);
                pbf_press_button(context, BUTTON_R, 160ms, 2000ms);
                pbf_press_button(context, BUTTON_ZL, 160ms, 3000ms);
            }
        }

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
