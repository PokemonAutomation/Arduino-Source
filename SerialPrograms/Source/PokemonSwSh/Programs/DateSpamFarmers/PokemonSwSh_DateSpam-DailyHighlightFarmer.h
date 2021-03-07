/*  Daily Highlight Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DailyHighlightFarmer_H
#define PokemonAutomation_PokemonSwSh_DailyHighlightFarmer_H

#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class DailyHighlightFarmer : public SingleSwitchProgram{
public:
    DailyHighlightFarmer();

    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    SimpleInteger<uint32_t> SKIPS;
    SimpleInteger<uint16_t> SAVE_ITERATIONS;
};


}
}
}
#endif



