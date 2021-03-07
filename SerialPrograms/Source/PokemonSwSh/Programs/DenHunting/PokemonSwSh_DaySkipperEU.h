/*  Day Skipper (EU)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DaySkipperEU_H
#define PokemonAutomation_PokemonSwSh_DaySkipperEU_H

#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class DaySkipperEU : public SingleSwitchProgram{
public:
    DaySkipperEU();

    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    SimpleInteger<uint32_t> SKIPS;
    SimpleInteger<uint16_t> REAL_LIFE_YEAR;
    SectionDivider m_advanced_options;
    SimpleInteger<uint16_t> CORRECTION_SKIPS;
};


}
}
}
#endif



