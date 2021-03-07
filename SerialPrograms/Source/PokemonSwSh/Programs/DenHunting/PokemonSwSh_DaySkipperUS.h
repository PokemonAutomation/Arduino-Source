/*  Day Skipper (US)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DaySkipperUS_H
#define PokemonAutomation_PokemonSwSh_DaySkipperUS_H

#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class DaySkipperUS : public SingleSwitchProgram{
public:
    DaySkipperUS();

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



