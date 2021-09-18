/*  Purple Beam Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_PurpleBeamFinder_H
#define PokemonAutomation_PokemonSwSh_PurpleBeamFinder_H

#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/FloatingPointOption.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
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

    StartInGripOrGameOption START_IN_GRIP_MENU;
    BooleanCheckBoxOption EXTRA_LINE;

    SectionDividerOption m_advanced_options;
    BooleanCheckBoxOption SAVE_SCREENSHOT;
    TimeExpressionOption<uint16_t> TIMEOUT_DELAY;
//    FloatingPoint MAX_STDDEV;
    FloatingPointOption MIN_BRIGHTNESS;
    FloatingPointOption MIN_EUCLIDEAN;
    FloatingPointOption MIN_DELTA_STDDEV_RATIO;
    FloatingPointOption MIN_SIGMA_STDDEV_RATIO;
};


}
}
}
#endif

