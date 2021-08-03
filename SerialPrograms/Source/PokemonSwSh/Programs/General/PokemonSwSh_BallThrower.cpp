/*  Ball Thrower
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "PokemonSwSh_BallThrower.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


BallThrower_Descriptor::BallThrower_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:BallThrower",
        "Ball Thrower",
        "SwSh-Arduino/wiki/Basic:-BallThrower",
        "Blindly throw balls at the opposing " + STRING_POKEMON + " until it catches.",
        FeedbackType::NONE,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



BallThrower::BallThrower(const BallThrower_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
{
    m_options.emplace_back(&START_IN_GRIP_MENU, "START_IN_GRIP_MENU");
}

void BallThrower::program(SingleSwitchProgramEnvironment& env){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
        pbf_press_button(env.console, BUTTON_HOME, 10, HOME_TO_GAME_DELAY);
    }else{
        pbf_press_button(env.console, BUTTON_X, 5, 5);
    }

    while (true){
        pbf_press_button(env.console, BUTTON_X, 50, 50);
        pbf_press_button(env.console, BUTTON_A, 50, 50);
        pbf_mash_button(env.console, BUTTON_B, 100);
    }

    pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
    end_program_callback(env.console);
    end_program_loop(env.console);
}



}
}
}
