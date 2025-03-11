/*  Ball Thrower
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_BallThrower_H
#define PokemonAutomation_PokemonSwSh_BallThrower_H

#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class BallThrower_Descriptor : public SingleSwitchProgramDescriptor{
public:
    BallThrower_Descriptor();
};



class BallThrower : public SingleSwitchProgramInstance{
public:
    BallThrower();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
};

}
}
}
#endif



