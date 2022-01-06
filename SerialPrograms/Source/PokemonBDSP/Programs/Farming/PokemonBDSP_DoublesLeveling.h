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
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonBDSP/Options/PokemonBDSP_EncounterBotCommon.h"
#include "PokemonBDSP/Programs/PokemonBDSP_OverworldTrigger.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class DoublesLeveling_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    DoublesLeveling_Descriptor();
};


class DoublesLeveling : public SingleSwitchProgramInstance{
public:
    DoublesLeveling(const DoublesLeveling_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;


private:
    struct Stats;
    bool battle(SingleSwitchProgramEnvironment& env);

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    Pokemon::EncounterBotLanguage LANGUAGE;

    OverworldTrigger TRIGGER_METHOD;
    EnumDropdownOption ON_LEARN_MOVE;

    EncounterBotCommonOptions ENCOUNTER_BOT_OPTIONS;

    EventNotificationOption NOTIFICATION_PROGRAM_FINISH;
    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
//    TimeExpressionOption<uint16_t> WATCHDOG_TIMER;
    TimeExpressionOption<uint16_t> EXIT_BATTLE_TIMEOUT;
};




}
}
}
#endif
