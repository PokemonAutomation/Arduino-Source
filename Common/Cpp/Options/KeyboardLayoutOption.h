/*  Keyboard Layout Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_KeyboardLayoutOption_H
#define PokemonAutomation_Options_KeyboardLayoutOption_H

#include "EnumDropdownOption.h"

namespace PokemonAutomation{


enum class KeyboardLayout{
    QWERTY,
    AZERTY,
};


class KeyboardLayoutOption : public EnumDropdownOption<KeyboardLayout>{
public:
    ~KeyboardLayoutOption();
    KeyboardLayoutOption(
        std::string label,
        LockMode lock_while_running,
        KeyboardLayout default_value
    );


};





}
#endif
