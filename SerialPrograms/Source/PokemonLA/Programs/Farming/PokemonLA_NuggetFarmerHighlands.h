/*  Money Farmer (Highlands)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_MoneyFarmerHighlands_H
#define PokemonAutomation_PokemonLA_MoneyFarmerHighlands_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class NuggetFarmerHighlands_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    NuggetFarmerHighlands_Descriptor();
};


class MoneyFarmerHighlands : public SingleSwitchProgramInstance{
public:
    MoneyFarmerHighlands(const NuggetFarmerHighlands_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    bool run_iteration(SingleSwitchProgramEnvironment& env);

private:
    class Stats;
    class RunRoute;

    ShinyDetectedActionOption SHINY_DETECTED;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
