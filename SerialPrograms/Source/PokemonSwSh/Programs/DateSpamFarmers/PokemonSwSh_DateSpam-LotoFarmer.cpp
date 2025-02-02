/*  Loto Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
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
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {SerialPABotBase::OLD_NINTENDO_SWITCH_DEFAULT_REQUIREMENTS}
    )
{}



LotoFarmer::LotoFarmer()
    : SKIPS(
        "<b>Number of Loto Attempts:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        100000
    )
    , MASH_B_DURATION(
        "<b>Mash B for this long to exit the dialog:</b><br>(Some languages like German need to increase this.)",
        LockMode::LOCK_WHILE_RUNNING,
        TICKS_PER_SECOND,
        "9 * TICKS_PER_SECOND"
    )
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
    })
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(SKIPS);
    PA_ADD_OPTION(MASH_B_DURATION);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void LotoFarmer::program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
        pbf_press_button(context, BUTTON_HOME, 80ms, GameSettings::instance().GAME_TO_HOME_DELAY_FAST0);
    }

    uint8_t year = MAX_YEAR;
    for (uint32_t c = 0; c < SKIPS; c++){
        env.log("Fetch Attempts: " + tostr_u_commas(c));
        home_roll_date_enter_game_autorollback(env.console, context, year);
        pbf_mash_button(context, BUTTON_B, 90);

        pbf_press_button(context, BUTTON_A, 10, 90);
        pbf_press_button(context, BUTTON_B, 10, 70);
        pbf_press_dpad(context, DPAD_DOWN, 10, 5);
        pbf_mash_button(context, BUTTON_ZL, 490);
        pbf_mash_button(context, BUTTON_B, MASH_B_DURATION);

        //  Tap HOME and quickly spam B. The B spamming ensures that we don't
        //  accidentally update the system if the system update window pops up.
        pbf_press_button(context, BUTTON_HOME, 10, 5);
        pbf_mash_button(context, BUTTON_B, GameSettings::instance().GAME_TO_HOME_DELAY_FAST0.get() - 120ms);
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}
}
}
