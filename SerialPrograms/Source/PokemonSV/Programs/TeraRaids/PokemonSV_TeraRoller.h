/*  Tera Roller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_TeraRoller_H
#define PokemonAutomation_PokemonSV_TeraRoller_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSV/Options/PokemonSV_TeraRollFilter.h"

namespace PokemonAutomation{
   struct VideoSnapshot;
namespace NintendoSwitch{
namespace PokemonSV{


class TeraRoller_Descriptor : public SingleSwitchProgramDescriptor{
public:
    TeraRoller_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;

};




class TeraRoller : public SingleSwitchProgramInstance{
public:
    TeraRoller();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    TeraRollFilter FILTER0;

    BooleanCheckBoxOption CHECK_ONLY_FIRST;
    SimpleIntegerOption<uint8_t> PERIODIC_RESET;

    //  Notifications
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationOption m_notification_noop;
    EventNotificationsOption NOTIFICATIONS;

};




}
}
}
#endif
