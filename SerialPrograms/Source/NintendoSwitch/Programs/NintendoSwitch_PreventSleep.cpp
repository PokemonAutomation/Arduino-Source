/*  Prevent Sleep
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch_PreventSleep.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


PreventSleep_Descriptor::PreventSleep_Descriptor()
    : SingleSwitchProgramDescriptor(
        "NintendoSwitch:PreventSleep",
        "Nintendo Switch", "Prevent Sleep",
        "ComputerControl/blob/master/Wiki/Programs/NintendoSwitch/PreventSleep.md",
        "Press B every 15 seconds to keep the Switch from sleeping.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}



PreventSleep::PreventSleep(){}

void PreventSleep::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    while (true){
        ssf_press_button(context, BUTTON_B, 15000ms, 80ms);
    }
}


}
}

