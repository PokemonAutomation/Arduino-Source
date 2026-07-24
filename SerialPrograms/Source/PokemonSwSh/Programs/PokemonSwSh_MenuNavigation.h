/*  Menu Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MenuNavigation_H
#define PokemonAutomation_PokemonSwSh_MenuNavigation_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
    class EventNotificationOption;
    class ProgramEnvironment;
namespace NintendoSwitch{
namespace PokemonSwSh{


// We assume Pokemon app is always at row 0, col 1
constexpr size_t POKEMON_APP_INDEX = 1;
// We assume Town Map app is always at row 1, col 0
constexpr size_t TOWN_MAP_APP_INDEX = 5;




// When Rotom Phone menu is opened, move the cursor to the target app.
// The target app index is from 0 to 9, in the order of top to bottom, left to right.
// e.g. by default, Pokemon app is at index 1, while Town Map app is at index 5.
// The function detects the current cursor location. So the function works on any initial cursor location.
// Will OperationFailedException::fire when failed to detect menu
void navigate_to_menu_app(
    VideoStream& stream,
    ProControllerContext& context,
    size_t app_index
);

// Move to Main Menu, starting from Overworld, Party menu, or Box menu
void menus_to_mainmenu(VideoStream& stream, ProControllerContext& context);
void menus_to_boxsystem(VideoStream& stream, ProControllerContext& context);


void save_game(VideoStream& stream, ProControllerContext& context);

// Used to wait until Y-Comm icon shows up.
// Throw error if it does not find it after 10 sec.
void mash_B_until_y_comm_icon(
    VideoStream& stream,
    ProControllerContext& context,
    const std::string& error_msg
);



}
}
}
#endif
