/*  Shiny Hunt Autonomous - IoATrade
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousIoATrade_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousIoATrade_H

#include "Common/Qt/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/StaticTextOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/Options/GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Options/Pokemon_EncounterBotOptions.h"
#include "PokemonSwSh/Options/PokemonSwSh_DateToucher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class ShinyHuntAutonomousIoATrade_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyHuntAutonomousIoATrade_Descriptor();

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class ShinyHuntAutonomousIoATrade : public SingleSwitchProgramInstance{
public:
    ShinyHuntAutonomousIoATrade();
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    StartInGripOrGameOption START_IN_GRIP_MENU;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    TouchDateIntervalOption TOUCH_DATE_INTERVAL;

    BooleanCheckBoxOption VIDEO_ON_SHINY;
    EventNotificationOption NOTIFICATION_NONSHINY;
    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
    TimeExpressionOption<uint16_t> MASH_TO_TRADE_DELAY;
    BooleanCheckBoxOption RUN_FROM_EVERYTHING;
};


}
}
}
#endif
