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

class SynchronizedSpinning : public MultiSwitchProgram{
public:
    SynchronizedSpinning();

    virtual void program(MultiSwitchProgramEnvironment& env) const override;
};




}
}
}
#endif
