/*  ShinyHuntUnattended-IoATrade
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_Misc.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh_ShinyHuntTools.h"
#include "PokemonSwSh_ShinyHuntUnattended-IoATrade.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


ShinyHuntUnattendedIoATrade_Descriptor::ShinyHuntUnattendedIoATrade_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:ShinyHuntUnattendedIoATrade",
        STRING_POKEMON + " SwSh", "Shiny Hunt Unattended - IoA Trade",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/ShinyHuntUnattended-IoATrade.md",
        "Hunt for shiny Isle of Armor trade. Stop when a shiny is found.",
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_31KB
    )
{}



ShinyHuntUnattendedIoATrade::ShinyHuntUnattendedIoATrade()
    : START_TO_RUN_DELAY(
        "<b>Start to Run Delay:</b><br>This needs to be carefully calibrated.",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "1260"
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , FLY_DURATION(
        "<b>Fly Duration:</b>",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "800"
    )
    , MOVE_DURATION(
        "<b>Move to Beartic Duration:</b>",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "300"
    )
    , MASH_TO_TRADE_DELAY(
        "<b>Mash to Trade Delay:</b><br>Time to perform the trade.",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "30 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(TOUCH_DATE_INTERVAL);

    PA_ADD_OPTION(START_TO_RUN_DELAY);
    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(FLY_DURATION);
    PA_ADD_OPTION(MOVE_DURATION);
    PA_ADD_OPTION(MASH_TO_TRADE_DELAY);
}

void ShinyHuntUnattendedIoATrade::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_back_out(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 500);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
    }

    for (uint32_t c = 0; ; c++){
        env.log("Starting Trade: " + tostr_u_commas(c + 1));

        pbf_press_button(context, BUTTON_A, 10, 100);
        pbf_press_button(context, BUTTON_A, 10, 60);
        pbf_press_button(context, BUTTON_A, 10, 100);
        pbf_press_button(context, BUTTON_A, 10, 50);
        pbf_press_button(context, BUTTON_A, 10, GameSettings::instance().POKEMON_TO_BOX_DELAY);
        pbf_press_dpad(context, DPAD_LEFT, 10, 10);
        pbf_mash_button(context, BUTTON_A, MASH_TO_TRADE_DELAY);

        if (true){
            //  Enter box system.
            pbf_press_button(context, BUTTON_X, 10, GameSettings::instance().OVERWORLD_TO_MENU_DELAY);
            pbf_press_dpad(context, DPAD_RIGHT, 10, 10);
            pbf_press_button(context, BUTTON_A, 10, GameSettings::instance().MENU_TO_POKEMON_DELAY);

            //  Move item from 2nd party member to 1st.
            pbf_press_button(context, BUTTON_X, 10, 50);
            pbf_press_dpad(context, DPAD_DOWN, 10, 50);
            pbf_press_button(context, BUTTON_A, 10, 50);
            pbf_press_dpad(context, DPAD_UP, 10, 50);
            pbf_press_button(context, BUTTON_A, 10, 50);

            //  Back out to menu.
            //  Prepend each B press by a DOWN press so that the B gets
            //  swallowed while in the summary.
            IoA_backout(context, GameSettings::instance().POKEMON_TO_MENU_DELAY);

            //  Enter map.
            pbf_press_dpad(context, DPAD_LEFT, 10, 0);
            pbf_move_left_joystick(context, 128, 255, 10, 0);
        }else{
            pbf_press_dpad(context, DPAD_DOWN, 10, 50);
        }
        pbf_press_button(context, BUTTON_A, 10, 350);

        //  Fly to Route 10.
        pbf_press_button(context, BUTTON_L, 10, 100);
        pbf_press_button(context, BUTTON_L, 10, 100);
        pbf_press_dpad(context, DPAD_RIGHT, 15, 10);
        pbf_press_dpad(context, DPAD_DOWN, 30, 10);
        pbf_mash_button(context, BUTTON_A, FLY_DURATION);

        //  Move to Beartic.
        pbf_move_left_joystick(context, 240, 0, MOVE_DURATION, 0);

        pbf_wait(context, START_TO_RUN_DELAY);

        //  Run away.
        run_away_with_lights(context);

        //  Enter Pokemon menu if shiny.
        enter_summary(context, false);

        //  Conditional close game.
        close_game_if_overworld(
            context,
            TOUCH_DATE_INTERVAL.ok_to_touch_now(),
            0
        );

        start_game_from_home_with_inference(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 0, 0, false);
    }

//    pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE);
}



}
}
}
