/*  ShinyHuntAutonomous-Regigigas2
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousRegigigas2_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousRegigigas2_H

#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/BooleanCheckBox.h"
#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "PokemonSwSh_EncounterStats.h"
#include "PokemonSwSh_EncounterTracker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class ShinyHuntAutonomousRegigigas2 : public SingleSwitchProgram{
public:
    ShinyHuntAutonomousRegigigas2();

    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    bool kill_and_return(SingleSwitchProgramEnvironment& env) const;

private:
    struct Stats : public EncounterStats{
        Stats() : EncounterStats(true) {}
        virtual std::string stats() const override;
        uint64_t m_timeouts = 0;
    };
    struct Tracker : public StandardEncounterTracker{
        Tracker(
            EncounterStats& stats,
            ProgramEnvironment& env,
            Logger& logger,
            ConsoleHandle& console,
            bool require_square,
            uint16_t exit_battle_time
        );
        virtual bool run_away() override;

        ProgramEnvironment& m_env;
        Logger& m_logger;
    };

    SimpleInteger<uint8_t> REVERSAL_PP;
    BooleanCheckBox REQUIRE_SQUARE;
    SectionDivider m_advanced_options;
    TimeExpression<uint16_t> CATCH_TO_OVERWORLD_DELAY;
    TimeExpression<uint32_t> TOUCH_DATE_INTERVAL;
};

}
}
}
#endif
