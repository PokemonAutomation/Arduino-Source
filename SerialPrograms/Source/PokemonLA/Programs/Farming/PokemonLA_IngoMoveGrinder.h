/*  Ingo Move Grinder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_IngoMoveGrinder_H
#define PokemonAutomation_PokemonLA_IngoMoveGrinder_H

#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_BattlePokemonActionTable.h"
#include "PokemonLA_IngoBattleGrinder.h"

#include <array>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



class IngoMoveGrinder_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    IngoMoveGrinder_Descriptor();
};

class IngoMoveGrinder : public SingleSwitchProgramInstance{
public:
    IngoMoveGrinder(const IngoMoveGrinder_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    bool run_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    //  Returns true if version 1.0.
    bool start_dialog(ConsoleHandle& console, BotBaseContext& context);

private:
    void go_to_next_move(SingleSwitchProgramEnvironment& env, BotBaseContext& context);
    void go_to_next_pokemon(SingleSwitchProgramEnvironment& env, BotBaseContext& context);
    size_t get_next_pokemon_to_switch_to() const;
    QString debug_current_info() const;
    QString debug_move_attempts_info() const;
    class Stats;

    EnumDropdownOption OPPONENT;
    MoveGrinderActionTable POKEMON_ACTIONS;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;

    size_t cur_pokemon = 0;
    size_t cur_move = 0;
    size_t next_pokemon_to_switch_to = 1;
    std::array<std::array<uint16_t, 4>, 5> move_issued{};
};





}
}
}
#endif
