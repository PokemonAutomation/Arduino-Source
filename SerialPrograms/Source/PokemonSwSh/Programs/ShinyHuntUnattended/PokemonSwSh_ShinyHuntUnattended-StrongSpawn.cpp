/*  ShinyHuntUnattended-StrongSpawn
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh_ShinyHuntTools.h"
#include "PokemonSwSh_ShinyHuntUnattended-StrongSpawn.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntUnattendedStrongSpawn_Descriptor::ShinyHuntUnattendedStrongSpawn_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:ShinyHuntUnattendedStrongSpawn",
        STRING_POKEMON + " SwSh", "Shiny Hunt Unattended - Strong Spawn",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/ShinyHuntUnattended-StrongSpawn.md",
        "Hunt for shiny strong spawns. Stop when a shiny is found.",
        FeedbackType::NONE, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



ShinyHuntUnattendedStrongSpawn::ShinyHuntUnattendedStrongSpawn(const ShinyHuntUnattendedStrongSpawn_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , ENTER_GAME_TO_RUN_DELAY(
        "<b>Enter Game to Run Delay:</b><br>This needs to be carefully calibrated.",
        "2280"
    )
    , START_GAME_WAIT_DELAY(
        "<b>Start Game Wait Delay:</b><br>Decrease this if your game starts quickly.",
        "20 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(TIME_ROLLBACK_HOURS);

    PA_ADD_OPTION(ENTER_GAME_TO_RUN_DELAY);
    PA_ADD_OPTION(START_GAME_WAIT_DELAY);
}



void ShinyHuntUnattendedStrongSpawn::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
    }else{
        pbf_press_button(env.console, BUTTON_B, 5, 5);
        pbf_press_button(env.console, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_FAST);
    }

    const uint32_t PERIOD = (uint32_t)TIME_ROLLBACK_HOURS * 3600 * TICKS_PER_SECOND;
    uint32_t last_touch = system_clock(env.console);
    for (uint32_t c = 0; ; c++){

        //  If the update menu isn't there, these will get swallowed by the opening
        //  animation for the select user menu.
        if (ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST){
            pbf_press_button(env.console, BUTTON_A, 5, 35); //  Choose game
            pbf_press_dpad(env.console, DPAD_UP, 5, 0);     //  Skip the update window.
        }

        pbf_press_button(env.console, BUTTON_A, 10, 180);   //  Enter select user menu.
        pbf_press_button(env.console, BUTTON_A, 10, 10);    //  Enter game

        //  Switch to mashing ZR instead of A to get into the game.
        //  Mash your way into the game.
        uint16_t duration = GameSettings::instance().START_GAME_MASH;
        if (ConsoleSettings::instance().START_GAME_REQUIRES_INTERNET){
            //  Need to wait a bit longer for the internet check.
            duration += ConsoleSettings::instance().START_GAME_INTERNET_CHECK_DELAY;
        }
        pbf_mash_button(env.console, BUTTON_ZR, duration);

        //  Wait for game to start.
        pbf_wait(env.console, START_GAME_WAIT_DELAY);

        //  Enter game.
        env.log("Starting Encounter: " + tostr_u_commas(c + 1));
        pbf_press_button(env.console, BUTTON_A, 10, ENTER_GAME_TO_RUN_DELAY);

        //  Run away.
        run_away_with_lights(env.console);

        //  Enter Pokemon menu if shiny.
        enter_summary(env.console, false);

        //  Touch the date and conditional close game.
//        if (true){
        if (TIME_ROLLBACK_HOURS > 0 && system_clock(env.console) - last_touch >= PERIOD){
            last_touch += PERIOD;
            close_game_if_overworld(env.console, false, TIME_ROLLBACK_HOURS);
        }else{
            close_game_if_overworld(env.console, false, 0);
        }

    }

    pbf_press_button(env.console, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE);
}


}
}
}
