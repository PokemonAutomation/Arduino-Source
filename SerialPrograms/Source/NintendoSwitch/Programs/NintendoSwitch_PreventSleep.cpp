/*  Prevent Sleep
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
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
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {SerialPABotBase::OLD_NINTENDO_SWITCH_DEFAULT_REQUIREMENTS}
    )
{}



PreventSleep::PreventSleep(){}

void PreventSleep::program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context){
    while (true){
        ssf_press_button2(context, BUTTON_B, 15 * TICKS_PER_SECOND, 10);
    }
}


}
}

