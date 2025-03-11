/*  TotK Paraglide Item Duper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ZeldaTotK_ParaglideItemDuper_H
#define PokemonAutomation_ZeldaTotK_ParaglideItemDuper_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace ZeldaTotK{

class ParaglideItemDuper_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ParaglideItemDuper_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class ParaglideItemDuper : public SingleSwitchProgramInstance{
public:
    ParaglideItemDuper();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    SimpleIntegerOption<uint32_t> ATTEMPTS;
    MillisecondsOption LOAD_DELAY0;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};

}
}
}
#endif



