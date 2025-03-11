/*  Clone Items
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_CloneItems_H
#define PokemonAutomation_PokemonSV_CloneItems_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class CloneItems101_Descriptor : public SingleSwitchProgramDescriptor{
public:
    CloneItems101_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;

};




class CloneItems101 : public SingleSwitchProgramInstance{
public:
    CloneItems101();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;


private:
    //  Returns true on success.
    bool clone_item(ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context);

private:
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    SimpleIntegerOption<uint16_t> ITEMS_TO_CLONE;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};




}
}
}
#endif
