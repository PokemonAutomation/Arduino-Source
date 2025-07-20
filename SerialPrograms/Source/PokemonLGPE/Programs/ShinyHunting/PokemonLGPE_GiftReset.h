/*  LGPE Gift Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLGPE_GiftReset_H
#define PokemonAutomation_PokemonLGPE_GiftReset_H

#include "NintendoSwitch/Controllers/NintendoSwitch_Joycon.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLGPE{

class GiftReset_Descriptor : public SingleSwitchProgramDescriptor{
public:
    GiftReset_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class GiftReset : public SingleSwitchProgramInstance{
public:
    GiftReset();
    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;

private:
    BooleanCheckBoxOption EXTRA_DIALOG;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};




}
}
}
#endif



