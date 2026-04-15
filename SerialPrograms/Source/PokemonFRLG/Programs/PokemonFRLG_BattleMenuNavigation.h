/*  Battle Menu Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_BattleMenuNavigation_H
#define PokemonAutomation_PokemonFRLG_BattleMenuNavigation_H

#include "PokemonFRLG/Inference/PokemonFRLG_BattleSelectionArrowDetector.h"

namespace PokemonAutomation{

template <typename Type> class ControllerContext;

namespace NintendoSwitch{

class ConsoleHandle;
class ProController;
using ProControllerContext = ControllerContext<ProController>;

namespace PokemonFRLG{


// Starting from the normal battle command menu, move the selection arrow to the specified option.
// Returns true if successful, false otherwise (e.g. if selection arrow is not detected).
bool move_cursor_to_option(
    ConsoleHandle& console, ProControllerContext& context,
    BattleMenuOption destination
);

// Starting from the Safari Zone battle command menu, move the selection arrow to the specified option.
// Returns true if successful, false otherwise (e.g. if selection arrow is not detected).
bool move_cursor_to_option(
    ConsoleHandle& console, ProControllerContext& context,
    SafariBattleMenuOption destination
);


}
}
}
#endif
