/*  LGPE Alolan Trade
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLGPE_AlolanTrade_H
#define PokemonAutomation_PokemonLGPE_AlolanTrade_H

#include "NintendoSwitch/Controllers/NintendoSwitch_Joycon.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLGPE{

class AlolanTrade_Descriptor : public SingleSwitchProgramDescriptor{
public:
    AlolanTrade_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class AlolanTrade : public SingleSwitchProgramInstance{
public:
    AlolanTrade();
    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;

private:
    void run_trade(SingleSwitchProgramEnvironment& env, JoyconContext& context);

    SimpleIntegerOption<uint32_t> NUM_TRADES;
    
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};




}
}
}
#endif



