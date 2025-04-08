/*  Code Entry Settings Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "NintendoSwitch_CodeEntrySettingsOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



KeyboardLayoutOption::KeyboardLayoutOption()
    : EnumDropdownOption<KeyboardLayout>(
        "<b>Keyboard Layout:</b>",
        {
            {KeyboardLayout::QWERTY, "qwerty", "QWERTY"},
            {KeyboardLayout::AZERTY, "azerty", "AZERTY"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        KeyboardLayout::QWERTY
    )
{}
KeyboardLayoutOption::KeyboardLayoutOption(std::string label)
    : EnumDropdownOption<KeyboardLayout>(
        std::move(label),
        {
            {KeyboardLayout::QWERTY, "qwerty", "QWERTY"},
            {KeyboardLayout::AZERTY, "azerty", "AZERTY"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        KeyboardLayout::QWERTY
    )
{}

CodeEntrySkipPlusOption::CodeEntrySkipPlusOption()
    : BooleanCheckBoxOption(
        "<b>Skip the Plus:</b><br>Don't press + to finalize the code. Useful for testing.",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
{}





DigitEntryTimingsOption::DigitEntryTimingsOption()
    : GroupOption(
        "Digit Entry Timings",
        LockMode::UNLOCK_WHILE_RUNNING,
        GroupOption::EnableMode::ALWAYS_ENABLED, true
    )
    , DIGIT_REORDERING(
        "<b>Digit Reordering:</b><br>Allow digits to be entered out of order.",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , TIME_UNIT(
        "<b>Time Unit:</b><br>Timesteps should increment in multiples of this unit.<br>"
        "<font color=\"red\">Controller timing variation will be added to this number.</font>",
        LockMode::UNLOCK_WHILE_RUNNING,
        PreloadSettings::instance().DEVELOPER_MODE ? "24 ms" : "40ms"
    )
{
    PA_ADD_OPTION(DIGIT_REORDERING);
    PA_ADD_OPTION(TIME_UNIT);
}



KeyboardEntryTimingsOption::KeyboardEntryTimingsOption()
    : GroupOption(
        "Keyboard Entry Timings",
        LockMode::UNLOCK_WHILE_RUNNING,
        GroupOption::EnableMode::ALWAYS_ENABLED, true
    )
    , DIGIT_REORDERING(
        "<b>Digit Reordering:</b><br>Allow digits to be entered out of order.",
        LockMode::UNLOCK_WHILE_RUNNING,
        PreloadSettings::instance().DEVELOPER_MODE
    )
    , TIME_UNIT(
        "<b>Time Unit:</b><br>Timesteps should increment in multiples of this unit.<br>"
        "<font color=\"red\">Controller timing variation will be added to this number.</font>",
        LockMode::UNLOCK_WHILE_RUNNING,
        PreloadSettings::instance().DEVELOPER_MODE ? "24 ms" : "40ms"
    )
{
    PA_ADD_OPTION(DIGIT_REORDERING);
    PA_ADD_OPTION(TIME_UNIT);
}







}
}
