/*  Clothing Buyer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh_ClothingBuyer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


ClothingBuyer_Descriptor::ClothingBuyer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:ClothingBuyer",
        STRING_POKEMON + " SwSh", "Clothing Buyer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/ClothingBuyer.md",
        "Buy out all the clothing in a store.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}



ClothingBuyer::ClothingBuyer()
    : CATEGORY_ROTATION(
        "<b>Rotate Categories:</b><br>This slows down the program, but ensures it will cover all categories.",
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(CATEGORY_ROTATION);
}

void ClothingBuyer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_no_interact(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
    }else{
        pbf_press_button(context, BUTTON_LCLICK, 5, 5);
    }

    while (true){
        pbf_press_button(context, BUTTON_A, 10, 90);
        if (CATEGORY_ROTATION){
            pbf_press_dpad(context, DPAD_RIGHT, 10, 40);
        }
        pbf_press_button(context, BUTTON_A, 10, 90);
        pbf_press_button(context, BUTTON_A, 10, 90);
        pbf_press_dpad(context, DPAD_DOWN, 10, 40);
    }
}


}
}
}
