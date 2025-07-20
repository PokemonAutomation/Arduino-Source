/*  Program Name
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_GameName_ProgramName_H
#define PokemonAutomation_GameName_ProgramName_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace GameName{


class ProgramName_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ProgramName_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class ProgramName : public SingleSwitchProgramInstance{
public:
    ProgramName();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    
private:

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;


    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};




}
}
}
#endif
