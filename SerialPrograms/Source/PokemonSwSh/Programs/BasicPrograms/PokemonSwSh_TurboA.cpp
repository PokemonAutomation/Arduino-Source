/*  Turbo A
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "PokemonSwSh_TurboA.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


TurboA_Descriptor::TurboA_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:TurboA",
        "Turbo A",
        "NativePrograms/TurboA.md",
        "Endlessly mash A.",
        FeedbackType::NONE,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



TurboA::TurboA(const TurboA_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
{}

void TurboA::program(SingleSwitchProgramEnvironment& env){
    grip_menu_connect_go_home(env.console);
    resume_game_back_out(env.console, TOLERATE_SYSTEM_UPDATE_MENU_FAST, 200);
    while (true){
        pbf_press_button(env.console, BUTTON_A, 5, 5);
    }
}


}
}
}

