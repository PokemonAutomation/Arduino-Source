/*  Ball Thrower
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh_BallThrower.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


BallThrower_Descriptor::BallThrower_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:BallThrower",
        STRING_POKEMON + " SwSh", "Ball Thrower",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/BallThrower.md",
        "Blindly throw balls at the opposing " + STRING_POKEMON + " until it catches.",
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



BallThrower::BallThrower(){
    PA_ADD_OPTION(START_LOCATION);
}

void BallThrower::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().HOME_TO_GAME_DELAY);
    }else{
        pbf_press_button(context, BUTTON_X, 5, 5);
    }

    while (true){
        pbf_press_button(context, BUTTON_X, 50, 50);
        pbf_press_button(context, BUTTON_A, 50, 50);
        pbf_mash_button(context, BUTTON_B, 100);
    }

    pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE);
}



}
}
}
