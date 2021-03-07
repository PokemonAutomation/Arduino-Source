/*  Turbo A
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_TurboA_H
#define PokemonAutomation_PokemonSwSh_TurboA_H

#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class TurboA : public SingleSwitchProgram{
public:
    TurboA();

    virtual void program(SingleSwitchProgramEnvironment& env) const override;
};


}
}
}
#endif



