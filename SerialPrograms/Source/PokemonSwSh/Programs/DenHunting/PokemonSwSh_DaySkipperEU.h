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


class DaySkipperEU_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    DaySkipperEU_Descriptor();
};



class DaySkipperEU : public SingleSwitchProgramInstance{
public:
    DaySkipperEU(const DaySkipperEU_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

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



