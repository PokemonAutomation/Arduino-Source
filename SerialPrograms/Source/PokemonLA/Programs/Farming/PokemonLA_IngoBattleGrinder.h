/*  Ingo Battle Grinder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_IngoBattleGrinder_H
#define PokemonAutomation_PokemonLA_IngoBattleGrinder_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_IngoOpponent.h"
#include "PokemonLA/Options/PokemonLA_BattlePokemonActionTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{





class IngoBattleGrinder_Descriptor : public SingleSwitchProgramDescriptor{
public:
    IngoBattleGrinder_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class IngoBattleGrinder : public SingleSwitchProgramInstance{
public:
    IngoBattleGrinder();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    bool run_iteration(SingleSwitchProgramEnvironment& env, ProControllerContext& context, std::map<size_t, size_t>& pokemon_move_attempts);

    //  Returns true if version 1.0.
    bool start_dialog(VideoStream& stream, ProControllerContext& context);

private:
    IngoOpponentOption OPPONENT;
    BattlePokemonActionTable POKEMON_ACTIONS;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
