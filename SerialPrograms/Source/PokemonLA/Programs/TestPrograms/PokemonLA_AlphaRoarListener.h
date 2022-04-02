/*  Alpha Roar Listener
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_AlphaRoarListener_H
#define PokemonAutomation_PokemonLA_AlphaRoarListener_H

#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/GoHomeWhenDoneOption.h"

namespace PokemonAutomation{ 
namespace NintendoSwitch{
namespace PokemonLA{


class AlphaRoarListener_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    AlphaRoarListener_Descriptor();
};


class AlphaRoarListener : public SingleSwitchProgramInstance{
public:
    AlphaRoarListener(const AlphaRoarListener_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env, const BotBaseContext& context) override;

private:
    BooleanCheckBoxOption STOP_ON_ALPHA_ROAR;
};




}
}
}
#endif
