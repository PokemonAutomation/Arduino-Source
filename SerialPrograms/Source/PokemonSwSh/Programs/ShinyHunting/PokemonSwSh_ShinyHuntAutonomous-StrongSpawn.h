/*  Shiny Hunt Autonomous - StrongSpawn
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousStrongSpawn_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousStrongSpawn_H

#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/BooleanCheckBox.h"
#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "PokemonSwSh_EncounterTracker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class ShinyHuntAutonomousStrongSpawn : public SingleSwitchProgram{
public:
    ShinyHuntAutonomousStrongSpawn();

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    struct Stats;
    struct Tracker : public StandardEncounterTracker{
        Tracker(ShinyHuntTracker& stats, ConsoleHandle& console);
        virtual bool run_away() override;
    };

    BooleanCheckBox GO_HOME_WHEN_DONE;
    SectionDivider m_advanced_options;
    TimeExpression<uint16_t> EXIT_BATTLE_MASH_TIME;
    SimpleInteger<uint8_t> TIME_ROLLBACK_HOURS;
};

}
}
}
#endif
