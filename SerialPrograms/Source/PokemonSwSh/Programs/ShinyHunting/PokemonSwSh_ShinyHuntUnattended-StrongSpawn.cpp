/*  ShinyHuntUnattended-StrongSpawn
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Clientside/PrettyPrint.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "PokemonSwSh_ShinyHuntTools.h"
#include "PokemonSwSh_ShinyHuntUnattended-StrongSpawn.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntUnattendedStrongSpawn::ShinyHuntUnattendedStrongSpawn()
    : SingleSwitchProgram(
        FeedbackType::NONE, PABotBaseLevel::PABOTBASE_12KB,
        "Shiny Hunt Unattended - Strong Spawn",
        "NativePrograms/ShinyHuntUnattended-StrongSpawn.md",
        "Hunt for shiny strong spawns. Stop when a shiny is found."
    )
    , ENTER_GAME_TO_RUN_DELAY(
        "<b>Enter Game to Run Delay:</b><br>This needs to be carefully calibrated.",
        "2280"
    )
    , START_GAME_WAIT_DELAY(
        "<b>Start Game Wait Delay:</b><br>Decrease this if your game starts quickly.",
        "20 * TICKS_PER_SECOND"
    )
    , TIME_ROLLBACK_HOURS(
        "<b>Time Rollback (in hours):</b><br>Periodically roll back the time to keep the weather the same. If set to zero, this feature is disabled.",
        1, 0, 11
    )
{
    m_options.emplace_back(&ENTER_GAME_TO_RUN_DELAY, "ENTER_GAME_TO_RUN_DELAY");
    m_options.emplace_back(&START_GAME_WAIT_DELAY, "START_GAME_WAIT_DELAY");
    m_options.emplace_back(&TIME_ROLLBACK_HOURS, "TIME_ROLLBACK_HOURS");
}



void ShinyHuntUnattendedStrongSpawn::program(SingleSwitchProgramEnvironment& env) const{
    grip_menu_connect_go_home(env.console);
//    resume_game_no_interact(env.console, false);

    const uint32_t PERIOD = (uint32_t)TIME_ROLLBACK_HOURS * 3600 * TICKS_PER_SECOND;
    uint32_t last_touch = system_clock(env.console);
    for (uint32_t c = 0; ; c++){

        //  If the update menu isn't there, these will get swallowed by the opening
        //  animation for the select user menu.
        if (TOLERATE_SYSTEM_UPDATE_MENU_FAST){
            pbf_press_button(env.console, BUTTON_A, 5, 35); //  Choose game
            pbf_press_dpad(env.console, DPAD_UP, 5, 0);     //  Skip the update window.
        }

        pbf_press_button(env.console, BUTTON_A, 10, 180);   //  Enter select user menu.
        pbf_press_button(env.console, BUTTON_A, 10, 10);    //  Enter game

        //  Switch to mashing ZR instead of A to get into the game.
        //  Mash your way into the game.
        uint16_t duration = START_GAME_MASH;
        if (START_GAME_REQUIRES_INTERNET){
            //  Need to wait a bit longer for the internet check.
            duration += START_GAME_INTERNET_CHECK_DELAY;
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

    pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
    end_program_callback(env.console);
    end_program_loop(env.console);
}


}
}
}
