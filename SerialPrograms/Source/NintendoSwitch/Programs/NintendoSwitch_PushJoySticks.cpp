/*  Turbo Button
 *
 *  From: https://github.com/PokemonAutomation/
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
        "Programs/NintendoSwitch/PushJoySticks.html",
        "Push Joy Sticks continuously.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}


//  x = 0 : left
//  x = 128 : neutral
//  x = 255 : right
//  y = 0 : up
//  y = 128 : neutral
//  y = 255 : down

PushJoySticks::PushJoySticks()
    : JOYCON_CLICK(
        "<b>Joycon Click:</b>",
        {
            {0, "no-click", "No Click"},
            {1, "left-click", "Click Left Joycon"},
            {2, "right-click", "Click Right Joycon"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        0
    )
    , LEFT_X(
        "<b>Left Joy Stick X direction:</b><br>Range: 0-255. 0: left, 128: neutral, 255: right.",
        LockMode::LOCK_WHILE_RUNNING,
        128, 0, 255
    )
    , LEFT_Y(
        "<b>Left Joy Stick Y direction:</b><br>Range: 0-255. 0: up, 128: neutral, 255: down.",
        LockMode::LOCK_WHILE_RUNNING,
        128, 0, 255
    )
    , RIGHT_X(
        "<b>Right Joy Stick X direction:</b><br>Range: 0-255. 0: left, 128: neutral, 255: right.",
        LockMode::LOCK_WHILE_RUNNING,
        128, 0, 255
    )
    , RIGHT_Y(
        "<b>Right Joy Stick Y direction:</b><br>Range: 0-255. 0: up, 128: neutral, 255: down.",
        LockMode::LOCK_WHILE_RUNNING,
        128, 0, 255
    )
    , PRESS_DURATION(
        "<b>Press Duration:</b><br>Hold the joysticks down for this long.",
        LockMode::LOCK_WHILE_RUNNING,
        "10000 ms"
    )
    , RELEASE_DURATION(
        "<b>Release Duration:</b><br>After releasing the joysticks, wait this long before pressing them again.",
        LockMode::LOCK_WHILE_RUNNING,
        "0 ms"
    )
{
    PA_ADD_OPTION(JOYCON_CLICK);
    PA_ADD_OPTION(LEFT_X);
    PA_ADD_OPTION(LEFT_Y);
    PA_ADD_OPTION(RIGHT_X);
    PA_ADD_OPTION(RIGHT_Y);
    PA_ADD_OPTION(PRESS_DURATION);
    PA_ADD_OPTION(RELEASE_DURATION);
}

void PushJoySticks::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // Determine which button to click based on the option
    Button button;
    switch (JOYCON_CLICK.current_value()){
        case 0:
            button = BUTTON_NONE;
            break;
        case 1:
            button = BUTTON_LCLICK;
            break;
        case 2:
            button = BUTTON_RCLICK;
            break;
        default:
            button = BUTTON_NONE;
            break;
    }

    while(true){
        pbf_controller_state(context, button, DPAD_NONE,
            LEFT_X, LEFT_Y, RIGHT_X, RIGHT_Y, PRESS_DURATION);
        if (RELEASE_DURATION.get() > std::chrono::milliseconds(0)){
            pbf_wait(context, RELEASE_DURATION);
        }
    }
}



}
}

