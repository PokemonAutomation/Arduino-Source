/*  Shiny Hunt Autonomous - BerryTree
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousBerryTree_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousBerryTree_H

#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/BooleanCheckBox.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class ShinyHuntAutonomousBerryTree : public SingleSwitchProgram{
public:
    ShinyHuntAutonomousBerryTree();

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    struct Stats;

    BooleanCheckBox GO_HOME_WHEN_DONE;
    BooleanCheckBox REQUIRE_SQUARE;
    SectionDivider m_advanced_options;
    TimeExpression<uint16_t> EXIT_BATTLE_TIMEOUT;
    BooleanCheckBox VIDEO_ON_SHINY;
    BooleanCheckBox RUN_FROM_EVERYTHING;
};

}
}
}
#endif
