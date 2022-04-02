/*  Turbo A
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_TurboA_H
#define PokemonAutomation_NintendoSwitch_TurboA_H

#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class TurboA_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    TurboA_Descriptor();
};



class TurboA : public SingleSwitchProgramInstance{
public:
    TurboA(const TurboA_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;

private:
    StartInGripOrGameOption START_IN_GRIP_MENU;
};


}
}
#endif



