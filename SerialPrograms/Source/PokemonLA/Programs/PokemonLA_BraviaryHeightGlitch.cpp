/*  Braviary Height Glitch
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLA_BraviaryHeightGlitch.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


BraviaryHeightGlitch_Descriptor::BraviaryHeightGlitch_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonLA:BraviaryHeightGlitch",
        STRING_POKEMON + " LA", "Braviary Height Glitch",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/BraviaryHeightGlitch.md",
        "Increase your height in place using the height glitch.",
        FeedbackType::NONE, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


BraviaryHeightGlitch::BraviaryHeightGlitch(const BraviaryHeightGlitch_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
{}


void BraviaryHeightGlitch::program(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    while (true){
        pbf_press_button(env.console, BUTTON_Y, 30, 0);
        pbf_press_button(env.console, BUTTON_PLUS, 30, 10);
        pbf_press_button(env.console, BUTTON_PLUS, 30, 30);
    }
}




}
}
}
