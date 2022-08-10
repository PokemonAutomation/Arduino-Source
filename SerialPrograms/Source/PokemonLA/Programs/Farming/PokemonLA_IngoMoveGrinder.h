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



class IngoMoveGrinder_Descriptor : public SingleSwitchProgramDescriptor{
public:
    IngoMoveGrinder_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class IngoMoveGrinder : public SingleSwitchProgramInstance2{
public:
    IngoMoveGrinder();
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    bool run_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    //  Returns true if version 1.0.
    bool start_dialog(ConsoleHandle& console, BotBaseContext& context);

private:
    void go_to_next_move(SingleSwitchProgramEnvironment& env, BotBaseContext& context);
    void go_to_next_pokemon(SingleSwitchProgramEnvironment& env, BotBaseContext& context);
    size_t get_next_move_to_switch_to() const;
    size_t get_next_pokemon_to_switch_to() const;
    std::string debug_current_info() const;
    std::string debug_move_attempts_info() const;

    EnumDropdownOption OPPONENT;
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
