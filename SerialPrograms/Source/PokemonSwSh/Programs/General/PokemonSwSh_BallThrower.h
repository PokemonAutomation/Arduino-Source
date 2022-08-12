/*  Ball Thrower
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_BallThrower_H
#define PokemonAutomation_PokemonSwSh_BallThrower_H

#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"

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

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    StartInGripOrGameOption START_IN_GRIP_MENU;
};

}
}
}
#endif



