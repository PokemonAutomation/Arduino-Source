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
    : RunnableSwitchProgramDescriptor(
        "NintendoSwitch:TurboButton",
        "Nintendo Switch", "Turbo Button",
        "ComputerControl/blob/master/Wiki/Programs/NintendoSwitch/TurboButton.md",
        "Mash a controller button. (similar to turbo controller)",
        FeedbackType::NONE, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



TurboButton::TurboButton(const TurboButton_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , BUTTON(
        "<b>Button to Mash:</b>",
        {
            "Y",
            "B",
            "A",
            "X",
            "L",
            "R",
            "ZL",
            "ZR",
            "Minus (-)",
            "Plus (+)",
            "L-Click (left joystick click)",
            "R-Click (right joystick click)",
            "Home",
            "Capture",
        },
        2
    )
    , PRESS_DURATION("<b>Press Duration:</b><br>Hold the button down for this long.", "5", 5)
    , RELEASE_DURATION("<b>Release Duration:</b><br>After releasing the button, wait this long before pressing it again.", "3", 3)
    , TOTAL_PRESSES("<b>Total Presses:</b><br>Stop the program after this many presses. If zero, run forever.", 0, 0)
{
    PA_ADD_OPTION(BUTTON);
    PA_ADD_OPTION(PRESS_DURATION);
    PA_ADD_OPTION(RELEASE_DURATION);
    PA_ADD_OPTION(TOTAL_PRESSES);
}
void TurboButton::program(SingleSwitchProgramEnvironment& env){
    if (TOTAL_PRESSES == 0){
        while (true){
            pbf_press_button(env.console, (Button)1 << BUTTON, PRESS_DURATION, RELEASE_DURATION);
        }
    }else{
        for (uint64_t c = 0; c < TOTAL_PRESSES; c++){
            pbf_press_button(env.console, (Button)1 << BUTTON, PRESS_DURATION, RELEASE_DURATION);
        }
    }
    env.console.botbase().wait_for_all_requests();
}



}
}

