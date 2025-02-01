/*  Ingo Move Grinder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_IngoMoveGrinder_H
#define PokemonAutomation_PokemonLA_IngoMoveGrinder_H

#include <array>
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_IngoOpponent.h"
#include "PokemonLA/Options/PokemonLA_BattlePokemonActionTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



class IngoMoveGrinder_Descriptor : public SingleSwitchProgramDescriptor{
public:
    IngoMoveGrinder_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class IngoMoveGrinder : public SingleSwitchProgramInstance{
public:
    IngoMoveGrinder();
    virtual void program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context) override;

private:
    bool run_iteration(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context);

    //  Returns true if version 1.0.
    bool start_dialog(VideoStream& stream, SwitchControllerContext& context);

private:
    void go_to_next_move(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context);
    void go_to_next_pokemon(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context);
    size_t get_next_move_to_switch_to() const;
    size_t get_next_pokemon_to_switch_to() const;
    std::string debug_current_info() const;
    std::string debug_move_attempts_info() const;

    IngoOpponentOption OPPONENT;
    MoveGrinderActionTable POKEMON_ACTIONS;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;

    size_t cur_pokemon = 0;
    size_t cur_move = 0;
    std::array<std::array<uint16_t, 4>, 5> move_issued{};
};





}
}
}
#endif
