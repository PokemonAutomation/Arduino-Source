/*  Emerald Starter Reset
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonEmerald_StarterReset_H
#define PokemonAutomation_PokemonEmerald_StarterReset_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonEmerald{

class StarterReset_Descriptor : public SingleSwitchProgramDescriptor{
public:
    StarterReset_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class StarterReset : public SingleSwitchProgramInstance{
public:
    StarterReset();
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    SimpleIntegerOption<uint32_t> ATTEMPTS;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};

}
}
}
#endif



