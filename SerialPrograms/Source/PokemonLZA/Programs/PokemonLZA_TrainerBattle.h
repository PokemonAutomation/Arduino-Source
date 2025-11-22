/*  Trainer Battle Functions
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_TrainerBattle_H
#define PokemonAutomation_PokemonLZA_TrainerBattle_H

//#include "Common/Cpp/Time.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{

template <typename Type> class ControllerContext;
class ProgramEnvironment;

namespace NintendoSwitch{

class ProController;
using ProControllerContext = ControllerContext<ProController>;

namespace PokemonLZA{




class TrainerBattleState{
public:
    TrainerBattleState();

    // Try to lock onto an opponent pokemon and launch an attack.
    // - use_move_ai: if True, select move based on type effectiveness. Otherwise, select the top, left
    //   then bottom (if allow_button_B_press) move in quick succession to ensure at least one of the moves
    //   are launched despite potential move cooldown.
    // - use_plus_moves: if True, press + to launch plus moves.
    // - allow_button_B_press: if true, allow the AI mode to press B if it cannot detect move effectiveness
    //   symbols and allow the non-AI mode to use the bottom move. This lets the function clear any pre-battle
    //   transparent dialog window it encounters.
    //   If false, the AI mode will do nothing when move effectiveness symbols are not detected and the
    //   non-AI mode will never select the bottom move. This ensures the function won't interfere with
    //   detecting flat white dialog windows but it will be external code's responsibility to clear
    //   pre-battle transparent dialog windows.
    bool attempt_one_attack(
        ProgramEnvironment& env,
        ConsoleHandle& console,
        ProControllerContext& context,
        bool use_move_ai,
        bool use_plus_moves,
        bool allow_button_B_press = true
    );


private:
    void run_lock_recovery(ConsoleHandle& console, ProControllerContext& context);


private:
    size_t m_consecutive_failures;
};




}
}
}
#endif
