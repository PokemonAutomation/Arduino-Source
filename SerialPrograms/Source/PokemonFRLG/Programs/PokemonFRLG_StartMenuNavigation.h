/*  Start Menu Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_StartMenuNavigation_H
#define PokemonAutomation_PokemonFRLG_StartMenuNavigation_H

#include "PokemonFRLG/Inference/PokemonFRLG_SelectionArrowDetector.h"

namespace PokemonAutomation{

class ImageRGB32;

template <typename Type> class ControllerContext;
struct ImageFloatBox;

namespace NintendoSwitch{

class ConsoleHandle;
class ProController;
using ProControllerContext = ControllerContext<ProController>;

namespace PokemonFRLG {

// Box that contains all the possible start menu selection arrow positions
const ImageFloatBox MENU_ARROW_BOX(0.727692, 0.0523077, 0.0369231, 0.6438461);

// Open the start menu from the overworld. Throws OperationFailedException if it fails to open the menu after several attempts.
void open_start_menu(ConsoleHandle& console, ProControllerContext& context);

// Close the start menu to return to the overworld
void close_start_menu(ConsoleHandle& console, ProControllerContext& context);

// Starting from the start menu, move the selection arrow to the specified position.
// Return true if successful, false otherwise (e.g. if selection arrow is not detected).
bool move_cursor_to_position(
    ConsoleHandle& console, ProControllerContext& context, 
    SelectionArrowPositionStartMenu destination
);

// Starting from the Safari Zone start menu, move the selection arrow to the specified position.
// Return true if successful, false otherwise (e.g. if selection arrow is not detected).
bool move_cursor_to_position(
    ConsoleHandle& console, ProControllerContext& context,
    SelectionArrowPositionSafariMenu destination
);

//  Starting from either the overworld or the main menu, save the game.
//  This function returns in the overworld.
void save_game_to_overworld(ConsoleHandle& console, ProControllerContext& context);

}
}
}

#endif
