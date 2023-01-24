/*  Prevent Sleep
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/NintendoSwitch/NintendoSwitch_Protocol_PushButtons.h"
#include "NintendoSwitch/FixedInterval.h"
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
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



PreventSleep::PreventSleep(){}

void PreventSleep::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    while (true){
        ssf_press_button2(context, BUTTON_B, 15 * TICKS_PER_SECOND, 10);
    }
}


}
}

