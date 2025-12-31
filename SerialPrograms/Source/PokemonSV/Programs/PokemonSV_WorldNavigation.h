/*  PokemonSV World Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Move player character around the overworld
 *
 */

#ifndef PokemonAutomation_PokemonSV_WorldNavigation_H
#define PokemonAutomation_PokemonSV_WorldNavigation_H

#include <unordered_set>
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"

namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{

enum class PlayerRealignMode{
    REALIGN_NEW_MARKER,
    REALIGN_OLD_MARKER,
    REALIGN_NO_MARKER,
};

enum class NavigationMovementMode{
    DIRECTIONAL_ONLY,
    DIRECTIONAL_SPAM_A,
    CLEAR_WITH_LETS_GO,
};

enum class FlyPoint{
    POKECENTER,
    FAST_TRAVEL,
};
struct ExpectedMarkerPosition{
    double x;
    double y;
};

enum class BattleStopCondition{
    STOP_OVERWORLD,
    STOP_DIALOG,
};

enum class CallbackEnum{
    ADVANCE_DIALOG,
    OVERWORLD,
    PROMPT_DIALOG,
    WHITE_A_BUTTON,
    DIALOG_ARROW,
    BATTLE,
    TUTORIAL,
    BLACK_DIALOG_BOX,
    NEXT_POKEMON,
    SWAP_MENU,
    MOVE_SELECT,
    SELECT_MOVE_TARGET,
    BATTLE_BAG,
};

enum class ZoomChange{
    ZOOM_IN,
    ZOOM_IN_TWICE,
    ZOOM_OUT,
    ZOOM_OUT_TWICE,
    KEEP_ZOOM,
};

struct MoveCursor{
    ZoomChange zoom_change;
    double move_x;
    double move_y;
    Milliseconds move_duration;
};

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

void print_flypoint_location(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context, FlyPoint fly_point);

void place_marker_offset_from_flypoint(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context, MoveCursor move_cursor_near_flypoint, FlyPoint fly_point, ExpectedMarkerPosition marker_offset);

// with the map open, move the cursor to a specific position offset from the flypoint, as per marker_offset
// throw exception if no flypoints visible on map
void move_cursor_to_position_offset_from_flypoint(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context, FlyPoint fly_point, ExpectedMarkerPosition marker_offset);

// While in the current map zoom level, detect pokecenter icons and move the map cursor there.
// Return true if succeed. Return false if no visible pokcenter on map
bool detect_closest_flypoint_and_move_map_cursor_there(
    const ProgramInfo& info,
    VideoStream& stream,
    ProControllerContext& context,
    FlyPoint fly_point,
    double push_scale = 0.29
);

bool fly_to_visible_closest_flypoint_cur_zoom_level(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context, 
    FlyPoint fly_point,
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






void walk_forward_until_dialog(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context,
    NavigationMovementMode movement_mode,
    Milliseconds timeout = Milliseconds(10000),
    uint8_t x = 128,
    uint8_t y = 0
);

// walk forward while using lets go to clear the path
// forward_duration: number of ticks to walk forward
// y = 0: walks forward. y = 128: stand in place. y = 255: walk backwards (towards camera)
// duration_between_lets_go: milliseconds between firing off Let's go to clear the path from wild pokemon
// delay_after_lets_go: milliseconds to wait after firing off Let's go.
void walk_forward_while_clear_front_path(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context,
    Milliseconds forward_duration,
    uint8_t y = 0,
    Milliseconds duration_between_lets_go = Milliseconds(1000),
    Milliseconds delay_after_lets_go = Milliseconds(2000)
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

// spam A button to choose the first move for double trainer battles
// detect_wipeout: can be false if you have multiple pokemon in your party, since an exception will be thrown if your lead faints.
// throw exception if wipeout or if your lead faints.
void run_trainer_double_battle_press_A(
    VideoStream& stream,
    ProControllerContext& context,
    BattleStopCondition stop_condition,
    std::unordered_set<CallbackEnum> enum_optional_callbacks = {},
    bool detect_wipeout = false
);


// spam A button to choose the first move for trainer battles
// detect_wipeout: can be false if you have multiple pokemon in your party, since an exception will be thrown if your lead faints.
// throw exception if wipeout or if your lead faints.
void run_trainer_battle_press_A(
    VideoStream& stream,
    ProControllerContext& context,
    BattleStopCondition stop_condition,
    std::unordered_set<CallbackEnum> enum_optional_callbacks = {},
    bool detect_wipeout = false
);

// spam A button to choose the first move for wild battles
// detect_wipeout: can be false if you have multiple pokemon in your party, since an exception will be thrown if your lead faints.
// throw exception if wipeout or if your lead faints.
void run_wild_battle_press_A(
    VideoStream& stream,
    ProControllerContext& context,
    BattleStopCondition stop_condition,
    std::unordered_set<CallbackEnum> enum_optional_callbacks = {},
    bool detect_wipeout = false
);

void select_top_move(VideoStream& stream, ProControllerContext& context, size_t consecutive_move_select);


}
}
}
#endif
