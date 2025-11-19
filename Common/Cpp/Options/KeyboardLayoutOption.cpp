/*  Keyboard Layout Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "KeyboardLayoutOption.h"

namespace PokemonAutomation{




KeyboardLayoutOption::~KeyboardLayoutOption() = default;

KeyboardLayoutOption::KeyboardLayoutOption(
    std::string label,
    LockMode lock_while_running,
    KeyboardLayout default_value
)
    : EnumDropdownOption<KeyboardLayout>(
        std::move(label),
        {
            {KeyboardLayout::QWERTY, "qwerty", "QWERTY"},
            {KeyboardLayout::AZERTY, "azerty", "AZERTY"},
        },
        lock_while_running,
        default_value
    )
{}





}


