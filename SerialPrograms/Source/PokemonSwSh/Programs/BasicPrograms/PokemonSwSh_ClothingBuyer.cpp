/*  Clothing Buyer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "PokemonSwSh_ClothingBuyer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

ClothingBuyer::ClothingBuyer()
    : SingleSwitchProgram(
        FeedbackType::NONE, PABotBaseLevel::PABOTBASE_12KB,
        "Clothing Buyer",
        "NativePrograms/ClothingBuyer.md",
        "Buy out all the clothing in a store."
    )
    , CATEGORY_ROTATION(
        "<b>Rotate Categories:</b><br>This slows down the program, but ensures it will cover all categories.",
        true
    )
{
    m_options.emplace_back(&CATEGORY_ROTATION, "CATEGORY_ROTATION");
}

void ClothingBuyer::program(SingleSwitchProgramEnvironment& env) const{
    grip_menu_connect_go_home();
    resume_game_no_interact(TOLERATE_SYSTEM_UPDATE_MENU_FAST);

    while (true){
        pbf_press_button(BUTTON_A, 10, 90);
        if (CATEGORY_ROTATION){
            pbf_press_dpad(DPAD_RIGHT, 10, 40);
        }
        pbf_press_button(BUTTON_A, 10, 90);
        pbf_press_button(BUTTON_A, 10, 90);
        pbf_press_dpad(DPAD_DOWN, 10, 40);
    }
}


}
}
}
