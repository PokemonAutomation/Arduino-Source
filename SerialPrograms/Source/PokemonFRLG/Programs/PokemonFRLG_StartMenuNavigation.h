/*  Start Menu Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_StartMenuNavigation_H
#define PokemonAutomation_PokemonFRLG_StartMenuNavigation_H

namespace PokemonAutomation{

class ImageRGB32;

template <typename Type> class ControllerContext;
struct ImageFloatBox;

namespace NintendoSwitch{

class ConsoleHandle;
class ProController;
using ProControllerContext = ControllerContext<ProController>;

namespace PokemonFRLG {

//  Starting from either the overworld or the main menu, save the game.
//  This function returns in the overworld.
void save_game_to_overworld(ConsoleHandle& console, ProControllerContext& context);

}
}
}

#endif