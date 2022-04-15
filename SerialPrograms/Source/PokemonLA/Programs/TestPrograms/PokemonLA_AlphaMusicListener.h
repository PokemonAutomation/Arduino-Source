/*  Alpha Music Listener
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_AlphaMusicListener_H
#define PokemonAutomation_PokemonLA_AlphaMusicListener_H

#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/GoHomeWhenDoneOption.h"

namespace PokemonAutomation{ 
namespace NintendoSwitch{
namespace PokemonLA{


class AlphaMusicListener_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    AlphaMusicListener_Descriptor();
};


class AlphaMusicListener : public SingleSwitchProgramInstance{
public:
    AlphaMusicListener(const AlphaMusicListener_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    BooleanCheckBoxOption STOP_ON_ALPHA_MUSIC;
};




}
}
}
#endif
