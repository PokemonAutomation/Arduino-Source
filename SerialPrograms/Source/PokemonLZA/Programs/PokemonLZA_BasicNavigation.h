/*  Basic Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_BasicNavigation_H
#define PokemonAutomation_PokemonLZA_BasicNavigation_H

#include "Common/Cpp/Time.h"
#include "PokemonLZA/Programs/PokemonLZA_Locations.h"

namespace PokemonAutomation{

template <typename Type> class ControllerContext;
struct ImageFloatBox;

namespace NintendoSwitch{

class ConsoleHandle;
class ProController;
using ProControllerContext = ControllerContext<ProController>;

namespace PokemonLZA{


//  Starting from either the overworld or the main menu, save the game.
//  This function returns in the main menu.
//  Return true if the game is saved successfully, false otherwise.
bool save_game_to_menu(ConsoleHandle& console, ProControllerContext& context);



// retrun type of the function fly_from_map()
enum class FastTravelState{
    SUCCESS,      // Successfully did a fast travel
    PURSUED,      // Being spotted and pursued by wild pokemon
    NOT_AT_FLY_SPOT  // the current map cursor is not on a fly spot
    // Future work: in some main story session the fast travel is disabled. We will implement
    // that state if we need
};

// Press button + to open the normal Lumiose map.
// Will repeatedly pressing button + to ensure the map is opened.
// Robust against day/night changes: if there is a day/night chane before opening the map,
// it will keep trying to open the map until day/night change finishes.
// zoom_to_max: whether to go to max zoom level after opening the map
// Return True if you are not chased by wild pokemon and can fast travel, False otherwise.
// Note the function uses flyable fast travel icons on map to detect if you are being chased. This
// means for most reliable detection, set zoom_to_max to True.
bool open_map(ConsoleHandle& console, ProControllerContext& context, bool zoom_to_max = false);

// Press button + to open Hyperspace Lumiose map.
// Will repeatedly pressing button + to ensure the map is opened.
// It does not detect whether the player can fast travel or not.
void open_hyperspace_map(ConsoleHandle& console, ProControllerContext& context);

// Starting at map view, press A to fast travel to the current selected fast travel location
// overworld_screen: if not nullptr, return the overworld screen when the fly is over.
//   This is useful for doing additional visual detection on the overworld screen. If fast travel
//   failed, no change will be made to `overworld_screen`.
// Return FastTravelState:
// - SUCCESS: fast travel successful. After the function returns, the player character is on the overworld
// - PURSUED: spotted and pursued by wild pokemon, cannot fast travel. After the function returns, the game
//   is in fly map with the spotted dialog.
// - NOT_AT_FLY_SPOT: the current map cursor is not on a fly spot, cannot fast travel. After the function
//   returns, the game is in fly map.
FastTravelState fly_from_map(ConsoleHandle& console, ProControllerContext& context,
    std::shared_ptr<const ImageRGB32>* overworld_screen = nullptr);

// Fast travel without moving map cursor.
// This is useful to fast travel back to the wild zone gate while in the zone.
// This function basically just calls `open_map()` and `fly_from_map()`. See the comments of those two
// functions for details.
FastTravelState open_map_and_fly_in_place(ConsoleHandle& console, ProControllerContext& context, bool zoom_to_max = false);

// Blind movement of map cursor from zone entrance to that zone fast travel icon on map
// this blind movement only works on max zoom level (fully zoomed out)!
void move_map_cursor_from_entrance_to_zone(ConsoleHandle& console, ProControllerContext& context, WildZone zone);

// Mash button B to leave map view and back to overworld
// If there is a day/night change, this function will wait for day/night change to finish.
void map_to_overworld(ConsoleHandle& console, ProControllerContext& context);


// Assuming the player character is facing the bench with buton A available, this function
// presses button A repeated to initiate day/night change and returns when the game finishes
// day/night change transition animation and is back to overworld.
void sit_on_bench(ConsoleHandle& console, ProControllerContext& context);

// default wait time is long enough to wait for day/night change to end
void wait_until_overworld(
    ConsoleHandle& console, ProControllerContext& context,
    std::chrono::milliseconds max_wait_time = std::chrono::seconds(40)
);

// Read the minimap to get the direction the blue directional arrow is facing. The arrow
// represents which direction the player character is facing.
// If day/night change happens to appear during reading the minimap, it will wait until
// the day/night hange finishes.
// Return a float of [0.0, 360.0), the angle of the arrow using the clock setting, i.e.
// - Pointing upwards is 0.0 degree.
// - Pointing to the right is 90.0 degrees.
double get_facing_direction(ConsoleHandle& console, ProControllerContext& context);

// Given two facing directions, find the angle between them. The angle range is [0, 180).
double get_angle_between_facing_directions(double dir1, double dir2);

// While at the gate in the zone, mash A to leave the zone. If day/night changes while
// leaving, it will wait until the change is done.
// Return true if there is no day/night change.
// Return false if day/night change happens. In this case, we don't know if the player
// character is still inside the zone or not.
// Note:
// We don't want to mash A after a day/night change to ensure we leave the zone because
// at Wild Zone 4 there are talkable npcs outside the gate. Button A appears on them.
// If the program mashes A after leaving the zone, it will stuck talking to the npcs.
bool leave_zone_gate(ConsoleHandle& console, ProControllerContext& context);

// Run towards a gate until either button A is detected at specified box region,
// or day/night change happens. If day/night changes, it will wait until the transition
// animation is done.
// Return
// -  0 if button A is detected
// -  1 if day/night change happens
// - -1 if it does not reach the gate in the end. Possible reasons are wrong run direction
//   or get stuck by terrain or obstacle on the way
int run_towards_gate_with_A_button(
    ConsoleHandle& console, ProControllerContext& context,
    double run_direction_x, double run_direction_y,
    Milliseconds run_time
);

// Run a straight line in the overworld, for *duration* long or until a day/night change happens.
// If day/night changes, it will wait until the transition animation is done.
// Return
// -  0 if a day/night change detected
// - -1 it ran for full *duration* without a day/night change
int run_a_straight_path_in_overworld(
    ConsoleHandle& console, ProControllerContext& context,
    double direction_x, double direction_y,
    PokemonAutomation::Milliseconds duration
);

}
}
}
#endif
