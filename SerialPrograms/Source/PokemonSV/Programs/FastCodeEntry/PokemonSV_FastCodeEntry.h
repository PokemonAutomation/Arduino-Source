/*  Fast Code Entry
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_FastCodeEntry_H
#define PokemonAutomation_PokemonSV_FastCodeEntry_H

#include "Common/Cpp/Options/TextEditOption.h"
#include "NintendoSwitch/NintendoSwitch_MultiSwitchProgram.h"
#include "PokemonSV_CodeEntry.h"

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
    virtual void update_active_consoles(size_t switch_count) override;
    virtual void program(MultiSwitchProgramEnvironment& env, CancellableScope& scope) override;

private:
    enum class Mode{
        NORMAL,
        ENTER_ON_PASTE,
        MYSTERY_GIFT,
    };
    EnumDropdownOption<Mode> MODE;

    TextEditOption CODE;
    FastCodeEntrySettingsOption SETTINGS;

};




}
}
}
#endif
