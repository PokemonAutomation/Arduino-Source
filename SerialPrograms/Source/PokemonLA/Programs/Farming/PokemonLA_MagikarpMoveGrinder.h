/*  Magikarp Move Grinder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Battle a magikarp to grind move related research tasks.
 */

#ifndef PokemonAutomation_PokemonLA_MagikarpMoveGrinder_H
#define PokemonAutomation_PokemonLA_MagikarpMoveGrinder_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/EnumDropdownOption.h"
#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "CommonFramework/Options/BatchOption/GroupOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_BattlePokemonActionTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class MagikarpMoveGrinder_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    MagikarpMoveGrinder_Descriptor();
};

class MagikarpMoveGrinder : public SingleSwitchProgramInstance{
public:
    MagikarpMoveGrinder(const MagikarpMoveGrinder_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env, const BotBaseContext& context) override;

private:
    bool battle_magikarp(SingleSwitchProgramEnvironment& env, const BotBaseContext& context);

    void switch_pokemon(SingleSwitchProgramEnvironment& env, size_t& next_pokemon_in_party_order);

private:
    class Stats;

    OneMoveBattlePokemonActionTable POKEMON_ACTIONS;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
