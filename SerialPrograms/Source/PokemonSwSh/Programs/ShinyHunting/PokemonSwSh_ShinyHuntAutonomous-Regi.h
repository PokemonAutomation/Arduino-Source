/*  ShinyHuntAutonomous-Regi
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
#include "PokemonSwSh_EncounterStats.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class ShinyHuntAutonomousRegi : public SingleSwitchProgram{
public:
    ShinyHuntAutonomousRegi();

    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    struct Stats : public EncounterStats{
        Stats() : EncounterStats(true) {}
        virtual std::string stats() const override;
        uint64_t m_light_resets = 0;
    };

    RegiSelector REGI_NAME;
    BooleanCheckBox REQUIRE_SQUARE;
    SectionDivider m_advanced_options;
    TimeExpression<uint16_t> EXIT_BATTLE_MASH_TIME;
    TimeExpression<uint16_t> TRANSITION_DELAY;
    TimeExpression<uint32_t> TOUCH_DATE_INTERVAL;
};

}
}
}
#endif
