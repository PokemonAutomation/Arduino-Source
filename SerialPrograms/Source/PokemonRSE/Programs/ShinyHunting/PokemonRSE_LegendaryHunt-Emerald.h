/*  Legendary Hunt - Emerald
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonRSE_LegendaryHuntEmerald_H
#define PokemonAutomation_PokemonRSE_LegendaryHuntEmerald_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{

class LegendaryHuntEmerald_Descriptor : public SingleSwitchProgramDescriptor{
public:
    LegendaryHuntEmerald_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class LegendaryHuntEmerald : public SingleSwitchProgramInstance{
public:
    LegendaryHuntEmerald();
    virtual void program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context) override;

private:
    enum class Target{
        regis,
        hooh,
        lugia,
        latis,
    };
    EnumDropdownOption<Target> TARGET;

    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;

    void reset_regi(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context);
    void reset_hooh(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context);
    void reset_lugia(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context);
};

}
}
}
#endif

