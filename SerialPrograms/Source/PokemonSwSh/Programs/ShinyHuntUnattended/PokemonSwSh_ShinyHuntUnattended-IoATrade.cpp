/*  ShinyHuntUnattended-IoATrade
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShMisc.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/PokemonSwSh/PokemonSwShDateSpam.h"
#include "PokemonSwSh_ShinyHuntTools.h"
#include "PokemonSwSh_ShinyHuntUnattended-IoATrade.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntUnattendedIoATrade_Descriptor::ShinyHuntUnattendedIoATrade_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:ShinyHuntUnattendedIoATrade",
        "Shiny Hunt Unattended - IoA Trade",
        "SwSh-Arduino/wiki/Basic:-ShinyHuntUnattended-IoATrade",
        "Hunt for shiny Isle of Armor trade. Stop when a shiny is found.",
        FeedbackType::NONE,
        PABotBaseLevel::PABOTBASE_31KB
    )
{}



ShinyHuntUnattendedIoATrade::ShinyHuntUnattendedIoATrade(const ShinyHuntUnattendedIoATrade_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , START_TO_RUN_DELAY(
        "<b>Start to Run Delay:</b><br>This needs to be carefully calibrated.",
        "1260"
    )
    , TOUCH_DATE_INTERVAL(
        "<b>Rollover Prevention:</b><br>Prevent a den from rolling over by periodically touching the date. If set to zero, this feature is disabled.",
        "4 * 3600 * TICKS_PER_SECOND"
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , FLY_DURATION(
        "<b>Fly Duration:</b>",
        "800"
    )
    , MOVE_DURATION(
        "<b>Move to Beartic Duration:</b>",
        "300"
    )
    , MASH_TO_TRADE_DELAY(
        "<b>Mash to Trade Delay:</b><br>Time to perform the trade.",
        "29 * TICKS_PER_SECOND"
    )
{
    m_options.emplace_back(&START_IN_GRIP_MENU, "START_IN_GRIP_MENU");

    m_options.emplace_back(&START_TO_RUN_DELAY, "START_TO_RUN_DELAY");
    m_options.emplace_back(&TOUCH_DATE_INTERVAL, "TOUCH_DATE_INTERVAL");
    m_options.emplace_back(&m_advanced_options, "");
    m_options.emplace_back(&FLY_DURATION, "FLY_DURATION");
    m_options.emplace_back(&MOVE_DURATION, "MOVE_DURATION");
    m_options.emplace_back(&MASH_TO_TRADE_DELAY, "MASH_TO_TRADE_DELAY");
}

void ShinyHuntUnattendedIoATrade::program(SingleSwitchProgramEnvironment& env){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
        resume_game_back_out(env.console, TOLERATE_SYSTEM_UPDATE_MENU_FAST, 500);
    }else{
        pbf_press_button(env.console, BUTTON_B, 5, 5);
    }

    uint32_t last_touch = system_clock(env.console) - TOUCH_DATE_INTERVAL;
    for (uint32_t c = 0; ; c++){
        env.log("Starting Trade: " + tostr_u_commas(c + 1));

        pbf_press_button(env.console, BUTTON_A, 10, 100);
        pbf_press_button(env.console, BUTTON_A, 10, 60);
        pbf_press_button(env.console, BUTTON_A, 10, 100);
        pbf_press_button(env.console, BUTTON_A, 10, 50);
        pbf_press_button(env.console, BUTTON_A, 10, POKEMON_TO_BOX_DELAY);
        pbf_press_dpad(env.console, DPAD_LEFT, 10, 10);
        pbf_mash_button(env.console, BUTTON_A, MASH_TO_TRADE_DELAY);

        if (true){
            //  Enter box system.
            pbf_press_button(env.console, BUTTON_X, 10, OVERWORLD_TO_MENU_DELAY);
            pbf_press_dpad(env.console, DPAD_RIGHT, 10, 10);
            pbf_press_button(env.console, BUTTON_A, 10, MENU_TO_POKEMON_DELAY);

            //  Move item from 2nd party member to 1st.
            pbf_press_button(env.console, BUTTON_X, 10, 50);
            pbf_press_dpad(env.console, DPAD_DOWN, 10, 50);
            pbf_press_button(env.console, BUTTON_A, 10, 50);
            pbf_press_dpad(env.console, DPAD_UP, 10, 50);
            pbf_press_button(env.console, BUTTON_A, 10, 50);

            //  Back out to menu.
            //  Prepend each B press by a DOWN press so that the B gets
            //  swallowed while in the summary.
            IoA_backout(env.console, POKEMON_TO_MENU_DELAY);

            //  Enter map.
            pbf_press_dpad(env.console, DPAD_LEFT, 10, 0);
            pbf_move_left_joystick(env.console, 128, 255, 10, 0);
        }else{
            pbf_press_dpad(env.console, DPAD_DOWN, 10, 50);
        }
        pbf_press_button(env.console, BUTTON_A, 10, 350);

        //  Fly to Route 10.
        pbf_press_button(env.console, BUTTON_L, 10, 100);
        pbf_press_button(env.console, BUTTON_L, 10, 100);
        pbf_press_dpad(env.console, DPAD_RIGHT, 15, 10);
        pbf_press_dpad(env.console, DPAD_DOWN, 30, 10);
        pbf_mash_button(env.console, BUTTON_A, FLY_DURATION);

        //  Move to Beartic.
        pbf_move_left_joystick(env.console, 240, 0, MOVE_DURATION, 0);

        pbf_wait(env.console, START_TO_RUN_DELAY);

        //  Run away.
        run_away_with_lights(env.console);

        //  Enter Pokemon menu if shiny.
        enter_summary(env.console, false);

        //  Touch the date and conditional close game.
        if (TOUCH_DATE_INTERVAL > 0 && system_clock(env.console) - last_touch >= TOUCH_DATE_INTERVAL){
            last_touch += TOUCH_DATE_INTERVAL;
            close_game_if_overworld(env.console, true, 0);
        }else{
            close_game_if_overworld(env.console, false, 0);
        }

        start_game_from_home(env.console, TOLERATE_SYSTEM_UPDATE_MENU_FAST, 0, 0, false);
    }

    pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
    end_program_callback(env.console);
    end_program_loop(env.console);
}



}
}
}
