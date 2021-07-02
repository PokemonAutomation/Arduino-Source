/*  Synchronized Spinning
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_SynchronizedSpinning_H
#define PokemonAutomation_PokemonSwSh_SynchronizedSpinning_H

#include "NintendoSwitch/Framework/MultiSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class SynchronizedSpinning_Descriptor : public MultiSwitchProgramDescriptor{
public:
    SynchronizedSpinning_Descriptor();
};



class SynchronizedSpinning : public MultiSwitchProgramInstance{
public:
    SynchronizedSpinning(const SynchronizedSpinning_Descriptor& description);

    virtual void program(MultiSwitchProgramEnvironment& env) override;
};




}
}
}
#endif
