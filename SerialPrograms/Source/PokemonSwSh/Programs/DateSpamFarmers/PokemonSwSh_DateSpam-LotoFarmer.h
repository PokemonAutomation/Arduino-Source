/*  Loto Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_LotoFarmer_H
#define PokemonAutomation_PokemonSwSh_LotoFarmer_H

#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class LotoFarmer : public SingleSwitchProgram{
public:
    LotoFarmer();

    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    SimpleInteger<uint32_t> SKIPS;
    TimeExpression<uint16_t> MASH_B_DURATION;
};


}
}
}
#endif



