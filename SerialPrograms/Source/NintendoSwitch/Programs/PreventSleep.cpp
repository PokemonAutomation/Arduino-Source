/*  Prevent Sleep
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "NintendoSwitch/FixedInterval.h"
#include "PreventSleep.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


PreventSleep_Descriptor::PreventSleep_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "NintendoSwitch:PreventSleep",
        "Prevent Sleep",
        "SwSh-Arduino/wiki/Advanced:-PreventSleep",
        "Press B every 15 seconds to keep the Switch from sleeping.",
        FeedbackType::NONE,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



PreventSleep::PreventSleep(const PreventSleep_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
{}

void PreventSleep::program(SingleSwitchProgramEnvironment& env){
    while (true){
        ssf_press_button2(env.console, BUTTON_B, 15 * TICKS_PER_SECOND, 10);
    }
}


}
}

