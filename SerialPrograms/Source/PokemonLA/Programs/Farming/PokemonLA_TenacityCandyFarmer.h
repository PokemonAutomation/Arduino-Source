/*  Tenacity Candy Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Battle Pearl Clan in Path of Tenacity to farm exp, Exp Candy XL and random evolution items.
 */

#ifndef PokemonAutomation_PokemonLA_TenacityCandyFarmer_H
#define PokemonAutomation_PokemonLA_TenacityCandyFarmer_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class TenacityCandyFarmer_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    TenacityCandyFarmer_Descriptor();
};

class TenacityCandyFarmer : public SingleSwitchProgramInstance{
public:
    TenacityCandyFarmer(const TenacityCandyFarmer_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    bool run_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

private:
    class Stats;

    BooleanCheckBoxOption FOURTH_MOVE_ON_MAMOSWINE;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
