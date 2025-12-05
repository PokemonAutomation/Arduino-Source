/*  Turbo Macro
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "NintendoSwitch_TurboMacro.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


TurboMacro_Descriptor::TurboMacro_Descriptor()
    : SingleSwitchProgramDescriptor(
        "NintendoSwitch:TurboMacro",
        "Nintendo Switch", "Turbo Macro",
        "Programs/NintendoSwitch/TurboMacro.html",
        "Create macros",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

TurboMacro::TurboMacro()
    : LOOP(
        "<b>Number of times to loop:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        100, 0
    )
    , TABLE(
        "Command Schedule:",
        {
            ControllerClass::NintendoSwitch_ProController,
            ControllerClass::NintendoSwitch_LeftJoycon,
            ControllerClass::NintendoSwitch_RightJoycon,
        }
    )
{
    PA_ADD_OPTION(LOOP);
    PA_ADD_OPTION(TABLE);
}


void TurboMacro::program(SingleSwitchProgramEnvironment& env, CancellableScope& scope){

    //  Connect the controller.
    //pbf_press_button(context, BUTTON_LCLICK, 5, 5);

    for (uint32_t c = 0; c < LOOP; c++){
        TABLE.run(scope, env.console.controller());
    }
}


}
}
