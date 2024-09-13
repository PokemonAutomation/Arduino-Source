/*  Navigation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Move between different game elements: overworld, save, internet, map, etc.
 *
 */

#ifndef PokemonAutomation_PokemonSV_Navigation_H
#define PokemonAutomation_PokemonSV_Navigation_H

//#include <stdint.h>
#include <string>
#include <functional>
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"

namespace PokemonAutomation{
    struct ProgramInfo;
    class ConsoleHandle;
    class BotBaseContext;
//    class ProgramEnvironment;
//    class EventNotificationOption;
namespace NintendoSwitch{
namespace PokemonSV{


void set_time_to_12am_from_home(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

//  Perform a No-op day skip that rolls over all the outbreaks and raids.
void day_skip_from_overworld(ConsoleHandle& console, BotBaseContext& context);

// Press B to return to the overworld
void press_Bs_to_back_to_overworld(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context, uint16_t seconds_between_b_presses = 3);

//  From overworld, open map. Will change map view from rotated to fixed if not already fixed.
void open_map_from_overworld(
    const ProgramInfo& info,
    ConsoleHandle& console, 
    BotBaseContext& context,
    bool clear_tutorial = false
);

//  From map, press A to fly to a travel spot.
//  check_fly_menuitem == true: will detect if the "Fly" menuitem is available. Return false if no "Fly" menuitem (the game
//    will be on the map menu opened state). Return true if the flight is successful (the game will be at the overworld).
//  check_fly_menuitem == false: will use GradientArrowDetector to check if a map menu is opened. No "Fly" menuitem check.
//    The function always returns true. It throws an error in the case of no "Fly" menuitem. But the error message will be about
//    timeout running the function.
bool fly_to_overworld_from_map(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context, bool check_fly_menuitem = false);

//  Assume the user can set up picnic at current location, start picnic from overworld.
void picnic_from_overworld(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

//  While in picnic, stop picnic and back to overworld.
void leave_picnic(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

//  Enter box system from overworld.
void enter_box_system_from_overworld(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

//  From box system go to overworld.
void leave_box_system_to_overworld(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

//  From overworld, open Pokédex.
void open_pokedex_from_overworld(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

//  From Pokédex, open Recently Battled.
void open_recently_battled_from_pokedex(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

//  From any of the rotom phone apps (Map/Pokédex/Profile) go to overworld.
void leave_phone_to_overworld(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

//  While on map (default zoom), move to the closest PokeCenter and fly there.
//  The PokeCenter must be already visited before (so having the little wing icon with it) and not occluded
//  by other map icons on the most zoomed-in level of the map.
void fly_to_closest_pokecenter_on_map(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

// Attempt to escape being stuck on a wall.
// Repeatedly center camera and try to backwards jump off.
// Finishes when map is successfully open.
void jump_off_wall_until_map_open(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

void reset_to_pokecenter(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

enum class PlayerRealignMode{
    REALIGN_NEW_MARKER,
    REALIGN_OLD_MARKER,
    REALIGN_NO_MARKER,
};

// align player orientation based on the alignment mode
// The direction is specified by (x, y):
// x = 0 : left
// x = 128 : neutral
// x = 255 : right
// y = 0 : up
// y = 128 : neutral
// y = 255 : down
// - REALIGN_NEW_MARKER: place down a map marker, which will align the player towards the marker
// location of the marker is set with move_x, move_y, move_duration
// - REALIGN_OLD_MARKER: assuming a marker is already set, open and close the map, 
// which will align the player towards the marker
// - REALIGN_NO_MARKER: move player towards in the direction set by move_x, move_y, move_duration
// then re-align the camera
void realign_player(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    PlayerRealignMode realign_mode,
    uint8_t move_x = 0, uint8_t move_y = 0, uint8_t move_duration = 0
);

enum class NavigationStopCondition{
    STOP_DIALOG,
    STOP_MARKER,
};

enum class NavigationMovementMode{
    DIRECTIONAL_ONLY,
    DIRECTIONAL_SPAM_A,
    CLEAR_WITH_LETS_GO,
};


void walk_forward_until_dialog(
    const ProgramInfo& info, 
    ConsoleHandle& console, 
    BotBaseContext& context,
    NavigationMovementMode movement_mode,
    uint16_t seconds_timeout = 10,
    uint8_t y = 0
);

// walk forward while using lets go to clear the path
// forward_ticks: number of ticks to walk forward
// y = 0: walks forward. y = 128: stand in place. y = 255: walk backwards (towards camera)
// ticks_between_lets_go: number of ticks between firing off Let's go to clear the path from wild pokemon
// delay_after_lets_go: number of ticks to wait after firing off Let's go.
void walk_forward_while_clear_front_path(
    const ProgramInfo& info, 
    ConsoleHandle& console, 
    BotBaseContext& context,
    uint16_t forward_ticks,
    uint8_t y = 0,
    uint16_t ticks_between_lets_go = 125,
    uint16_t delay_after_lets_go = 250
);

// mashes A button by default
void mash_button_till_overworld(
    ConsoleHandle& console, 
    BotBaseContext& context, 
    uint16_t button = BUTTON_A, uint16_t seconds_run = 360
);

// fly to the pokecenter that overlaps with the player on the map, and return true.
// if no overlapping pokecenter, return false.
bool attempt_fly_to_overlapping_flypoint(
    const ProgramInfo& info, 
    ConsoleHandle& console, 
    BotBaseContext& context
);

// fly to the pokecenter that overlaps with the player on the map
// throw exception if unsuccessful
void fly_to_overlapping_flypoint(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

// throw exception if there is a fly point/pokecenter that overlaps with the player on the map
void confirm_no_overlapping_flypoint(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

// enter menu and move the cursor the given side, and index. then press the A button
// if menu_index is -1, return once the menu is detected.
void enter_menu_from_overworld(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    int menu_index,
    MenuSide side = MenuSide::RIGHT,
    bool has_minimap = true
);

// press given button until gradient arrow appears in given box_area_to_check.
void press_button_until_gradient_arrow(
    const ProgramInfo& info, 
    ConsoleHandle& console, 
    BotBaseContext& context,
    ImageFloatBox box_area_to_check,
    uint16_t button = BUTTON_A,
    GradientArrowType arrow_type = GradientArrowType::RIGHT
);

// navigate menus using only gradient arrow detection, without OCR
// first, wait for the gradient arrow to appear within `arrow_box_start`.
// then, press `dpad_button` a certain number of times, as per `num_button_presses`.
// then, watch for the gradient arrow within `arrow_box_end`. 
// If arrow not seen, press `dpad_button` a maximum of 3 times, while still watching for the gradient arrow
// If arrow still not seen, throw exception.
void basic_menu_navigation(
    const ProgramInfo& info, 
    ConsoleHandle& console, 
    BotBaseContext& context,
    ImageFloatBox arrow_box_start,
    ImageFloatBox arrow_box_end,
    uint8_t dpad_button,
    uint16_t num_button_presses
);

// heal at the pokecenter, that your character is currently at.
// if not currently at the pokecenter, throws error.
void heal_at_pokecenter(
    const ProgramInfo& info, 
    ConsoleHandle& console, 
    BotBaseContext& context
);


}
}
}
#endif
