/*  Code Entry Settings Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "NintendoSwitch_CodeEntrySettingsOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

using namespace std::chrono_literals;


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



CodeboardTimingsOption::CodeboardTimingsOption(
    std::string label,
    bool switch2,
    ControllerPerformanceClass performance_class
)
    : GroupOption(
        std::move(label),
        LockMode::UNLOCK_WHILE_RUNNING,
        GroupOption::EnableMode::ALWAYS_ENABLED, true
    )
    , unit(get_unit_timing(performance_class))
    , REORDERING(
        "<b>Digit Reordering:</b><br>Allow digits to be entered out of order.",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , HOLD(
        "<b>Hold:</b><br>Duration to hold each button press down.",
        LockMode::UNLOCK_WHILE_RUNNING,
        PreloadSettings::instance().DEVELOPER_MODE && performance_class == ControllerPerformanceClass::SerialPABotBase_Wired
            ? "40 ms"
            : std::to_string(unit.count() * 2) + " ms"
    )
    , COOLDOWN(
        "<b>Cooldown:</b><br>Do not reuse a button until this long after it is reused.",
        LockMode::UNLOCK_WHILE_RUNNING,
        std::to_string(unit.count() * 1) + " ms"
    )
    , PRESS_DELAY(
        "<b>Press Delay:</b><br>Wait this long after entering a character.",
        LockMode::UNLOCK_WHILE_RUNNING,
        switch2
            ? std::to_string(unit.count() * 2) + " ms"
            : std::to_string(unit.count() * 1) + " ms"
    )
    , MOVE_DELAY(
        "<b>Move Delay:</b><br>Wait this long after moving the cursor.",
        LockMode::UNLOCK_WHILE_RUNNING,
        switch2
            ? std::to_string(unit.count() * 2) + " ms"
            : std::to_string(unit.count() * 1) + " ms"
    )
    , SCROLL_DELAY(
        "<b>Scroll Delay:</b><br>Wait this long after scrolling.",
        LockMode::UNLOCK_WHILE_RUNNING,
        switch2
            ? std::to_string(unit.count() * 2) + " ms"
            : std::to_string(unit.count() * 1) + " ms"
    )
    , WRAP_DELAY(
        "<b>Wrap Delay:</b><br>Wait this long after a wrapping scroll.",
        LockMode::UNLOCK_WHILE_RUNNING,
        std::to_string(unit.count() * 2) + " ms"
    )
{
    PA_ADD_OPTION(REORDERING);
    PA_ADD_OPTION(HOLD);
    PA_ADD_OPTION(COOLDOWN);
    PA_ADD_OPTION(PRESS_DELAY);
    PA_ADD_OPTION(MOVE_DELAY);
    PA_ADD_OPTION(SCROLL_DELAY);
    PA_ADD_OPTION(WRAP_DELAY);
}

Milliseconds CodeboardTimingsOption::get_unit_timing(ControllerPerformanceClass performance_class){
    switch (performance_class){
    case ControllerPerformanceClass::SerialPABotBase_Wired:
        return PreloadSettings::instance().DEVELOPER_MODE ? 24ms : 40ms;
    case ControllerPerformanceClass::SerialPABotBase_Wireless:
        return PreloadSettings::instance().DEVELOPER_MODE ? 34ms : 40ms;
    case ControllerPerformanceClass::SysbotBase:
        return 100ms;
    default:
        throw InternalProgramError(
            nullptr,
            PA_CURRENT_FUNCTION,
            "Invalid performance class: " + std::to_string((int)performance_class)
        );
    }
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
