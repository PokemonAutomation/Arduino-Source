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
        "NativePrograms/BallThrower.md",
        "Blindly throw balls at the opposing " + STRING_POKEMON + " until it catches.",
        FeedbackType::NONE,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



BallThrower::BallThrower(const BallThrower_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
{}

void BallThrower::program(SingleSwitchProgramEnvironment& env){
    grip_menu_connect_go_home(env.console);
    pbf_press_button(env.console, BUTTON_HOME, 10, HOME_TO_GAME_DELAY);

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
