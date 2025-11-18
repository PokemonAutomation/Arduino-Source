/*  Basic Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_BasicNavigation_H
#define PokemonAutomation_PokemonLZA_BasicNavigation_H

#include "PokemonLZA/Programs/PokemonLZA_Locations.h"

namespace PokemonAutomation{

template <typename Type> class ControllerContext;

namespace NintendoSwitch{

class ConsoleHandle;
class ProController;
using ProControllerContext = ControllerContext<ProController>;

namespace PokemonLZA{


//  Starting from either the overworld or the main menu, save the game.
//  This function returns in the main menu.
void save_game_to_menu(ConsoleHandle& console, ProControllerContext& context);



// retrun type of the function fly_from_map()
enum class FastTravelState{
    SUCCESS,      // Successfully did a fast travel
    PURSUED,      // Being spotted and pursued by wild pokemon
    NOT_AT_FLY_SPOT  // the current map cursor is not on a fly spot
    // Future work: in some main story session the fast travel is disabled. We will implement
    // that state if we need
};

// Press button + to open the map.
// Will repeatedly pressing button + to ensure the map is opened.
// Robust against day/night changes: if there is a day/night chane before opening the map,
// it will keep trying to open the map until day/night change finishes.
// zoom_to_max: whether to go to max zoom level after opening the map
// Return True if you are not chased by wild pokemon and can fast travel, False otherwise.
// Note the function uses flyable fast travel icons on map to detect if you are being chased. This
// means for most reliable detection, set zoom_to_max to True.
bool open_map(ConsoleHandle& console, ProControllerContext& context, bool zoom_to_max = false);
// Starting at map view, press A to fast travel to the current selected fast travel location
// Return FastTravelState:
// - SUCCESS: fast travel successful. After the function returns, the player character is on the overworld
// - PURSUED: spotted and pursued by wild pokemon, cannot fast travel. After the function returns, the game
//   is in fly map with the spotted dialog.
// - NOT_AT_FLY_SPOT: the current map cursor is not on a fly spot, cannot fast travel. After the function
//   returns, the game is in fly map.
FastTravelState fly_from_map(ConsoleHandle& console, ProControllerContext& context);

// Blind movement of map cursor from zone entrance to that zone fast travel icon on map
// this blind movement only works on max zoom level (fully zoomed out)!
void move_map_cursor_from_entrance_to_zone(ConsoleHandle& console, ProControllerContext& context, WildZone zone);


// Assuming the player character is facing the bench with buton A available, this function
// presses button A repeated to initiate day/night change and returns when the game finishes
// day/night change transition animation and is back to overworld.
void sit_on_bench(ConsoleHandle& console, ProControllerContext& context);

// default wait time is long enough to wait for day/night change to end
void wait_until_overworld(
    ConsoleHandle& console, ProControllerContext& context,
    std::chrono::milliseconds max_wait_time = std::chrono::seconds(40)
);


}
}
}
#endif
