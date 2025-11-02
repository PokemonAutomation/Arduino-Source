/*  Trainer Battle Functions
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_TrainerBattle_H
#define PokemonAutomation_PokemonLZA_TrainerBattle_H

namespace PokemonAutomation{

template <typename Type> class ControllerContext;

namespace NintendoSwitch{

class SingleSwitchProgramEnvironment;
class ProController;
using ProControllerContext = ControllerContext<ProController>;

namespace PokemonLZA{


// Try to lock onto an opponent pokemon and launch an attack.
// - use_move_ai: if True, select move based on type effectiveness. Otherwise, select the top,
//   left then bottom move in quick succession to ensure at least one of the three moves are launched
//   despite potential move cooldown.
// - use_plus_moves: if True, press + to launch plus moves.
bool attempt_one_attack(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    bool use_move_ai, bool use_plus_moves
);

}
}
}
#endif
