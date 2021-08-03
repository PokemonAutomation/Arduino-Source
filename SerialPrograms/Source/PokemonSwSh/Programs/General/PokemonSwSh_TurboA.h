/*  Turbo A
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_TurboA_H
#define PokemonAutomation_PokemonSwSh_TurboA_H

#include "NintendoSwitch/Options/StartInGripMenu.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class TurboA_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    TurboA_Descriptor();
};



class TurboA : public SingleSwitchProgramInstance{
public:
    TurboA(const TurboA_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    StartInGripOrGame START_IN_GRIP_MENU;
};


}
}
}
#endif



