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





DigitEntryTimingsOption::DigitEntryTimingsOption(bool switch2)
    : GroupOption(
        switch2
            ? "Switch 2 Digit Entry Timings"
            : "Switch 1 Digit Entry Timings",
        LockMode::UNLOCK_WHILE_RUNNING,
        GroupOption::EnableMode::ALWAYS_ENABLED, true
    )
    , REORDERING(
        "<b>Digit Reordering:</b><br>Allow digits to be entered out of order.",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , TIME_UNIT(
        "<b>Time Unit:</b><br>Timesteps should increment in multiples of this unit.<br>"
        "<font color=\"red\">Controller timing variation will be added to this number.</font>",
        LockMode::UNLOCK_WHILE_RUNNING,
        switch2
            ? PreloadSettings::instance().DEVELOPER_MODE ? "48 ms" : "64 ms"
            : PreloadSettings::instance().DEVELOPER_MODE ? "24 ms" : "40 ms"
    )
    , HOLD(
        "<b>Hold:</b><br>Duration to hold each button press down.<br>"
        "<font color=\"red\">Controller timing variation will be added to this number.</font>",
        LockMode::UNLOCK_WHILE_RUNNING,
        "48 ms"
    )
    , COOLDOWN(
        "<b>Cooldown:</b><br>Do not reuse a button until this long after it is reused.<br>"
        "<font color=\"red\">Controller timing variation will be added to this number.</font>",
        LockMode::UNLOCK_WHILE_RUNNING,
        "24 ms"
    )
{
    PA_ADD_OPTION(REORDERING);
    PA_ADD_OPTION(TIME_UNIT);
    PA_ADD_OPTION(HOLD);
    PA_ADD_OPTION(COOLDOWN);
}



KeyboardEntryTimingsOption::KeyboardEntryTimingsOption(bool switch2)
    : GroupOption(
        switch2
            ? "Switch 2 Keyboard Entry Timings"
            : "Switch 1 Keyboard Entry Timings",
        LockMode::UNLOCK_WHILE_RUNNING,
        GroupOption::EnableMode::ALWAYS_ENABLED, true
    )
    , REORDERING(
        "<b>Character Reordering:</b><br>Allow characters to be entered out of order.",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , TIME_UNIT(
        "<b>Time Unit:</b><br>Timesteps should increment in multiples of this unit.<br>"
        "<font color=\"red\">Controller timing variation will be added to this number.</font>",
        LockMode::UNLOCK_WHILE_RUNNING,
        switch2
            ? PreloadSettings::instance().DEVELOPER_MODE ? "48 ms" : "64 ms"
            : PreloadSettings::instance().DEVELOPER_MODE ? "24 ms" : "40 ms"
    )
    , HOLD(
        "<b>Hold:</b><br>Duration to hold each button press down.<br>"
        "<font color=\"red\">Controller timing variation will be added to this number.</font>",
        LockMode::UNLOCK_WHILE_RUNNING,
        "48 ms"
    )
    , COOLDOWN(
        "<b>Cooldown:</b><br>Do not reuse a button until this long after it is reused.<br>"
        "<font color=\"red\">Controller timing variation will be added to this number.</font>",
        LockMode::UNLOCK_WHILE_RUNNING,
        "24 ms"
    )
{
    PA_ADD_OPTION(REORDERING);
    PA_ADD_OPTION(TIME_UNIT);
    PA_ADD_OPTION(HOLD);
    PA_ADD_OPTION(COOLDOWN);
}



KeyboardControllerTimingsOption::KeyboardControllerTimingsOption()
    : GroupOption(
        "Keyboard Controller Timings",
        LockMode::UNLOCK_WHILE_RUNNING,
        GroupOption::EnableMode::ALWAYS_ENABLED, true
    )
    , PARLLELIZE(
        "<b>Parallel Entry:</b><br>Allow characters to be entered in parallel if possible.",
        LockMode::UNLOCK_WHILE_RUNNING,
        PreloadSettings::instance().DEVELOPER_MODE
    )
    , TIME_UNIT(
        "<b>Time Unit:</b><br>Timesteps should increment in multiples of this unit.",
        LockMode::UNLOCK_WHILE_RUNNING,
        PreloadSettings::instance().DEVELOPER_MODE ? "24 ms" : "40 ms"
    )
    , HOLD(
        "<b>Hold:</b><br>Duration to hold each key press down.",
        LockMode::UNLOCK_WHILE_RUNNING,
        "40 ms"
    )
    , COOLDOWN(
        "<b>Cooldown:</b><br>Do not reuse a key until this long after it is reused.",
        LockMode::UNLOCK_WHILE_RUNNING,
        "24 ms"
    )
{
    PA_ADD_OPTION(PARLLELIZE);
    PA_ADD_OPTION(TIME_UNIT);
    PA_ADD_OPTION(HOLD);
    PA_ADD_OPTION(COOLDOWN);
}







}
}
