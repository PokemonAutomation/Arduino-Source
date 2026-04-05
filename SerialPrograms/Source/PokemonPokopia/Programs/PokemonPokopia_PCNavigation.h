/*  PC Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_PCNavigation_H
#define PokemonAutomation_PokemonLZA_PCNavigation_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "PokemonPokopia/Inference/PokemonPokopia_MovesDetection.h"
#include "PokemonPokopia/Inference/PokemonPokopia_PCDetection.h"
#include "PokemonPokopia/Inference/PokemonPokopia_SelectionArrowDetector.h"

namespace PokemonAutomation{

template <typename Type> class ControllerContext;

namespace NintendoSwitch{

class ConsoleHandle;
class ProController;
using ProControllerContext = ControllerContext<ProController>;

namespace PokemonPokopia{

// Locations of the PC menu option boxes
// Challenges and Shop are in the top row and do not move
// The rest are on the bottom row and change positions depending on whether the player is in Palette Town or not
const ImageFloatBox PC_CHALLENGES_BOX{0.312500, 0.205000, 0.034000, 0.052500};
const ImageFloatBox PC_SHOP_BOX{0.650500, 0.205000, 0.034000, 0.052500};
// Non-Palette Town
const ImageFloatBox PC_LINK_PLAY_BOX{0.256000, 0.648500, 0.034000, 0.052500};
const ImageFloatBox PC_GET_ITEMS_BOX{0.482500, 0.648500, 0.034000, 0.052500};
const ImageFloatBox PC_STAMP_CARD_BOX{0.709000, 0.648500, 0.034000, 0.052500};
// Palette Town
const ImageFloatBox PC_PALETTE_LINK_PLAY_BOX{0.162000, 0.648500, 0.034000, 0.052500};
const ImageFloatBox PC_PALETTE_GET_ITEMS_BOX{0.376000, 0.648500, 0.034000, 0.052500};
const ImageFloatBox PC_PALETTE_STAMP_CARD_BOX{0.590000, 0.648500, 0.034000, 0.052500};
const ImageFloatBox PC_PALETTE_APPRAISE_BOX{0.802500, 0.648500, 0.034000, 0.052500};

// Shop Menu items. Apply the offset to get boxes for items 2-5
const ImageFloatBox SHOP_DAILY_ITEM_TYPE_BOX_1{0.125000, 0.300000, 0.034000, 0.064000};
const ImageFloatBox SHOP_DAILY_ITEM_SELECTOR_BOX_1{0.165000, 0.170000, 0.034000, 0.052500}; // Issue when detecting orange arrow on orange label in shop menu
const ImageFloatBox SHOP_DAILY_ITEM_COIN_ICON_BOX_1(0.133000, 0.377000, 0.025000, 0.045000);
const std::pair<double, double> SHOP_DAILY_ITEM_OFFSET(0.158000, 0);

// Link Play menu options
const ImageFloatBox LINK_PLAY_INVITE_BOX{0.048500, 0.249000, 0.027500, 0.057500};
const ImageFloatBox LINK_PLAY_VISIT_BOX{0.048500, 0.443000, 0.027500, 0.057500};
const ImageFloatBox LINK_PLAY_PLAY_BOX{0.048500, 0.637000, 0.027500, 0.057500};
const ImageFloatBox LINK_PLAY_SETTINGS_BOX{0.048500, 0.786000, 0.027500, 0.057500};
const std::vector<ImageFloatBox> LINK_PLAY_MENU{LINK_PLAY_INVITE_BOX, LINK_PLAY_VISIT_BOX, LINK_PLAY_PLAY_BOX, LINK_PLAY_SETTINGS_BOX};

// Cloud Island menu options
const ImageFloatBox CLOUD_ISLAND_GO_TO_BOX{0.048500, 0.249000, 0.027500, 0.057500};
const ImageFloatBox CLOUD_ISLAND_SEARCH_BOX{0.048500, 0.443000, 0.027500, 0.057500};
const ImageFloatBox CLOUD_ISLAND_CREATE_BOX{0.048500, 0.637000, 0.027500, 0.057500};
const std::vector<ImageFloatBox> CLOUD_ISLAND_MENU{CLOUD_ISLAND_GO_TO_BOX, CLOUD_ISLAND_SEARCH_BOX, CLOUD_ISLAND_CREATE_BOX};

// The top box of a generic PC menu item with unit height. Each subsequent item is exactly .1 Y units below
const ImageFloatBox PC_GENERIC_TOP_BOX(0.048500, 0.200000, 0.027500, 0.057500);

// Yes and No locations for standard confirmation prompts
const ImageFloatBox CONFIRM_YES_BOX{0.657000, 0.592000, 0.027500, 0.057500};
const ImageFloatBox CONFIRM_NO_BOX{0.657000, 0.677000, 0.027500, 0.057500};
const std::vector<ImageFloatBox> CONFIRM_YES_NO{CONFIRM_YES_BOX, CONFIRM_NO_BOX};

const ImageFloatBox STAMP_CARD_BOX{0.045000, 0.040000, 0.600000, 0.750000};
const ImageFloatBox ADD_STAMP_BOX{0.705000, 0.790000, 0.027500, 0.057500};
const std::pair<double, double> SELECTION_BOX_TO_STAMP_BOX_OFFSET(-0.071, .033);
// The stamp locations actually move week to week, get the locations dynamically
// const ImageFloatBox STAMP1_SELECT_BOX{0.124000, 0.120000, 0.034000, 0.052500};
// const ImageFloatBox STAMP2_SELECT_BOX{0.176000, 0.525500, 0.034000, 0.052500};
// const ImageFloatBox STAMP3_SELECT_BOX{0.323500, 0.248500, 0.034000, 0.052500};
// const ImageFloatBox STAMP4_SELECT_BOX{0.463000, 0.527500, 0.034000, 0.052500};
// const ImageFloatBox STAMP5_SELECT_BOX{0.562000, 0.163500, 0.034000, 0.052500};
// const std::vector<ImageFloatBox> STAMP_SELECT_BOXES{STAMP1_SELECT_BOX, STAMP2_SELECT_BOX, STAMP3_SELECT_BOX, STAMP4_SELECT_BOX, STAMP5_SELECT_BOX};

enum class PCOpenStatus{
    MAIN,
    STAMP_CARD,
    ENVIRONMENT_CHANGE,
    FAILURE
};

enum class PCMenuOption{
    CHALLENGES,
    SHOP,
    LINK_PLAY,
    GET_ITEMS,
    STAMP_CARD,
    APPRAISE // Only present in Palette Town
};

// Wait until the overworld is fully loaded
void wait_for_overworld(ConsoleHandle& console, ProControllerContext& context);

// Get locations of generic PC menu options for menus with "option_count" number of equal and minimal height boxes
std::vector<ImageFloatBox> get_generic_options_boxes(size_t option_count);

// From the overworld in front of a PC, attempt to access the PC
void access_pc_from_overworld(ConsoleHandle& console, ProControllerContext& context, bool stop_on_stamp_card=false);

// Close out of all PC menus and return to overworld
void exit_pc(ConsoleHandle& console, ProControllerContext& context);

// With the PC menu open and actionable, open the specified menu option
void open_menu_option(ConsoleHandle& console, ProControllerContext& context, PCMenuOption option);

// A generic function to open and select an option for a vertical or horizontal list
// Takes in a vector of ImageBoxes for the selection arrow locations for each item
// Specify the index of the target option
// Specify whether the selection arrow is right or down which determines if it is a vertical or horizontal list
// If the movement in the list matches the arrow direction, set move_parallel. False for most cases
void generic_select_and_open(
    ConsoleHandle& console, 
    ProControllerContext& context, 
    const std::vector<ImageFloatBox>& option_boxes,
    int target_index,
    SelectionArrowType arrow_type,
    bool move_parallel = false
);

// A wrapper for generic_select_and_open() for the common yes/no confirmation prompt with the locations pre-populated
void yes_no_select_and_confirm(
    ConsoleHandle& console, 
    ProControllerContext& context, 
    bool select_yes
);

// Keep pressing A to progress though dialog until a prompt appears
void continue_until_prompt(
    ConsoleHandle& console, 
    ProControllerContext& context, 
    const ImageFloatBox prompt_box
);

// Check if the daily shop item at the specified index is a recipe
// Return single, double, triple, or not a recipe
RecipeType item_is_recipe(
    ConsoleHandle& console,
    ProControllerContext& context,
    int item_index
);

// Check if the daily shop item at the specified index is available for purchase
bool item_is_available(
    ConsoleHandle& console,
    ProControllerContext& context,
    int item_index
);

// Buy the daily shop item at the specified index
// TODO: Handle the case where you cannot afford the item. This is usually gated elsewhere to ensure this case is never reached
void buy_item(
    ConsoleHandle& console,
    ProControllerContext& context,
    int item_index
);

// Get the stamp in the specified area
Stamp get_stamp(
    ConsoleHandle& console, 
    ProControllerContext& context, 
    ImageFloatBox stamp_box
);

// Get the stamp based on the location of the selection arrow for the corresponding stamp slot
Stamp get_stamp_from_selection(
    ConsoleHandle& console, 
    ProControllerContext& context, 
    ImageFloatBox selection_arrow_box
);

// Add today's stamp to the stamp collection
// Return true if the stamp was added, false if a replacement is needed
bool add_stamp(
    ConsoleHandle& console,
    ProControllerContext& context,
    SelectionArrowWatcher& replace_stamp_selection_watcher
);

// Replace the specified stamp slot with today's stamp by pressing "Add Stamp!" and navigating to the target
// Stamp indicies follow the path from left to right
void replace_stamp(
    ConsoleHandle& console, 
    ProControllerContext& context, 
    SelectionArrowWatcher& replace_stamp_selection_watcher
);

// navigate to the next stamp
void move_to_next_stamp(
    ConsoleHandle& console, 
    ProControllerContext& context, 
    SelectionArrowWatcher& replace_stamp_selection_watcher
);



}
}
}
#endif
