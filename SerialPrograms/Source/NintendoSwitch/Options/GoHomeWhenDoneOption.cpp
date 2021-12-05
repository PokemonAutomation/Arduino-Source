/*  Go Home When Done
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_PushButtons.h"
#include "GoHomeWhenDoneOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


GoHomeWhenDoneOption::GoHomeWhenDoneOption(bool default_value)
    : BooleanCheckBoxOption(
        "<b>Go Home when Done:</b><br>"
        "After finding a shiny, go to the Switch Home menu to idle. (turn this off for unattended streaming)",
        default_value
    )
{}

void GoHomeWhenDoneOption::run_end_of_program(const BotBaseContext& context){
    if (*this){
        pbf_wait(context, 5 * TICKS_PER_SECOND);
        pbf_press_button(context, BUTTON_HOME, 10, 125);
    }
}


}
}
