/*  ShinyHuntUnattended-Regigigas2
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
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
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



ShinyHuntUnattendedRegigigas2::ShinyHuntUnattendedRegigigas2()
    : REVERSAL_PP(
        "<b>Reversal PP:</b><br>The amount of Reversal PP you are saved with.",
        LockWhileRunning::LOCKED,
        24
    )
    , START_TO_ATTACK_DELAY(
        "<b>Start to Attack Delay:</b><br>This needs to be carefully calibrated.",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "3750"
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , ATTACK_TO_CATCH_DELAY(
        "<b>Attack to Catch Delay:</b><br>Increase this if you seem to be catching Regigigas very often.",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "9 * TICKS_PER_SECOND"
    )
    , CATCH_TO_OVERWORLD_DELAY(
        "<b>Catch to Overworld Delay:</b>",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "8 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(TOUCH_DATE_INTERVAL);

    PA_ADD_OPTION(REVERSAL_PP);
    PA_ADD_OPTION(START_TO_ATTACK_DELAY);
    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(ATTACK_TO_CATCH_DELAY);
    PA_ADD_OPTION(CATCH_TO_OVERWORLD_DELAY);
}

void ShinyHuntUnattendedRegigigas2::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
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
            pbf_press_button(context, BUTTON_A, 10, START_TO_ATTACK_DELAY);

            set_leds(context, true);
            pbf_press_button(context, BUTTON_A, 10, 2 * TICKS_PER_SECOND);
            set_leds(context, false);

            //  Enter Pokemon menu if shiny.
            pbf_press_dpad(context, DPAD_DOWN, 10, 0);
            pbf_mash_button(context, BUTTON_A, 2 * TICKS_PER_SECOND);

            pbf_press_dpad(context, DPAD_DOWN, 10, 0);
            pbf_press_button(context, BUTTON_A, 10, TICKS_PER_SECOND);
            pbf_press_dpad(context, DPAD_DOWN, 10, 0);
            pbf_press_button(context, BUTTON_A, 10, TICKS_PER_SECOND);

            pbf_wait(context, ATTACK_TO_CATCH_DELAY);
            pbf_press_dpad(context, DPAD_DOWN, 10, 0);
            pbf_press_button(context, BUTTON_A, 10, CATCH_TO_OVERWORLD_DELAY);
        }

        //  Conditional close game.
        close_game_if_overworld(
            context,
            TOUCH_DATE_INTERVAL.ok_to_touch_now(),
            0
        );

        start_game_from_home_with_inference(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 0, 0, false);
    }

    pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE);
}



}
}
}
