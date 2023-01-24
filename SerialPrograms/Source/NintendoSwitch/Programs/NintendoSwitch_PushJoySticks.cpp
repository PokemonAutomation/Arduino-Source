/*  Turbo Button
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch_PushJoySticks.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



PushJoySticks_Descriptor::PushJoySticks_Descriptor()
    : SingleSwitchProgramDescriptor(
        "NintendoSwitch:PushJoySticks",
        "Nintendo Switch", "Push Joy Sticks",
        "ComputerControl/blob/master/Wiki/Programs/NintendoSwitch/PushJoySticks.md",
        "Push Joy Sticks continously.",
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


//  x = 0 : left
//  x = 128 : neutral
//  x = 255 : right
//  y = 0 : up
//  y = 128 : neutral
//  y = 255 : down

PushJoySticks::PushJoySticks()
    : LEFT_X(
        "<b>Left Joy Stick X direction:</b><br>Range: 0-255. 0: left, 128: neutral, 255: right.",
        LockWhileRunning::LOCKED,
        128, 0, 255
    )
    , LEFT_Y(
        "<b>Left Joy Stick Y direction:</b><br>Range: 0-255. 0: up, 128: neutral, 255: down.",
        LockWhileRunning::LOCKED,
        128, 0, 255
    )
    , RIGHT_X(
        "<b>Right Joy Stick X direction:</b><br>Range: 0-255. 0: left, 128: neutral, 255: right.",
        LockWhileRunning::LOCKED,
        128, 0, 255
    )
    , RIGHT_Y(
        "<b>Right Joy Stick Y direction:</b><br>Range: 0-255. 0: up, 128: neutral, 255: down.",
        LockWhileRunning::LOCKED,
        128, 0, 255
    )
{
    PA_ADD_OPTION(LEFT_X);
    PA_ADD_OPTION(LEFT_Y);
    PA_ADD_OPTION(RIGHT_X);
    PA_ADD_OPTION(RIGHT_Y);
}

void PushJoySticks::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    while(true){
        pbf_controller_state(context, BUTTON_LCLICK, DPAD_NONE,
            LEFT_X, LEFT_Y, RIGHT_X, RIGHT_Y, TICKS_PER_SECOND);
    }
}



}
}

