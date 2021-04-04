/*  Shiny Hunt Autonomous - IoATrade
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousIoATrade_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousIoATrade_H

#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/BooleanCheckBox.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "PokemonSwSh_EncounterStats.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class ShinyHuntAutonomousIoATrade : public SingleSwitchProgram{
public:
    ShinyHuntAutonomousIoATrade();

    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    struct Stats : public EncounterStats{
        Stats() : EncounterStats(false) {}
        virtual std::string stats() const override;
        uint64_t m_errors = 0;
    };

    BooleanCheckBox GO_HOME_WHEN_DONE;
    SectionDivider m_advanced_options;
    TimeExpression<uint16_t> MASH_TO_TRADE_DELAY;
    TimeExpression<uint32_t> TOUCH_DATE_INTERVAL;
};


}
}
}
#endif
