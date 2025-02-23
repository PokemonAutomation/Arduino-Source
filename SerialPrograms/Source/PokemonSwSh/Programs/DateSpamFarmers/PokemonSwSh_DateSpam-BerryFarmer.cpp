/*  Berry Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh_DateSpam-BerryFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


BerryFarmer_Descriptor::BerryFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:BerryFarmer",
        STRING_POKEMON + " SwSh", "Date Spam - Berry Farmer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/DateSpam-BerryFarmer.md",
        "Farm berries.",
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {ControllerFeature::NintendoSwitch_ProController},
        FasterIfTickPrecise::MUCH_FASTER
    )
{}



BerryFarmer::BerryFarmer()
    : SKIPS(
        "<b>Number of Fetch Attempts:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        100000
    )
    , SAVE_ITERATIONS(
        "<b>Save Every this Many Fetches:</b><br>(zero disables saving): ",
        LockMode::LOCK_WHILE_RUNNING,
        0
    )
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
    })
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(SKIPS);
    PA_ADD_OPTION(SAVE_ITERATIONS);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void BerryFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
        ssf_press_button_ptv(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_FAST0);
    }

    uint8_t year = MAX_YEAR;
    uint16_t save_count = 0;
    for (uint32_t c = 0; c < SKIPS; c++){
        env.log("Fetch Attempts: " + tostr_u_commas(c));

        home_roll_date_enter_game_autorollback(env.console, context, year);
        pbf_mash_button(context, BUTTON_B, 90);

        pbf_press_button(context, BUTTON_A, 10, 10);
        pbf_mash_button(context, BUTTON_ZL, 385);
        pbf_mash_button(context, BUTTON_B, 600);

        if (SAVE_ITERATIONS != 0){
            save_count++;
            if (save_count >= SAVE_ITERATIONS){
                save_count = 0;
                pbf_mash_button(context, BUTTON_B, 2 * TICKS_PER_SECOND);
                pbf_press_button(context, BUTTON_X, 160ms, GameSettings::instance().OVERWORLD_TO_MENU_DELAY0);
                pbf_press_button(context, BUTTON_R, 20, 2 * TICKS_PER_SECOND);
                pbf_press_button(context, BUTTON_ZL, 20, 3 * TICKS_PER_SECOND);
            }
        }

        //  Tap HOME and quickly spam B. The B spamming ensures that we don't
        //  accidentally update the system if the system update window pops up.
        ssf_press_button_ptv(context, BUTTON_HOME, 120ms);
        pbf_mash_button(context, BUTTON_B, GameSettings::instance().GAME_TO_HOME_DELAY_FAST0.get() - 120ms);
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}




}
}
}
