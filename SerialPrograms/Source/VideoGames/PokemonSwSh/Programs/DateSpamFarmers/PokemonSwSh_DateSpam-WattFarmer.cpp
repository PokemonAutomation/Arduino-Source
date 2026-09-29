/*  Watt Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "Controllers/ControllerTypes.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_HomeToDateTime.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_RollDateForward1.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "VideoGames/PokemonSwSh/PokemonSwSh_Settings.h"
#include "VideoGames/PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "VideoGames/PokemonSwSh/Options/PokemonSwSh_Catchability.h"
#include "VideoGames/PokemonSwSh/Programs/Hosting/PokemonSwSh_DenTools.h"
#include "PokemonSwSh_DateSpam-WattFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Pokemon;


WattFarmer_Descriptor::WattFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:WattFarmer",
        STRING_POKEMON + " SwSh", "Date Spam - Watt Farmer",
        "Programs/PokemonSwSh/DateSpam-WattFarmer.html",
        "Farm watts. (7.2 seconds/fetch, 1 million watts/hour with a tick-precise controller)",
        ProgramControllerClass::StandardController_PerformanceClassSensitive,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
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
    , YCOMM_GLITCH_ENABLED(
        "<b>Y-COMM Glitch is Enabled:</b><br>"
        "Uncheck this box to allow this program to function without enabling the Y-COMM glitch. "
        "However, this will drastically slow down the program.",
        LockMode::LOCK_WHILE_RUNNING,
        true
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
    PA_ADD_OPTION(YCOMM_GLITCH_ENABLED);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void WattFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        pbf_wait(context, GRIP_MENU_WAIT0);
    }else{
        //  Connect the controller.
        require_player(env.console, context, BUTTON_B);
        ssf_press_button(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_FAST0, 160ms);
    }

    uint8_t year = MAX_YEAR;
    uint16_t save_count = 0;

    if (!YCOMM_GLITCH_ENABLED){
        bool has_watts = false;

        //  First subdivide the number of skips into batches of 60 because the roll_den function only goes up to 60.
        for (uint32_t c = 0; c < SKIPS; c++){
            //  Skip forward.
            home_roll_date_enter_game_autorollback(env.console, context, year);

            //  Exit Raid
            ssf_press_button(context, BUTTON_B, 960ms, 400ms);
            ssf_press_button(context, BUTTON_A, GameSettings::instance().REENTER_DEN_DELAY0, 400ms);
            has_watts = true;

            //  1st iteration is unclear if there are watts.
            if (c < 1){
                pbf_mash_button(context, BUTTON_B, EXIT_DEN_WAIT);
                ssf_press_button_ptv(context, BUTTON_A, 40ms);
                pbf_mash_button(context, BUTTON_B, EXIT_DEN_WAIT);
                has_watts = false;
            }

            // Save after X skips
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

            enter_den(context, 0ms, has_watts, false);
            enter_lobby(context, 0ms, false, Catchability::ALWAYS_CATCHABLE);

            ssf_press_button(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_FAST0, 80ms);
        }
    }else{
        //  First subdivide the number of skips into batches of 60 because the roll_den function only goes up to 60.
        for (uint32_t c = 0; c < SKIPS; c++){
            env.log("Fetch Attempts: " + tostr_u_commas(c));

            home_roll_date_enter_game_autorollback(env.console, context, year);
            pbf_mash_button(context, BUTTON_B, 720ms);

            ssf_press_button_ptv(context, BUTTON_A, 40ms);
            pbf_mash_button(context, BUTTON_B, EXIT_DEN_WAIT);

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
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}
}
}

