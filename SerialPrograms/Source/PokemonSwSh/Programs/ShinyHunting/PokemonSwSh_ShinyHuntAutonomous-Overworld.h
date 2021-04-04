/*  Shiny Hunt Autonomous - Overworld
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousOverworld_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousOverworld_H

#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/BooleanCheckBox.h"
#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "PokemonSwSh_EncounterTracker.h"
#include "PokemonSwSh_EncounterStats.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class ShinyHuntAutonomousOverworld : public SingleSwitchProgram{
public:
    ShinyHuntAutonomousOverworld();

    virtual void program(SingleSwitchProgramEnvironment& env) const override;


private:
    struct Stats : public EncounterStats{
        Stats() : EncounterStats(true) {}
        virtual std::string stats() const override;
        uint64_t m_timeouts = 0;
        uint64_t m_unexpected_battles = 0;
        uint64_t m_resets = 0;
    };

    void move_in_circle(SingleSwitchProgramEnvironment& env, uint8_t size_ticks) const;
    bool find_encounter(
        SingleSwitchProgramEnvironment& env,
        Stats& stats,
        StandardEncounterTracker& tracker
    ) const;


private:
    BooleanCheckBox GO_HOME_WHEN_DONE;
    BooleanCheckBox PRIORITIZE_EXCLAMATION_POINTS;
    BooleanCheckBox ENABLE_CIRCLING;
    TimeExpression<uint16_t> MAX_MOVE_DURATION;
    TimeExpression<uint16_t> WATCHDOG_TIMER;
    SectionDivider m_advanced_options;
    TimeExpression<uint16_t> EXIT_BATTLE_MASH_TIME;
    SimpleInteger<uint8_t> TIME_ROLLBACK_HOURS;
};


}
}
}
#endif
