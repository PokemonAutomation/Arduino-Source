/*  Cafe Buyer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_CafeBuyer_H
#define PokemonAutomation_PokemonLZA_CafeBuyer_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

class CafeBuyer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    CafeBuyer_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class CafeBuyer : public SingleSwitchProgramInstance{
public:
    CafeBuyer();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    void purchase_water(SingleSwitchProgramEnvironment& env, ProControllerContext& context, bool first);
    void handle_purchase(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    SimpleIntegerOption<uint32_t> PURCHASES;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
