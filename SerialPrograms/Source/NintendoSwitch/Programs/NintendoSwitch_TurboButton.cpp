/*  Turbo Button
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
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
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
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
        },
        LockWhileRunning::LOCKED,
        BUTTON_A
    )
    , PRESS_DURATION(
        "<b>Press Duration:</b><br>Hold the button down for this long.",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        5, "5"
    )
    , RELEASE_DURATION(
        "<b>Release Duration:</b><br>After releasing the button, wait this long before pressing it again.",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        3, "3"
    )
    , TOTAL_PRESSES(
        "<b>Total Presses:</b><br>Stop the program after this many presses. If zero, run forever.",
        LockWhileRunning::LOCKED,
        0
    )
{
    PA_ADD_OPTION(BUTTON);
    PA_ADD_OPTION(PRESS_DURATION);
    PA_ADD_OPTION(RELEASE_DURATION);
    PA_ADD_OPTION(TOTAL_PRESSES);
}
void TurboButton::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    if (TOTAL_PRESSES == 0){
        while (true){
            pbf_press_button(context, (Button)BUTTON.current_value(), PRESS_DURATION, RELEASE_DURATION);
        }
    }else{
        for (uint64_t c = 0; c < TOTAL_PRESSES; c++){
            pbf_press_button(context, (Button)BUTTON.current_value(), PRESS_DURATION, RELEASE_DURATION);
        }
    }
    context.wait_for_all_requests();
}



}
}

