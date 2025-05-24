/*  Three-Segment Dudunsparce Finder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_ThreeSegmentDudunsparceFinder_H
#define PokemonAutomation_PokemonSV_ThreeSegmentDudunsparceFinder_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class ThreeSegmentDudunsparceFinder_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ThreeSegmentDudunsparceFinder_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class ThreeSegmentDudunsparceFinder : public SingleSwitchProgramInstance{
public:
    ThreeSegmentDudunsparceFinder();
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
