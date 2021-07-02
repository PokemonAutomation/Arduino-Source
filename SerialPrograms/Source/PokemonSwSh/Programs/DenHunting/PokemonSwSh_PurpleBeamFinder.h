/*  Purple Beam Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_PurpleBeamFinder_H
#define PokemonAutomation_PokemonSwSh_PurpleBeamFinder_H

#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/BooleanCheckBox.h"
#include "CommonFramework/Options/FloatingPoint.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class PurpleBeamFinder_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    PurpleBeamFinder_Descriptor();
};



class PurpleBeamFinder : public SingleSwitchProgramInstance{
public:
    PurpleBeamFinder(const PurpleBeamFinder_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    struct Stats;

    BooleanCheckBox EXTRA_LINE;

    SectionDivider m_advanced_options;
    BooleanCheckBox SAVE_SCREENSHOT;
    TimeExpression<uint16_t> TIMEOUT_DELAY;
//    FloatingPoint MAX_STDDEV;
    FloatingPoint MIN_BRIGHTNESS;
    FloatingPoint MIN_EUCLIDEAN;
    FloatingPoint MIN_DELTA_STDDEV_RATIO;
    FloatingPoint MIN_SIGMA_STDDEV_RATIO;
};


}
}
}
#endif

