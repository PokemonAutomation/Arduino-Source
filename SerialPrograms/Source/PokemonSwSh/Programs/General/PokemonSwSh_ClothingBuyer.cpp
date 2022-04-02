/*  Clothing Buyer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh_ClothingBuyer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ClothingBuyer_Descriptor::ClothingBuyer_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:ClothingBuyer",
        STRING_POKEMON + " SwSh", "Clothing Buyer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/ClothingBuyer.md",
        "Buy out all the clothing in a store.",
        FeedbackType::NONE, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



ClothingBuyer::ClothingBuyer(const ClothingBuyer_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , CATEGORY_ROTATION(
        "<b>Rotate Categories:</b><br>This slows down the program, but ensures it will cover all categories.",
        true
    )
{
    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(CATEGORY_ROTATION);
}

void ClothingBuyer::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(context);
        resume_game_no_interact(context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
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
