/*  Shiny Sound Listener
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_ShinySoundListener_H
#define PokemonAutomation_PokemonLA_ShinySoundListener_H

#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/GoHomeWhenDoneOption.h"

namespace PokemonAutomation{ 
namespace NintendoSwitch{
namespace PokemonLA{


class ShinySoundListener_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    ShinySoundListener_Descriptor();
};


class ShinySoundListener : public SingleSwitchProgramInstance{
public:
    ShinySoundListener(const ShinySoundListener_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;

private:
    BooleanCheckBoxOption STOP_ON_SHINY_SOUND;
};




}
}
}
#endif
