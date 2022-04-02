/*  Fast Code Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_DigitEntry.h"
#include "PokemonSwSh_FastCodeEntry.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


FastCodeEntry_Descriptor::FastCodeEntry_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:FastCodeEntry",
        STRING_POKEMON + " SwSh", "Fast Code Entry (FCE)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/FastCodeEntry.md",
        "Force your way into raids by entering 8-digit codes in under 1 second.",
        FeedbackType::NONE, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



FastCodeEntry::FastCodeEntry(const FastCodeEntry_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
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
    PA_ADD_OPTION(RAID_CODE);
    PA_ADD_OPTION(INITIAL_DELAY);
}

void FastCodeEntry::program(SingleSwitchProgramEnvironment& env, const BotBaseContext& context){
    uint8_t code[8];
    RAID_CODE.to_str(code);

    if (INITIAL_DELAY != 0){
        start_program_flash(env.console, INITIAL_DELAY);
    }

    pbf_press_button(env.console, BUTTON_PLUS, 5, 5);
    pbf_press_button(env.console, BUTTON_PLUS, 5, 5);
    enter_digits(env.console, 8, code);
}



}
}
}
