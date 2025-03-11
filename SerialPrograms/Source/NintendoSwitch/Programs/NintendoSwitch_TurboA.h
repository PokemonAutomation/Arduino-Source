/*  Turbo A
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_TurboA_H
#define PokemonAutomation_NintendoSwitch_TurboA_H

#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class TurboA_Descriptor : public SingleSwitchProgramDescriptor{
public:
    TurboA_Descriptor();
};



class TurboA : public SingleSwitchProgramInstance{
public:
    TurboA();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
};




}
}
#endif



