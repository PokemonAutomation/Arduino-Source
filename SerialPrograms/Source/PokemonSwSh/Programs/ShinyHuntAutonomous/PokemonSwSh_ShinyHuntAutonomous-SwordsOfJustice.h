/*  Shiny Hunt Autonomous - Swords Of Justice
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousSwordsOfJustice_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousSwordsOfJustice_H

#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/Options/GoHomeWhenDoneOption.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "Pokemon/Options/Pokemon_EncounterBotOptions.h"
#include "PokemonSwSh/Options/PokemonSwSh_DateToucher.h"
#include "PokemonSwSh/Options/PokemonSwSh_EncounterBotCommon.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class ShinyHuntAutonomousSwordsOfJustice_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    ShinyHuntAutonomousSwordsOfJustice_Descriptor();
};



class ShinyHuntAutonomousSwordsOfJustice : public SingleSwitchProgramInstance{
public:
    ShinyHuntAutonomousSwordsOfJustice(const ShinyHuntAutonomousSwordsOfJustice_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    StartInGripOrGameOption START_IN_GRIP_MENU;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    TimeRollbackHoursOption TIME_ROLLBACK_HOURS;

    EncounterBotLanguage LANGUAGE;

    BooleanCheckBoxOption AIRPLANE_MODE;
    EncounterBotCommonOptions ENCOUNTER_BOT_OPTIONS;

    EventNotificationOption NOTIFICATION_PROGRAM_FINISH;
    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
    TimeExpressionOption<uint16_t> EXIT_BATTLE_TIMEOUT;
    TimeExpressionOption<uint16_t> POST_BATTLE_MASH_TIME;
    TimeExpressionOption<uint16_t> ENTER_CAMP_DELAY;
};

}
}
}
#endif
