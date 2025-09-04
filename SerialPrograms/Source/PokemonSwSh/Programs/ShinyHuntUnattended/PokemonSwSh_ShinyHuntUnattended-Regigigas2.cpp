/*  ShinyHuntUnattended-Regigigas2
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh_ShinyHuntTools.h"
#include "PokemonSwSh_ShinyHuntUnattended-Regigigas2.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


ShinyHuntUnattendedRegigigas2_Descriptor::ShinyHuntUnattendedRegigigas2_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:ShinyHuntUnattendedRegigigas2",
        STRING_POKEMON + " SwSh", "Shiny Hunt Unattended - Regigigas2",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/ShinyHuntUnattended-Regigigas2.md",
        "A new version of the Regigigas program that is faster.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        true
    )
{}



ShinyHuntUnattendedRegigigas2::ShinyHuntUnattendedRegigigas2()
    : REVERSAL_PP(
        "<b>Reversal PP:</b><br>The amount of Reversal PP you are saved with.",
        LockMode::LOCK_WHILE_RUNNING,
        24
    )
    , START_TO_ATTACK_DELAY0(
        "<b>Start to Attack Delay:</b><br>This needs to be carefully calibrated.",
        LockMode::LOCK_WHILE_RUNNING,
        "30000 ms"
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , ATTACK_TO_CATCH_DELAY0(
        "<b>Attack to Catch Delay:</b><br>Increase this if you seem to be catching Regigigas very often.",
        LockMode::LOCK_WHILE_RUNNING,
        "9000 ms"
    )
    , CATCH_TO_OVERWORLD_DELAY0(
        "<b>Catch to Overworld Delay:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        "8000 ms"
    )
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(TOUCH_DATE_INTERVAL);

    PA_ADD_OPTION(REVERSAL_PP);
    PA_ADD_OPTION(START_TO_ATTACK_DELAY0);
    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(ATTACK_TO_CATCH_DELAY0);
    PA_ADD_OPTION(CATCH_TO_OVERWORLD_DELAY0);
}

void ShinyHuntUnattendedRegigigas2::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StartProgramChecks::check_performance_class_wired_or_wireless(context);

    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_back_out(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 500);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
    }

    uint32_t encounter = 0;
    while (true){
        for (uint8_t pp = REVERSAL_PP; pp > 0; pp--){
            env.log("Starting Regigigas Encounter: " + tostr_u_commas(++encounter));

            pbf_press_button(context, BUTTON_A, 10, 3 * TICKS_PER_SECOND);
            pbf_press_button(context, BUTTON_A, 10, TICKS_PER_SECOND);
            pbf_press_button(context, BUTTON_A, 80ms, START_TO_ATTACK_DELAY0);

//            set_leds(context, true);
            pbf_press_button(context, BUTTON_A, 10, 2 * TICKS_PER_SECOND);
//            set_leds(context, false);

            //  Enter Pokemon menu if shiny.
            pbf_press_dpad(context, DPAD_DOWN, 10, 0);
            pbf_mash_button(context, BUTTON_A, 2 * TICKS_PER_SECOND);

            pbf_press_dpad(context, DPAD_DOWN, 10, 0);
            pbf_press_button(context, BUTTON_A, 10, TICKS_PER_SECOND);
            pbf_press_dpad(context, DPAD_DOWN, 10, 0);
            pbf_press_button(context, BUTTON_A, 10, TICKS_PER_SECOND);

            pbf_wait(context, ATTACK_TO_CATCH_DELAY0);
            pbf_press_dpad(context, DPAD_DOWN, 10, 0);
            pbf_press_button(context, BUTTON_A, 80ms, CATCH_TO_OVERWORLD_DELAY0);
        }

        //  Conditional close game.
        close_game_if_overworld(
            env.console,
            context,
            TOUCH_DATE_INTERVAL.ok_to_touch_now(),
            0
        );

        start_game_from_home_with_inference(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 0, 0, false);
    }

    pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0);
}



}
}
}
