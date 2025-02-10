/*  Fast Code Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_DigitEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh_FastCodeEntry.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


FastCodeEntry_Descriptor::FastCodeEntry_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:FastCodeEntry",
        STRING_POKEMON + " SwSh", "Fast Code Entry (FCE)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/FastCodeEntry.md",
        "Force your way into raids by entering 8-digit codes in under 1 second.",
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {SerialPABotBase::OLD_NINTENDO_SWITCH_DEFAULT_REQUIREMENTS}
    )
{}



FastCodeEntry::FastCodeEntry()
    : RAID_CODE(
        "<b>Raid Code:</b>",
        8,
        "9107 3091"
    )
    , INITIAL_DELAY0(
        "<b>Initial Delay:</b><br>Wait this long before entering the code.",
        LockMode::LOCK_WHILE_RUNNING,
        "0 ms"
    )
{
    PA_ADD_OPTION(RAID_CODE);
    PA_ADD_OPTION(INITIAL_DELAY0);
}

void FastCodeEntry::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    uint8_t code[8];
    RAID_CODE.to_str(code);

    pbf_wait(context, INITIAL_DELAY0);

    pbf_press_button(context, BUTTON_PLUS, 5, 5);
    pbf_press_button(context, BUTTON_PLUS, 5, 5);
    enter_digits(context, 8, code);
}



}
}
}
