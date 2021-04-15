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

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class ShinyHuntAutonomousOverworld : public SingleSwitchProgram{
public:
    ShinyHuntAutonomousOverworld();

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) const override;


private:
    struct Stats;

    void move_in_circle(
        SingleSwitchProgramEnvironment& env,
        uint8_t size_ticks,
        uint8_t current_direction_x,
        uint8_t current_direction_y
    ) const;

    enum WatchResult{
        TIMEOUT,
        BATTLE_START,
        BATTLE_MENU,
    };
    WatchResult whistle_and_watch(
        SingleSwitchProgramEnvironment& env,
        std::vector<InferenceBox>& exclamations,
        std::vector<InferenceBox>& questions
    ) const;

    bool find_encounter(
        SingleSwitchProgramEnvironment& env,
        Stats& stats,
        StandardEncounterTracker& tracker
    ) const;


private:
    BooleanCheckBox GO_HOME_WHEN_DONE;
    BooleanCheckBox PRIORITIZE_EXCLAMATION_POINTS;
    BooleanCheckBox TARGET_CIRCLING;
    SimpleInteger<uint8_t> LOCAL_CIRCLING;
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
