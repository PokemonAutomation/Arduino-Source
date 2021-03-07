/*  Berry Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_BerryFarmer_H
#define PokemonAutomation_PokemonSwSh_BerryFarmer_H

#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class BerryFarmer : public SingleSwitchProgram{
public:
    BerryFarmer();

    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    SimpleInteger<uint32_t> SKIPS;
    SimpleInteger<uint16_t> SAVE_ITERATIONS;
};


}
}
}
#endif



