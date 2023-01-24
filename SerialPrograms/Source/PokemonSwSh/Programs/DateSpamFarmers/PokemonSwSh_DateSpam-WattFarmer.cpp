/*  Watt Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
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
        "Farm watts. (7.2 seconds/fetch, 1 million watts/hour)",
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



WattFarmer::WattFarmer()
    : GRIP_MENU_WAIT(
        "<b>Exit Grip Menu Delay:</b> "
        "Wait this long after leaving the grip menu to allow for the Switch to reestablish local connection.",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "5 * TICKS_PER_SECOND"
    )
    , SKIPS(
        "<b>Number of Fetch Attempts:</b>",
        LockWhileRunning::LOCKED,
        33334
    )
    , SAVE_ITERATIONS(
        "<b>Save Every this Many Fetches:</b><br>(zero disables saving): ",
        LockWhileRunning::LOCKED,
        0, 0
    )
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(SKIPS);
    PA_ADD_OPTION(SAVE_ITERATIONS);
}

void WattFarmer::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        pbf_wait(context, GRIP_MENU_WAIT);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
        pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_FAST);
    }

    uint8_t year = MAX_YEAR;
    uint16_t save_count = 0;
    for (uint32_t c = 0; c < SKIPS; c++){
        env.log("Fetch Attempts: " + tostr_u_commas(c));

        home_roll_date_enter_game_autorollback(env.console, context, year);
        pbf_mash_button(context, BUTTON_B, 90);

        pbf_press_button(context, BUTTON_A, 5, 5);
        pbf_mash_button(context, BUTTON_B, 215);

        if (SAVE_ITERATIONS != 0){
            save_count++;
            if (save_count >= SAVE_ITERATIONS){
                save_count = 0;
                pbf_mash_button(context, BUTTON_B, 2 * TICKS_PER_SECOND);
                pbf_press_button(context, BUTTON_X, 20, GameSettings::instance().OVERWORLD_TO_MENU_DELAY);
                pbf_press_button(context, BUTTON_R, 20, 2 * TICKS_PER_SECOND);
                pbf_press_button(context, BUTTON_ZL, 20, 3 * TICKS_PER_SECOND);
            }
        }

        //  Tap HOME and quickly spam B. The B spamming ensures that we don't
        //  accidentally update the system if the system update window pops up.
        pbf_press_button(context, BUTTON_HOME, 10, 5);
        pbf_mash_button(context, BUTTON_B, GameSettings::instance().GAME_TO_HOME_DELAY_FAST - 15);
    }
}


}
}
}

