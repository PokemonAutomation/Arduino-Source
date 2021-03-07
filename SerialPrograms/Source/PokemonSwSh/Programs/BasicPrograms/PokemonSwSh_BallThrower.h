/*  Ball Thrower
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_BallThrower_H
#define PokemonAutomation_PokemonSwSh_BallThrower_H

#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class BallThrower : public SingleSwitchProgram{
public:
    BallThrower();

    virtual void program(SingleSwitchProgramEnvironment& env) const override;

};

}
}
}
#endif



