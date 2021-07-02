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
#include "CommonFramework/Options/FloatingPoint.h"
#include "CommonFramework/Options/EnumDropdown.h"
#include "CommonFramework/Options/LanguageOCR.h"
#include "Pokemon/Pokemon_NameReader.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_EncounterTracker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class ShinyHuntAutonomousOverworld_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    ShinyHuntAutonomousOverworld_Descriptor();
};



class ShinyHuntAutonomousOverworld : public SingleSwitchProgramInstance{
public:
    ShinyHuntAutonomousOverworld(const ShinyHuntAutonomousOverworld_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;


private:
    struct Stats;

    bool find_encounter(
        SingleSwitchProgramEnvironment& env,
        Stats& stats,
        StandardEncounterTracker& tracker,
        std::chrono::system_clock::time_point expiration
    ) const;


private:
    BooleanCheckBox GO_HOME_WHEN_DONE;

    Pokemon::PokemonNameReader m_name_reader;
    LanguageOCR LANGUAGE;

    FloatingPoint MARK_OFFSET;
    EnumDropdown MARK_PRIORITY;
    EnumDropdown TRIGGER_METHOD;
    TimeExpression<uint16_t> MAX_MOVE_DURATION;
    TimeExpression<uint16_t> WATCHDOG_TIMER;
    SimpleInteger<uint8_t> TIME_ROLLBACK_HOURS;
    SectionDivider m_advanced_options;
    TimeExpression<uint16_t> EXIT_BATTLE_TIMEOUT;
    BooleanCheckBox TARGET_CIRCLING;
    FloatingPoint MAX_TARGET_ALPHA;
    BooleanCheckBox VIDEO_ON_SHINY;
    BooleanCheckBox RUN_FROM_EVERYTHING;
};


}
}
}
#endif
