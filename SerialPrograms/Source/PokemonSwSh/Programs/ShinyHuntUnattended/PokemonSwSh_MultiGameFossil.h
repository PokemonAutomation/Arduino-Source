/*  Multiple Game Fossil
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MultiGameFossil_H
#define PokemonAutomation_PokemonSwSh_MultiGameFossil_H

#include "Common/PokemonSwSh/PokemonSwSh_FossilTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class MultiGameFossil_Descriptor : public SingleSwitchProgramDescriptor{
public:
    MultiGameFossil_Descriptor();
};



class MultiGameFossil : public SingleSwitchProgramInstance{
public:
    MultiGameFossil();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    StartInGripOrClosedOption START_LOCATION;
    FossilTable GAME_LIST;
};


}
}
}
#endif

