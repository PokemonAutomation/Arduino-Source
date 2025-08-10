/*  Watt Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh_DateSpam-WattFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Pokemon;


WattFarmer_Descriptor::WattFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:WattFarmer",
        STRING_POKEMON + " SwSh", "Date Spam - Watt Farmer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/DateSpam-WattFarmer.md",
        "Farm watts. (7.2 seconds/fetch, 1 million watts/hour with a tick-precise controller)",
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {ControllerFeature::NintendoSwitch_ProController},
        FasterIfTickPrecise::MUCH_FASTER
    )
{}



WattFarmer::WattFarmer()
    : GRIP_MENU_WAIT0(
        "<b>Exit Grip Menu Delay:</b><br>"
        "Wait this long after leaving the grip menu to allow for the Switch to reestablish local connection.",
        LockMode::LOCK_WHILE_RUNNING,
        "5000 ms"
    )
    , EXIT_DEN_WAIT(
        "<b>Exit Den Wait Time:</b><br>"
        "Wait this long after backing out of the den before date skipping.",
        LockMode::LOCK_WHILE_RUNNING,
        "1720 ms"
    )
    , SKIPS(
        "<b>Number of Fetch Attempts:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        33334
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
    PA_ADD_OPTION(GRIP_MENU_WAIT0);
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(EXIT_DEN_WAIT);
    PA_ADD_OPTION(SKIPS);
    PA_ADD_OPTION(SAVE_ITERATIONS0);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void WattFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        pbf_wait(context, GRIP_MENU_WAIT0);
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

        ssf_press_button_ptv(context, BUTTON_A, 40ms);
        pbf_mash_button(context, BUTTON_B, EXIT_DEN_WAIT);

        if (SAVE_ITERATIONS0 != 0){
            save_count++;
            if (save_count >= SAVE_ITERATIONS0){
                save_count = 0;
                pbf_mash_button(context, BUTTON_B, 2 * TICKS_PER_SECOND);
                pbf_press_button(context, BUTTON_X, 160ms, GameSettings::instance().OVERWORLD_TO_MENU_DELAY0);
                pbf_press_button(context, BUTTON_R, 20, 2 * TICKS_PER_SECOND);
                pbf_press_button(context, BUTTON_ZL, 20, 3 * TICKS_PER_SECOND);
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

