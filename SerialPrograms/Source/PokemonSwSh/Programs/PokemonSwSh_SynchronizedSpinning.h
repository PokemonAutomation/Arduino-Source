/*  Synchronized Spinning
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_SynchronizedSpinning_H
#define PokemonAutomation_PokemonSwSh_SynchronizedSpinning_H

#include "NintendoSwitch/NintendoSwitch_MultiSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class SynchronizedSpinning_Descriptor : public MultiSwitchProgramDescriptor{
public:
    SynchronizedSpinning_Descriptor();
};



class SynchronizedSpinning : public MultiSwitchProgramInstance{
public:
    SynchronizedSpinning();

    virtual void program(MultiSwitchProgramEnvironment& env, CancellableScope& scope) override;
};




}
}
}
#endif
