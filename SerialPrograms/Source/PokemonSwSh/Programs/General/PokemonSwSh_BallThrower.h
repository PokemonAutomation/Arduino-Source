/*  Ball Thrower
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_BallThrower_H
#define PokemonAutomation_PokemonSwSh_BallThrower_H

#include "NintendoSwitch/Options/StartInGripMenu.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class BallThrower_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    BallThrower_Descriptor();
};



class BallThrower : public SingleSwitchProgramInstance{
public:
    BallThrower(const BallThrower_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    StartInGripOrGame START_IN_GRIP_MENU;
};

}
}
}
#endif



