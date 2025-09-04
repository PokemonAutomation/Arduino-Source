/*  Turbo Button
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch_TurboButton.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



TurboButton_Descriptor::TurboButton_Descriptor()
    : SingleSwitchProgramDescriptor(
        "NintendoSwitch:TurboButton",
        "Nintendo Switch", "Turbo Button",
        "ComputerControl/blob/master/Wiki/Programs/NintendoSwitch/TurboButton.md",
        "Mash a controller button. (similar to turbo controller)",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}



TurboButton::TurboButton()
    : BUTTON(
        "<b>Button to Mash:</b>",
        {
            {BUTTON_Y,          "Y", "Y"},
            {BUTTON_B,          "B", "B"},
            {BUTTON_A,          "A", "A"},
            {BUTTON_X,          "X", "X"},
            {BUTTON_L,          "L", "L"},
            {BUTTON_R,          "R", "R"},
            {BUTTON_ZL,         "ZL", "ZL"},
            {BUTTON_ZR,         "ZR", "ZR"},
            {BUTTON_MINUS,      "MINUS", "Minus (-)"},
            {BUTTON_PLUS,       "PLUS", "Plus (+)"},
            {BUTTON_LCLICK,     "LCLICK", "L-Click (left joystick click)"},
            {BUTTON_RCLICK,     "RCLICK", "R-Click (right joystick click)"},
            {BUTTON_HOME,       "HOME", "Home"},
            {BUTTON_CAPTURE,    "CAPTURE", "Capture"},
            {BUTTON_GR,         "GR", "GR (Switch 2)"},
            {BUTTON_GL,         "GL", "GL (Switch 2)"},
            {BUTTON_C,          "C", "C (Switch 2)"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        BUTTON_A
    )
    , PRESS_DURATION0(
        "<b>Press Duration:</b><br>Hold the button down for this long.",
        LockMode::LOCK_WHILE_RUNNING,
        "40 ms"
    )
    , RELEASE_DURATION0(
        "<b>Release Duration:</b><br>After releasing the button, wait this long before pressing it again.",
        LockMode::LOCK_WHILE_RUNNING,
        "24 ms"
    )
    , TOTAL_PRESSES(
        "<b>Total Presses:</b><br>Stop the program after this many presses. If zero, run forever.",
        LockMode::LOCK_WHILE_RUNNING,
        0
    )
{
    PA_ADD_OPTION(BUTTON);
    PA_ADD_OPTION(PRESS_DURATION0);
    PA_ADD_OPTION(RELEASE_DURATION0);
    PA_ADD_OPTION(TOTAL_PRESSES);
}
void TurboButton::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    if (TOTAL_PRESSES == 0){
        while (true){
            pbf_press_button(
                context,
                (Button)BUTTON.current_value(),
                PRESS_DURATION0,
                RELEASE_DURATION0
            );
        }
    }else{
        for (uint64_t c = 0; c < TOTAL_PRESSES; c++){
            pbf_press_button(
                context,
                (Button)BUTTON.current_value(),
                PRESS_DURATION0,
                RELEASE_DURATION0
            );
        }
    }
    context.wait_for_all_requests();
}



}
}

