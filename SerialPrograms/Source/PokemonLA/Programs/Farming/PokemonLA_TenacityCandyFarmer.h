/*  Tenacity Candy Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Battle Pearl Clan in Path of Tenacity to farm exp, Exp Candy XL and random evolution items.
 */

#ifndef PokemonAutomation_PokemonLA_TenacityCandyFarmer_H
#define PokemonAutomation_PokemonLA_TenacityCandyFarmer_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
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
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    bool run_iteration(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

private:
    enum class FourthMoveOn{
        None,
        Mamoswine,
        Avalugg,
    };
    EnumDropdownOption<FourthMoveOn> FOURTH_MOVE_ON;

    SimpleIntegerOption<uint32_t> SAVE_EVERY_FEW_BATTLES;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
