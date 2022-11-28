/*  Fast Code Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_FastCodeEntry_H
#define PokemonAutomation_PokemonSV_FastCodeEntry_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "NintendoSwitch/NintendoSwitch_MultiSwitchProgram.h"
#include "NintendoSwitch/Programs/NintendoSwitch_FastCodeEntry.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class FastCodeEntry_Descriptor : public MultiSwitchProgramDescriptor{
public:
    FastCodeEntry_Descriptor();
};




class FastCodeEntry : public MultiSwitchProgramInstance{
public:
    FastCodeEntry();
    virtual void program(MultiSwitchProgramEnvironment& env, CancellableScope& scope) override;

private:
    StringOption CODE;
    EnumDropdownOption<KeyboardLayout> KEYBOARD_LAYOUT;
    BooleanCheckBoxOption FAST_MODE;
};




}
}
}
#endif
