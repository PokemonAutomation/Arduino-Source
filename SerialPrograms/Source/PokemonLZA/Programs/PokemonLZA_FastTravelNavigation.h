/*  Fast Travel Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_FastTravelNavigation_H
#define PokemonAutomation_PokemonLZA_FastTravelNavigation_H

#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "PokemonLZA_BasicNavigation.h"
#include "PokemonLZA/Inference/Map/PokemonLZA_MapIconDetector.h"
#include "PokemonLZA/Inference/Map/PokemonLZA_MapDetector.h"
#include "PokemonLZA/Inference/Map/PokemonLZA_LocationNameReader.h"
#include "PokemonLZA/Inference/PokemonLZA_DialogDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_OverworldPartySelectionDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_SelectionArrowDetector.h"
#include "PokemonLZA/Resources/PokemonLZA_Locations.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{



// The arrow box positions for fast travel menu items
const std::vector<ImageFloatBox>& FAST_TRAVEL_ARROW_BOXES();

// The arrow box positions for the fast travel menu filter
const std::vector<ImageFloatBox>& FAST_TRAVEL_FILTER_ARROW_BOXES();

// Get the index of the currently selected item in arrow_boxes using the SelectionArrowDetector
// Return -1 if unable to determine the current selector index
int get_current_selector_index(
    ConsoleHandle& console,
    const std::vector<ImageFloatBox>& arrow_boxes
);

// Determine whether it's better to navigate up or down in the fast travel menu
bool should_navigate_down(
    const LocationItem& current_selection,
    const LocationItem& target_destination
);

// Navigate pages in the fast travel menu via dpad left and right to find the page containing the target destination
// What locations are on a page can depend on filters and if a location is unlocked or not
// Return true when the page should contain the target destination based on indexes
// Return false if no pages contain the target index in their range after checking all pages
// This function does not own the validation of whether the target destination exists or not
bool navigate_to_destination_page_in_fast_travel_menu(
    ConsoleHandle& console,
    ProControllerContext& context,
    Language language,
    LocationItem& target_destination
);

// Get the index of the target destination in the current page's location items
// Return -1 if not found
int get_target_location_index_within_page(
    const LocationItem& target_destination,
    const std::vector<LocationItem>& current_page_locations
);

// Navigate to the target destination assuming we are on the correct page already
// Return true if the target destination is found, hover over the option
// Return false if the target destination is not found
bool navigate_to_destination_within_page(
    ConsoleHandle& console,
    ProControllerContext& context,
    Language language,
    const LocationItem& target_destination
);

// Special handling for lumiouse-sewers-1 and lumiouse-sewers-2 since they have the same OCR result
// Disambiguate using a detector on the background map
// Return true if the target destination is found, hover over the option
// Return false if the target destination is not found
bool navigate_to_lumiose_sewers_location(
    ConsoleHandle& console,
    ProControllerContext& context,
    Language language,
    const LocationItem& target_destination
);

// Navigate to the target destination in the fast travel menu
// Find the correct page first with navigate_to_destination_page_in_fast_travel_menu()
// Then validate whether the target destination exists on the current page
// Return true if the target destination is found on the current page, hover over the option
// Return false if the target destination is not found on the current page
bool navigate_to_destination_in_fast_travel_menu(
    ConsoleHandle& console,
    ProControllerContext& context,
    Language language,
    const LocationItem& target_destination
);

// From the fast travel menu, set the fast travel menu filter to the specified option
void set_fast_travel_menu_filter(
    ConsoleHandle& console,
    ProControllerContext& context,
    FAST_TRAVEL_FILTER filter
);

// From the map screen, open the fast travel menu
void open_fast_travel_menu(
    ConsoleHandle& console,
    ProControllerContext& context
);

// With the fast travel menu opened, read all visible locations in the menu
// Use parallelized OCR to read the 7 items
// Use the dictionary to get the information for each given display name
// Returns a list of LocationItem in the order that they appear
std::vector<LocationItem> read_current_page_location_items(
    ConsoleHandle& console,
    Language language
);

// Open map and fast travel to a specified location.
// This function calls `open_map()`, and uses the fast travel menu to fast travel to the location.
// Assumes the filter is set to show all available fast travel locations.
// Return FastTravelState:
// - SUCCESS: fast travel successful. After the function returns, the player character is on the overworld
// - PURSUED: spotted and pursued by wild pokemon, cannot fast travel. After the function returns, the game
//   is in fly map with the spotted dialog.
// - NOT_FOUND: unable to find the specified location on map, possibly not unlocked yet. After the function
//   returns, the game is in fly map.
FastTravelState open_map_and_fly_to(
    ConsoleHandle& console,
    ProControllerContext& context,
    Language language,
    Location location,
    bool zoom_to_max = false,
    bool clear_filters = false
);
    
}
}
}
#endif