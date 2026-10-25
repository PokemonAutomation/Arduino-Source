/*  Typewriter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Typewriter_H
#define PokemonAutomation_NintendoSwitch_Typewriter_H

#include "Common/Cpp/Options/TextEditOption.h"
#include "Controllers/StandardHid/StandardHid_Keyboard.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_CodeEntrySettingsOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace Typewriter{


class Typewriter_Descriptor : public SingleSwitchProgramDescriptor{
public:
    Typewriter_Descriptor();
};


class Typewriter : public SingleSwitchProgramInstance{
public:
    using Descriptor = Typewriter_Descriptor;
    Typewriter();

    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;

private:
    TextEditOption TEXT;
    KeyboardLayoutOption KEYBOARD_LAYOUT;
    MillisecondsOption DELAY;
    MillisecondsOption HOLD;
    MillisecondsOption COOLDOWN;
};



}
}
}
#endif
