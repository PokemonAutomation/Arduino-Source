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


const char* enter_code(
    MultiSwitchProgramEnvironment& env, CancellableScope& scope,
    const FastCodeEntrySettings& settings, const std::string& code,
    bool connect_controller_press
);


class FastCodeEntry_Descriptor : public MultiSwitchProgramDescriptor{
public:
    FastCodeEntry_Descriptor();
};




class FastCodeEntry : public MultiSwitchProgramInstance{
public:
    FastCodeEntry();
    virtual void program(MultiSwitchProgramEnvironment& env, CancellableScope& scope) override;

private:
    enum class Mode{
        NORMAL,
        ENTER_ON_PASTE,
    };
    EnumDropdownOption<Mode> MODE;

    TextEditOption CODE;

    FastCodeEntrySettingsOption SETTINGS;
};




}
}
}
#endif
