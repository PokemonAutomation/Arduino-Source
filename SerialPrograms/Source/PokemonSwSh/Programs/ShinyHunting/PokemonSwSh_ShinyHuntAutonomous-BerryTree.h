/*  ShinyHuntAutonomous-BerryTree
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
#include "PokemonSwSh_EncounterStats.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class ShinyHuntAutonomousBerryTree : public SingleSwitchProgram{
public:
    ShinyHuntAutonomousBerryTree();

    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    struct Stats : public EncounterStats{
        Stats() : EncounterStats(true) {}
        virtual std::string stats() const override;
        uint64_t m_timeouts = 0;
    };

    BooleanCheckBox REQUIRE_SQUARE;
    SectionDivider m_advanced_options;
    TimeExpression<uint16_t> EXIT_BATTLE_MASH_TIME;
};

}
}
}
#endif
