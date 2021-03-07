/*  Day Skipper (JPN) - 7.8k version
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DaySkipperJPN7p8k_H
#define PokemonAutomation_PokemonSwSh_DaySkipperJPN7p8k_H

#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Options/SwitchDate.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class DaySkipperJPN7p8k : public SingleSwitchProgram{
public:
    DaySkipperJPN7p8k();

    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    SimpleInteger<uint32_t> SKIPS;
    SwitchDate START_DATE;
    SectionDivider m_advanced_options;
    SimpleInteger<uint16_t> CORRECTION_SKIPS;
};


}
}
}
#endif



