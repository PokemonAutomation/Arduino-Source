/*  Basic Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_BasicNavigation_H
#define PokemonAutomation_PokemonLZA_BasicNavigation_H

namespace PokemonAutomation{

template <typename Type> class ControllerContext;

namespace NintendoSwitch{

class ConsoleHandle;
class ProController;
using ProControllerContext = ControllerContext<ProController>;

namespace PokemonLZA{


// retrun type of the function fly_from_map()
enum class FastTravelState{
    SUCCESS,      // Successfully did a fast travel
    PURSUED,      // Being spotted and pursued by wild pokemon
    NON_FLY_SPOT  // the current map cursor is not on a fly spot
    // Future work: in some main story session the fast travel is disabled. We will implement
    // that state if we need
};

// Press button + to open the map.
// Will repeatedly pressing button + to ensure the map is opened.
// Robust against day/night changes: if there is a day/night chane before opening the map,
// it will keep trying to open the map until day/night change finishes.
void open_map(ConsoleHandle& console, ProControllerContext& context);
// Starting at map view, press A to fast travel to the current selected fast travel location
// Return FastTravelState:
// - SUCCESS: fast travel successful. After the function returns, the player character is on the overworld
// - PURSUED: spotted and pursued by wild pokemon, cannot fast travel. After the function returns, the game
//   is in fly map with the spotted dialog.
// - NON_FLY_SPOT: the current map cursor is not on a fly spot, cannot fast travel. After the function
//   returns, the game is in fly map.
FastTravelState fly_from_map(ConsoleHandle& console, ProControllerContext& context);

void sit_on_bench(ConsoleHandle& console, ProControllerContext& context);



}
}
}
#endif
