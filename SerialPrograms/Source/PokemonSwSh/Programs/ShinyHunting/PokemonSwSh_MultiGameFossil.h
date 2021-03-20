/*  Multiple Game Fossil
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MultiGameFossil_H
#define PokemonAutomation_PokemonSwSh_MultiGameFossil_H

#include "PokemonSwSh/Options/FossilTable.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class MultiGameFossil : public SingleSwitchProgram{
public:
    MultiGameFossil();

    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    FossilTable GAME_LIST;
};


}
}
}
#endif

