/*  PokemonSV World Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Move player character around the overworld
 *
 */

#ifndef PokemonAutomation_PokemonSV_WorldNavigation_H
#define PokemonAutomation_PokemonSV_WorldNavigation_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Programs/AutoStory/PokemonSV_AutoStoryTools.h"

namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{


//  From map, press A to fly to a travel spot.
//  check_fly_menuitem == true: will detect if the "Fly" menuitem is available. Return false if no "Fly" menuitem (the game
//    will be on the map menu opened state). Return true if the flight is successful (the game will be at the overworld).
//  check_fly_menuitem == false: will use GradientArrowDetector to check if a map menu is opened. No "Fly" menuitem check.
//    The function always returns true. It throws an error in the case of no "Fly" menuitem. But the error message will be about
//    timeout running the function.
bool fly_to_overworld_from_map(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context, bool check_fly_menuitem = false);

//  Assume the user can set up picnic at current location, start picnic from overworld.
void picnic_from_overworld(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

//  While in picnic, stop picnic and back to overworld.
void leave_picnic(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

// While in the current map zoom level, detect pokecenter icons and move the map cursor there.
// Return true if succeed. Return false if no visible pokcenter on map
bool detect_closest_pokecenter_and_move_map_cursor_there(
    const ProgramInfo& info,
    VideoStream& stream,
    ProControllerContext& context,
    double push_scale = 0.29
);

bool fly_to_visible_closest_pokecenter_cur_zoom_level(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context, 
    double push_scale = 0.29
);

//  While on map (default zoom), move to the closest PokeCenter and fly there.
//  The PokeCenter must be already visited before (so having the little wing icon with it) and not occluded
//  by other map icons on the most zoomed-in level of the map.
void fly_to_closest_pokecenter_on_map(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

// Attempt to escape being stuck on a wall.
// Repeatedly center camera and try to backwards jump off.
// Finishes when map is successfully open.
void jump_off_wall_until_map_open(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

void reset_to_pokecenter(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);


// align player orientation based on the alignment mode
// if battle detected, propagates UnexpectedBattleException to the calling function
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
void realign_player(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    PlayerRealignMode realign_mode,
    uint8_t move_x = 0, uint8_t move_y = 0, uint16_t move_duration = 0
);



void walk_forward_until_dialog(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context,
    NavigationMovementMode movement_mode,
    uint16_t seconds_timeout = 10,
    uint8_t x = 128,
    uint8_t y = 0
);

// walk forward while using lets go to clear the path
// forward_ticks: number of ticks to walk forward
// y = 0: walks forward. y = 128: stand in place. y = 255: walk backwards (towards camera)
// ticks_between_lets_go: number of ticks between firing off Let's go to clear the path from wild pokemon
// delay_after_lets_go: number of ticks to wait after firing off Let's go.
void walk_forward_while_clear_front_path(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context,
    uint16_t forward_ticks,
    uint8_t y = 0,
    uint16_t ticks_between_lets_go = 125,
    uint16_t delay_after_lets_go = 250
);

// fly to the pokecenter that overlaps with the player on the map, and return true.
// if no overlapping pokecenter, return false.
bool attempt_fly_to_overlapping_flypoint(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context
);

// fly to the pokecenter that overlaps with the player on the map
// throw exception if unsuccessful
void fly_to_overlapping_flypoint(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

// throw exception if there is a fly point/pokecenter that overlaps with the player on the map
void confirm_no_overlapping_flypoint(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);


// heal at the pokecenter, that your character is currently at.
// if not currently at the pokecenter, throws error.
void heal_at_pokecenter(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context
);


}
}
}
#endif
