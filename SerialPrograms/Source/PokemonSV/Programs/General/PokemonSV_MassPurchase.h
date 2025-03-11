/*  Mass Purchase
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_MassPurchase_H
#define PokemonAutomation_PokemonSV_MassPurchase_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class MassPurchase_Descriptor : public SingleSwitchProgramDescriptor{
public:
    MassPurchase_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class MassPurchase : public SingleSwitchProgramInstance{
public:
    MassPurchase();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    bool mass_purchase(ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context);
    bool extra_items(ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context);
    
private:
    SimpleIntegerOption<uint16_t> ITEMS;
    SimpleIntegerOption<uint16_t> QUANTITY;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    BooleanCheckBoxOption PAY_LP;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
