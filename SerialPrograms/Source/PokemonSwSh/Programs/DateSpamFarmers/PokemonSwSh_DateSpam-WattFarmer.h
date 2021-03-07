/*  Watt Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_WattFarmer_H
#define PokemonAutomation_PokemonSwSh_WattFarmer_H

#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class WattFarmer : public SingleSwitchProgram{
public:
    WattFarmer();

    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    SimpleInteger<uint32_t> SKIPS;
    SimpleInteger<uint16_t> SAVE_ITERATIONS;
};


}
}
}
#endif



