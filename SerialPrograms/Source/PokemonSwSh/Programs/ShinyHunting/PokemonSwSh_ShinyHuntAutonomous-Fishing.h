/*  ShinyHuntAutonomous-Fishing
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousFishing_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousFishing_H

#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "PokemonSwSh_EncounterStats.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class ShinyHuntAutonomousFishing : public SingleSwitchProgram{
public:
    ShinyHuntAutonomousFishing();

    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    struct Stats : public EncounterStats{
        Stats() : EncounterStats(true) {}
        virtual std::string stats() const override;
        uint64_t m_misses = 0;
        uint64_t m_timeouts = 0;
        uint64_t m_unexpected_battles = 0;
    };

    SectionDivider m_advanced_options;
    TimeExpression<uint16_t> EXIT_BATTLE_MASH_TIME;
    TimeExpression<uint16_t> FISH_RESPAWN_TIME;
    SimpleInteger<uint8_t> TIME_ROLLBACK_HOURS;
};


}
}
}
#endif
