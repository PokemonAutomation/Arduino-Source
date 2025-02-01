/*  TotK Bow Item Duper
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ZeldaTotK_BowItemDuper_H
#define PokemonAutomation_ZeldaTotK_BowItemDuper_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace ZeldaTotK{

class BowItemDuper_Descriptor : public SingleSwitchProgramDescriptor{
public:
    BowItemDuper_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class BowItemDuper : public SingleSwitchProgramInstance{
public:
    BowItemDuper();
    virtual void program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context) override;

private:
    SimpleIntegerOption<uint32_t> ATTEMPTS;
    TimeExpressionOption<uint16_t> TICK_DELAY;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};

}
}
}
#endif



