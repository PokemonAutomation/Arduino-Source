/*  Day Skipper (JPN)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DaySkipperJPN_H
#define PokemonAutomation_PokemonSwSh_DaySkipperJPN_H

#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class DaySkipperJPN : public SingleSwitchProgram{
public:
    DaySkipperJPN();

    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    SimpleInteger<uint32_t> SKIPS;
    SectionDivider m_advanced_options;
    SimpleInteger<uint16_t> CORRECTION_SKIPS;
};


}
}
}
#endif



