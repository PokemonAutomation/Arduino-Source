/*  Shiny Hunt Autonomous - Overworld
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousOverworld_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousOverworld_H

#include "CommonFramework/Options/StaticTextOption.h"
#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "CommonFramework/Options/FloatingPointOption.h"
#include "CommonFramework/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/Options/GoHomeWhenDoneOption.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Options/Pokemon_EncounterBotOptions.h"
#include "PokemonSwSh/Options/PokemonSwSh_EncounterBotCommon.h"
#include "PokemonSwSh_OverworldTargetTracker.h"

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
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;


private:
    struct Stats;

    bool find_encounter(
        SingleSwitchProgramEnvironment& env, BotBaseContext& context,
        Stats& stats,
        WallClock expiration
    ) const;

    bool charge_at_target(
        ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
        const std::pair<double, OverworldTarget>& target
    ) const;

private:
    StartInGripOrGameOption START_IN_GRIP_MENU;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    SimpleIntegerOption<uint8_t> TIME_ROLLBACK_HOURS;

    EncounterBotLanguage LANGUAGE;

    FloatingPointOption MARK_OFFSET;
    EnumDropdownOption MARK_PRIORITY;
    EnumDropdownOption TRIGGER_METHOD;
    TimeExpressionOption<uint16_t> MAX_MOVE_DURATION;
    FloatingPointOption MAX_TARGET_ALPHA;

    EncounterBotCommonOptions ENCOUNTER_BOT_OPTIONS;

    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
    TimeExpressionOption<uint16_t> WATCHDOG_TIMER;
    TimeExpressionOption<uint16_t> EXIT_BATTLE_TIMEOUT;
    BooleanCheckBoxOption TARGET_CIRCLING;
};


}
}
}
#endif
