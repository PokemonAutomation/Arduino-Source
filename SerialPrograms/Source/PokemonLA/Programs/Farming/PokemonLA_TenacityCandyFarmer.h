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
#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "CommonFramework/Options/EnumDropdownOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class TenacityCandyFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    TenacityCandyFarmer_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class TenacityCandyFarmer : public SingleSwitchProgramInstance{
public:
    TenacityCandyFarmer();
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    bool run_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

private:
    EnumDropdownOption FOURTH_MOVE_ON;

    SimpleIntegerOption<uint32_t> SAVE_EVERY_FEW_BATTLES;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
