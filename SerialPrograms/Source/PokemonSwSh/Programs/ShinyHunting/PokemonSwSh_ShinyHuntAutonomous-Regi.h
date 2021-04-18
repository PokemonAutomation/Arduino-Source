/*  Shiny Hunt Autonomous - Regi
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousRegi_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousRegi_H

#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/BooleanCheckBox.h"
#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "PokemonSwSh/Options/RegiSelector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class ShinyHuntAutonomousRegi : public SingleSwitchProgram{
public:
    ShinyHuntAutonomousRegi();

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    struct Stats;

    BooleanCheckBox GO_HOME_WHEN_DONE;
    RegiSelector REGI_NAME;
    BooleanCheckBox REQUIRE_SQUARE;
    TimeExpression<uint32_t> TOUCH_DATE_INTERVAL;
    SectionDivider m_advanced_options;
    TimeExpression<uint16_t> EXIT_BATTLE_MASH_TIME;
    TimeExpression<uint16_t> TRANSITION_DELAY;
    BooleanCheckBox VIDEO_ON_SHINY;
    BooleanCheckBox RUN_FROM_EVERYTHING;
};

}
}
}
#endif
