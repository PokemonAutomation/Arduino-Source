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
#include "NintendoSwitch/Options/StartInGripMenu.h"
#include "NintendoSwitch/Options/GoHomeWhenDone.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "Pokemon/Options/Pokemon_EncounterBotOptions.h"
#include "PokemonSwSh/Options/PokemonSwSh_EncounterBotCommon.h"

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
        std::chrono::system_clock::time_point expiration
    ) const;


private:
    StartInGripOrGame START_IN_GRIP_MENU;
    GoHomeWhenDone GO_HOME_WHEN_DONE;
    SimpleInteger<uint8_t> TIME_ROLLBACK_HOURS;

    Pokemon::EncounterBotLanguage LANGUAGE;

    FloatingPoint MARK_OFFSET;
    EnumDropdown MARK_PRIORITY;
    EnumDropdown TRIGGER_METHOD;
    TimeExpression<uint16_t> MAX_MOVE_DURATION;
    FloatingPoint MAX_TARGET_ALPHA;

    EncounterBotCommonOptions ENCOUNTER_BOT_OPTIONS;

    SectionDivider m_advanced_options;
    TimeExpression<uint16_t> WATCHDOG_TIMER;
    TimeExpression<uint16_t> EXIT_BATTLE_TIMEOUT;
    BooleanCheckBox TARGET_CIRCLING;
};


}
}
}
#endif
