/*  Shiny Hunt - Shaymin Runaway
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_ShinyHuntShaymin_H
#define PokemonAutomation_PokemonBDSP_ShinyHuntShaymin_H

#include "CommonFramework/Options/StaticTextOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/GoHomeWhenDoneOption.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "Pokemon/Options/Pokemon_EncounterBotOptions.h"
#include "PokemonBDSP/Options/PokemonBDSP_ShortcutDirection.h"
#include "PokemonBDSP/Options/PokemonBDSP_EncounterBotCommon.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class ShinyHuntShaymin_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    ShinyHuntShaymin_Descriptor();
};


class ShinyHuntShaymin : public SingleSwitchProgramInstance{
public:
    ShinyHuntShaymin(const ShinyHuntShaymin_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;


private:
    bool start_encounter(SingleSwitchProgramEnvironment& env) const;

private:
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    ShortcutDirection SHORTCUT;

    EncounterBotCommonOptions ENCOUNTER_BOT_OPTIONS;

    EventNotificationOption NOTIFICATION_PROGRAM_FINISH;
    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
    TimeExpressionOption<uint16_t> EXIT_BATTLE_TIMEOUT;
};



}
}
}
#endif
