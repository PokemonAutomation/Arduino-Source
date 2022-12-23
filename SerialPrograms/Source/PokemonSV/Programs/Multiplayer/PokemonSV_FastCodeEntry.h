/*  Fast Code Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_FastCodeEntry_H
#define PokemonAutomation_PokemonSV_FastCodeEntry_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"
#include "Common/Cpp/Options/TextEditOption.h"
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
    TextEditOption CODE;
    EnumDropdownOption<KeyboardLayout> KEYBOARD_LAYOUT;
    SectionDividerOption m_advanced_options;
    BooleanCheckBoxOption SKIP_PLUS;
    BooleanCheckBoxOption DIGIT_REORDERING;
    TimeExpressionOption<uint8_t> SCROLL_DELAY;
    TimeExpressionOption<uint8_t> WRAP_DELAY;
};




}
}
}
#endif
