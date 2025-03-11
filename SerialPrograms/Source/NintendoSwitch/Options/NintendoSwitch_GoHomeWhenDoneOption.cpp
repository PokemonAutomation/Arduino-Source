/*  Go Home When Done
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch_GoHomeWhenDoneOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


GoHomeWhenDoneOption::GoHomeWhenDoneOption(bool default_value)
    : BooleanCheckBoxOption(
        "<b>Go Home when Done:</b><br>"
        "When the program finishes, go to the Switch Home menu to idle. (turn this off for unattended streaming)",
        LockMode::UNLOCK_WHILE_RUNNING,
        default_value
    )
{}

void GoHomeWhenDoneOption::run_end_of_program(ProControllerContext& context){
    if (*this){
        pbf_wait(context, 5 * TICKS_PER_SECOND);
        pbf_press_button(context, BUTTON_HOME, 20, 125);
    }
}



}
}
