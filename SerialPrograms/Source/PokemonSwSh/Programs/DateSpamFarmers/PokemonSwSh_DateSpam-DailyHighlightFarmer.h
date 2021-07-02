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


class DailyHighlightFarmer_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    DailyHighlightFarmer_Descriptor();
};



class DailyHighlightFarmer : public SingleSwitchProgramInstance{
public:
    DailyHighlightFarmer(const DailyHighlightFarmer_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    SimpleInteger<uint32_t> SKIPS;
    SimpleInteger<uint16_t> SAVE_ITERATIONS;
};


}
}
}
#endif



