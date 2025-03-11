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
    , BUTTON_HOLD(
        "<b>Button Hold Duration:</b><br>Hold button down for this long.<br>"
        "<font color=\"red\">This timing is only relevant if the controller is tick-precise.</font>",
        LockMode::UNLOCK_WHILE_RUNNING,
        "40 ms"
    )
    , BUTTON_COOLDOWN(
        "<b>Button Cooldown Delay:</b><br>After releasing a button, do not use it again for this long.<br>"
        "<font color=\"red\">This timing is only relevant if the controller is tick-precise.</font>",
        LockMode::UNLOCK_WHILE_RUNNING,
        "24 ms"
    )
    , PRESS_DELAY(
        "<b>Press Delay:</b><br>Delay after entering a digit.<br>"
        "<font color=\"red\">This timing is only relevant if the controller is tick-precise.</font>",
        LockMode::UNLOCK_WHILE_RUNNING,
        "32 ms"
    )
    , SCROLL_DELAY(
        "<b>Scroll Delay:</b><br>Delay to scroll between adjacent keys.<br>"
        "<font color=\"red\">This timing is only relevant if the controller is tick-precise.</font>",
        LockMode::UNLOCK_WHILE_RUNNING,
        "32 ms"
    )
{
    PA_ADD_OPTION(DIGIT_REORDERING);

    PA_ADD_OPTION(BUTTON_HOLD);
    PA_ADD_OPTION(BUTTON_COOLDOWN);

    PA_ADD_OPTION(PRESS_DELAY);
    PA_ADD_OPTION(SCROLL_DELAY);
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
    , BUTTON_HOLD(
        "<b>Button Hold Duration:</b><br>Hold button down for this long.<br>"
        "<font color=\"red\">This timing is only relevant if the controller is tick-precise.</font>",
        LockMode::UNLOCK_WHILE_RUNNING,
        "40 ms"
    )
    , BUTTON_COOLDOWN(
        "<b>Button Cooldown Delay:</b><br>After releasing a button, do not use it again for this long.<br>"
        "<font color=\"red\">This timing is only relevant if the controller is tick-precise.</font>",
        LockMode::UNLOCK_WHILE_RUNNING,
        "24 ms"
    )
    , PRESS_DELAY(
        "<b>Press Delay:</b><br>Delay after entering a digit.<br>"
        "<font color=\"red\">This timing is only relevant if the controller is tick-precise.</font>",
        LockMode::UNLOCK_WHILE_RUNNING,
        "32 ms"
    )
    , SCROLL_DELAY(
        "<b>Scroll Delay:</b><br>Delay to scroll between adjacent keys.<br>"
        "<font color=\"red\">This timing is only relevant if the controller is tick-precise.</font>",
        LockMode::UNLOCK_WHILE_RUNNING,
        PreloadSettings::instance().DEVELOPER_MODE ? "40 ms" : "48 ms"
    )
    , WRAP_DELAY(
        "<b>Wrap Delay:</b><br>Delay to wrap between left/right edges.<br>"
        "<font color=\"red\">This timing is only relevant if the controller is tick-precise.</font>",
        LockMode::UNLOCK_WHILE_RUNNING,
        "48 ms"
    )
{
    PA_ADD_OPTION(DIGIT_REORDERING);

    PA_ADD_OPTION(BUTTON_HOLD);
    PA_ADD_OPTION(BUTTON_COOLDOWN);

    PA_ADD_OPTION(PRESS_DELAY);
    PA_ADD_OPTION(SCROLL_DELAY);
    PA_ADD_OPTION(WRAP_DELAY);
}







}
}
