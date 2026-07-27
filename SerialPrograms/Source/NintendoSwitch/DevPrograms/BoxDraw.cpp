/*  Box Draw
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "BoxDraw.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{




BoxDraw_Descriptor::BoxDraw_Descriptor()
    : SingleSwitchProgramDescriptor(
        "NintendoSwitch:BoxDraw",
        "Nintendo Switch", "Box Draw",
        "",
        "Test box coordinates for development.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::ENABLE_COMMANDS
    )
{}

BoxDraw::BoxDraw()
    : BOX_DRAW(LockMode::UNLOCK_WHILE_RUNNING)
{
    PA_ADD_OPTION(BOX_DRAW);
}




void BoxDraw::program(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    auto drawn_box = BOX_DRAW.make_session(env.console.overlay());
    scope.wait_until_cancel();
}




}
}
