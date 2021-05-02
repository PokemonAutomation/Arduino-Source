/*  Fast Code Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/SwitchRoutines/SwitchDigitEntry.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "PokemonSwSh_FastCodeEntry.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

FastCodeEntry::FastCodeEntry()
    : SingleSwitchProgram(
        FeedbackType::NONE, PABotBaseLevel::PABOTBASE_12KB,
        "Fast Code Entry (FCE)",
        "NativePrograms/FastCodeEntry.md",
        "Force your way into raids by entering 8-digit codes in under 1 second."
    )
    , RAID_CODE(
        "<b>Raid Code:</b>",
        8,
        "9107 3091"
    )
    , INITIAL_DELAY(
        "<b>Initial Delay:</b><br>Wait this long before entering the code.",
        "0 * TICKS_PER_SECOND"
    )
{
    m_options.emplace_back(&RAID_CODE, "RAID_CODE");
    m_options.emplace_back(&INITIAL_DELAY, "INITIAL_DELAY");
}

void FastCodeEntry::program(SingleSwitchProgramEnvironment& env) const{
    uint8_t code[8];
    RAID_CODE.to_str(code);

    if (INITIAL_DELAY != 0){
        start_program_flash(env.console, INITIAL_DELAY);
    }

    pbf_press_button(env.console, BUTTON_PLUS, 5, 5);
    pbf_press_button(env.console, BUTTON_PLUS, 5, 5);
    enter_digits(env.console, 8, code);

    end_program_callback(env.console);
}


}
}
}
