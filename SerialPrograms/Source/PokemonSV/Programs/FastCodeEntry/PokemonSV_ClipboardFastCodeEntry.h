/*  Fast Code Entry (Clipboard)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_ClipboardFastCodeEntry_H
#define PokemonAutomation_PokemonSV_ClipboardFastCodeEntry_H

#include "NintendoSwitch/NintendoSwitch_MultiSwitchProgram.h"
#include "PokemonSV_CodeEntry.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class ClipboardFastCodeEntry_Descriptor : public MultiSwitchProgramDescriptor{
public:
    ClipboardFastCodeEntry_Descriptor();
};




class ClipboardFastCodeEntry : public MultiSwitchProgramInstance{
public:
    ClipboardFastCodeEntry();
    virtual void update_active_consoles(size_t switch_count) override;
    virtual void program(MultiSwitchProgramEnvironment& env, CancellableScope& scope) override;

private:
    FastCodeEntrySettingsOption SETTINGS;
};




}
}
}
#endif
