/*  Shiny Hunt Autonomous - Swords Of Justice
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousSwordsOfJustice_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousSwordsOfJustice_H

#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/BooleanCheckBox.h"
#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class ShinyHuntAutonomousSwordsOfJustice : public SingleSwitchProgram{
public:
    ShinyHuntAutonomousSwordsOfJustice();

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    struct Stats;

    BooleanCheckBox GO_HOME_WHEN_DONE;
    BooleanCheckBox AIRPLANE_MODE;
    SimpleInteger<uint8_t> TIME_ROLLBACK_HOURS;
    SectionDivider m_advanced_options;
    TimeExpression<uint16_t> EXIT_BATTLE_TIMEOUT;
    TimeExpression<uint16_t> POST_BATTLE_MASH_TIME;
    TimeExpression<uint16_t> ENTER_CAMP_DELAY;
    BooleanCheckBox VIDEO_ON_SHINY;
    BooleanCheckBox RUN_FROM_EVERYTHING;
};

}
}
}
#endif
