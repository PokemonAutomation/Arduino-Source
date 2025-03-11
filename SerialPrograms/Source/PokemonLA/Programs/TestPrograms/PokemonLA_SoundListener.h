/*  Sound Listener
 *
 *  From: https://github.com/PokemonAutomation/
 *  
 *  Debug program to test all kinds of sound detectors.
 */

#ifndef PokemonAutomation_PokemonLA_SoundListener_H
#define PokemonAutomation_PokemonLA_SoundListener_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{ 
namespace NintendoSwitch{
namespace PokemonLA{


class SoundListener_Descriptor : public SingleSwitchProgramDescriptor{
public:
    SoundListener_Descriptor();
};


class SoundListener : public SingleSwitchProgramInstance{
public:
    SoundListener();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    enum class SoundType{
        Shiny,
        AlphaRoar,
        AlphaMusic,
        ItemDrop,
    };
    EnumDropdownOption<SoundType> SOUND_TYPE;
    BooleanCheckBoxOption STOP_ON_DETECTED_SOUND;
};




}
}
}
#endif
