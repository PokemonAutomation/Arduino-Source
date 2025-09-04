/*  Joycon Test Program
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Joycon.h"
#include "JoyconProgram.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


JoyconProgram_Descriptor::JoyconProgram_Descriptor()
    : SingleSwitchProgramDescriptor(
        "NintendoSwitch:JoyconProgram",
        "Nintendo Switch", "Joycon Program",
        "",
        "Template for a joycon program.",
        ProgramControllerClass::SpecializedController,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}


JoyconProgram::JoyconProgram(){}
void JoyconProgram::program(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    JoyconContext context(scope, env.console.controller<JoyconController>());

    //
    //  Sample joycon program.
    //
    //  This will start from the grip menu and enter the system settings.
    //  Note that the joycon is horizontal.
    //
    //  You can press any button, but if it doesn't exist, it won't do anything.
    //
    //  No support for gyro yet. That's coming later.
    //

    pbf_move_joystick(context, 64, 64, 10000ms, 0ms);

#if 0
    pbf_press_button(context, BUTTON_A, 200ms, 2000ms);
    pbf_press_button(context, BUTTON_HOME, 200ms, 2000ms);
    pbf_move_joystick(context, 128, 0, 100ms, 100ms);
    pbf_move_joystick(context, 128, 0, 100ms, 100ms);
    pbf_move_joystick(context, 255, 128, 100ms, 100ms);
    pbf_move_joystick(context, 128, 0, 100ms, 100ms);
    pbf_press_button(context, BUTTON_X, 200ms, 2000ms);
#endif

}






}
}
