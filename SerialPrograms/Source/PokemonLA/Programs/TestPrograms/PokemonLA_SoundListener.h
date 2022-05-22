/*  Sound Listener
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *  
 *  Debug program to test all kinds of sound detectors.
 */

#ifndef PokemonAutomation_PokemonLA_SoundListener_H
#define PokemonAutomation_PokemonLA_SoundListener_H

#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/GoHomeWhenDoneOption.h"
#include "CommonFramework/Options/EnumDropdownOption.h"

namespace PokemonAutomation{ 
namespace NintendoSwitch{
namespace PokemonLA{


class SoundListener_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    SoundListener_Descriptor();
};


class SoundListener : public SingleSwitchProgramInstance{
public:
    SoundListener(const SoundListener_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:

    EnumDropdownOption SOUND_TYPE;
    BooleanCheckBoxOption STOP_ON_DETECTED_SOUND;
};




}
}
}
#endif
