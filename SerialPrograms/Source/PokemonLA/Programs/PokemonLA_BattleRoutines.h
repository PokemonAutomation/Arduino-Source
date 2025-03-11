/*  Battle Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_BattleRoutines_H
#define PokemonAutomation_PokemonLA_BattleRoutines_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "PokemonLA/Options/PokemonLA_BattlePokemonActionTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


void mash_A_until_end_of_battle(VideoStream& stream, ProControllerContext& context);

// Assuming the game is in the switching pokemon screen, select a pokemon to send to battle.
// pokemon_to_switch_to: the index of the pokemon in the party to switch to.
// If the pokemon cannot be sent either due to already on the battlefield or fainted, it will select the next
// pokemon in the party list, until it is successfully sent.
// Return the index of the party that is actually sent to battle.
// max_num_pokemon: if the pokemon index to switch to is >= `max_num_pokemon`, throw an OperationFailedException.
size_t switch_pokemon(
    VideoStream& stream, ProControllerContext& context,
    size_t pokemon_to_switch_to,
    size_t max_num_pokemon = SIZE_MAX
);

// Assuming the game is at the move selection menu, use the current selected move in desired style in battle.
// `cur_pokemon` does not affect game control. It is passed in only for the logging purpose.
// `cur_move` the index of the move (0, 1, 2 or 3). It is only used when `check_success` is true.
// If `check_success` is true, return whether the move is successfully used, by checking whether the game is still in the move selection
// menu after some ticks (need correct `cur_move` to do this). If `check_success` is false, always return true.
// `wait_for_all_requests()` is called at the end of the function no matter `check_success` is true or false.
bool use_move(
    VideoStream& stream, ProControllerContext& context,
    size_t cur_pokemon,
    size_t cur_move,
    MoveStyle style,
    bool check_success
);

// Assuming the game is at the move selection menu, use the current selected move with no style.
// If the move has no PP, move to use the next move.
// `cur_pokemon` does not affect game control. It is passed in only for the logging purpose.
// In order for the move PP detection to be correct, must pass in the correct `cur_move`.
// When the function moves to using the next move, it also updates `cur_move`.
// This function assumes it starts with the move selection menu, so it does not consider the case the pokemon has no PP and has to use Struggle.
void use_next_move_with_pp(
    VideoStream& stream, ProControllerContext& context,
    size_t cur_pokemon,
    size_t& cur_move
);


}
}
}
#endif
