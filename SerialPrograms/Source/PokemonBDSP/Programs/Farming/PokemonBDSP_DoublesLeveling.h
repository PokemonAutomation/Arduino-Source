/*  Double Battle Leveling
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_DoublesLeveling_H
#define PokemonAutomation_PokemonBDSP_DoublesLeveling_H

#include "CommonFramework/Options/StaticTextOption.h"
#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonBDSP/Options/PokemonBDSP_EncounterBotCommon.h"
#include "PokemonBDSP/Programs/PokemonBDSP_OverworldTrigger.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class DoublesLeveling_Descriptor : public SingleSwitchProgramDescriptor{
public:
    DoublesLeveling_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class DoublesLeveling : public SingleSwitchProgramInstance{
public:
    DoublesLeveling();
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;


private:
    bool battle(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    Pokemon::EncounterBotLanguage LANGUAGE;

    OverworldTrigger TRIGGER_METHOD;
    EnumDropdownOption ON_LEARN_MOVE;

    EncounterBotCommonOptions ENCOUNTER_BOT_OPTIONS;

    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
//    TimeExpressionOption<uint16_t> WATCHDOG_TIMER;
    TimeExpressionOption<uint16_t> EXIT_BATTLE_TIMEOUT;
};




}
}
}
#endif
