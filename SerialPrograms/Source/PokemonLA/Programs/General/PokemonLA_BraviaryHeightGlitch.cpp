/*  Braviary Height Glitch
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA_BraviaryHeightGlitch.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{
    using namespace Pokemon;


BraviaryHeightGlitch_Descriptor::BraviaryHeightGlitch_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLA:BraviaryHeightGlitch",
        STRING_POKEMON + " LA", "Braviary Height Glitch",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/BraviaryHeightGlitch.md",
        "Increase your height in place using the height glitch.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}


BraviaryHeightGlitch::BraviaryHeightGlitch(){}


void BraviaryHeightGlitch::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    while (true){
        pbf_press_button(context, BUTTON_Y, 30, 0);
        pbf_press_button(context, BUTTON_PLUS, 30, 10);
        pbf_press_button(context, BUTTON_PLUS, 30, 30);
    }
}




}
}
}
