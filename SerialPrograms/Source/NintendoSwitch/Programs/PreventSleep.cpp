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


PreventSleep::PreventSleep()
    : SingleSwitchProgram(
        FeedbackType::NONE, PABotBaseLevel::PABOTBASE_12KB,
        "Prevent Sleep",
        "SerialPrograms/PreventSleep.md",
        "Press B every 15 seconds to keep the Switch from sleeping."
    )
{}

void PreventSleep::program(SingleSwitchProgramEnvironment& env) const{
    while (true){
        ssf_press_button2(BUTTON_B, 15 * TICKS_PER_SECOND, 10);
    }
}


}
}

