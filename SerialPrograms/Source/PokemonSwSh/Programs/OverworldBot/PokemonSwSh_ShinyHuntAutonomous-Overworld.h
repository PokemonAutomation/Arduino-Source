/*  Shiny Hunt Autonomous - Overworld
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousOverworld_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousOverworld_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Options/Pokemon_EncounterBotOptions.h"
#include "PokemonSwSh/Options/PokemonSwSh_EncounterBotCommon.h"
#include "PokemonSwSh_OverworldTargetTracker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Pokemon;


class ShinyHuntAutonomousOverworld_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyHuntAutonomousOverworld_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class ShinyHuntAutonomousOverworld : public SingleSwitchProgramInstance{
public:
    ShinyHuntAutonomousOverworld();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;


private:
    bool find_encounter(
        VideoStream& stream, ProControllerContext& context,
        ShinyHuntAutonomousOverworld_Descriptor::Stats& stats,
        WallClock expiration
    ) const;

    bool charge_at_target(
        VideoStream& stream, ProControllerContext& context,
        const std::pair<double, OverworldTarget>& target
    ) const;

private:
    StartInGripOrGameOption START_LOCATION;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    SimpleIntegerOption<uint8_t> TIME_ROLLBACK_HOURS;

    EncounterBotLanguage LANGUAGE;

    FloatingPointOption MARK_OFFSET;
    EnumDropdownOption<MarkPriority> MARK_PRIORITY;

    enum class TriggerMethod{
        WhistleOnly,
        Whistle3Circle1,
        Circle3Whistle3,
        CircleOnly,
        Horizontal,
        Whistle3Horizontal1,
        Horizontal3Whistle3,
        Vertical,
        Whistle3Vertical1,
        Vertical3Whistle3,
    };
    EnumDropdownOption<TriggerMethod> TRIGGER_METHOD;

    MillisecondsOption MAX_MOVE_DURATION0;
    FloatingPointOption MAX_TARGET_ALPHA;

    EncounterBotCommonOptions ENCOUNTER_BOT_OPTIONS;

    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
    MillisecondsOption WATCHDOG_TIMER0;
    MillisecondsOption EXIT_BATTLE_TIMEOUT0;
    BooleanCheckBoxOption TARGET_CIRCLING;
};


}
}
}
#endif
