/*  Den Roller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "PokemonSwSh/Programs/PokemonSwSh_StartGame.h"
#include "PokemonSwSh_DenTools.h"
#include "PokemonSwSh_DenRoller.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

DenRoller::DenRoller()
    : SingleSwitchProgram(
        FeedbackType::OPTIONAL_, PABotBaseLevel::PABOTBASE_12KB,
        "Den Roller",
        "NativePrograms/DenRoller.md",
        "Roll den to the N'th day, SR and repeat."
    )
    , SKIPS(
        "<b>Number of Skips:</b>",
        3, 0, 60
    )
    , VIEW_TIME(
        "<b>View Time before Reset:</b>",
        "5 * TICKS_PER_SECOND"
    )
{
    m_options.emplace_back(&SKIPS, "SKIPS");
    m_options.emplace_back(&CATCHABILITY, "CATCHABILITY");
    m_options.emplace_back(&VIEW_TIME, "VIEW_TIME");
}


void DenRoller::ring_bell(const BotBaseContext& context, int count) const{
    for (int c = 0; c < count; c++){
        pbf_press_button(context, BUTTON_LCLICK, 5, 10);
    }
    pbf_wait(context, 200);
}

void DenRoller::program(SingleSwitchProgramEnvironment& env) const{
    grip_menu_connect_go_home(env.console);

    rollback_date_from_home(env.console, SKIPS);
    resume_game_front_of_den_nowatts(env.console, TOLERATE_SYSTEM_UPDATE_MENU_SLOW);

    while (true){
        roll_den(env.console, 0, 0, SKIPS, CATCHABILITY);

        ring_bell(env.console, 20);
        enter_den(env.console, 0, SKIPS != 0, false);

        //  Give user time to look at the mon.
        pbf_wait(env.console, VIEW_TIME);

        //  Add a little extra wait time since correctness matters here.
        pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE - 10);

        rollback_date_from_home(env.console, SKIPS);
//        reset_game_from_home(TOLERATE_SYSTEM_UPDATE_MENU_SLOW);
        reset_game_from_home_with_inference(
            env, env.console,
            TOLERATE_SYSTEM_UPDATE_MENU_SLOW
        );
    }

    end_program_callback(env.console);
    end_program_loop(env.console);
}



}
}
}
