/*  Multiple Game Fossil
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MultiGameFossil_H
#define PokemonAutomation_PokemonSwSh_MultiGameFossil_H

#include "PokemonSwSh/Options/PokemonSwSh_FossilTable.h"
#include "NintendoSwitch/Options/StartInGripMenu.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class MultiGameFossil_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    MultiGameFossil_Descriptor();
};



class MultiGameFossil : public SingleSwitchProgramInstance{
public:
    MultiGameFossil(const MultiGameFossil_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    StartInGripOrClosed START_IN_GRIP_MENU;
    FossilTable GAME_LIST;
};


}
}
}
#endif

