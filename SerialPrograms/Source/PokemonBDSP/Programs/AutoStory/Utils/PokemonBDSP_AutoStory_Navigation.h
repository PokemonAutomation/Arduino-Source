/*  BDSP AutoStory - Navigation Utilities
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Helpers for moving the player character in the BDSP overworld.
 *  BDSP uses a tile-based grid, so most movement is fixed D-pad sequences.
 *  Detection-based stop conditions and post-battle recalibration are also here.
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_AutoStory_Navigation_H
#define PokemonAutomation_PokemonBDSP_AutoStory_Navigation_H

#include <cstdint>
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonBDSP{


// ---------------------------------------------------------------------------
//  Fixed-sequence walking (no detection — pure tile counting)
// ---------------------------------------------------------------------------

//  Walk `steps` tiles in the given D-pad direction.
//  direction must be one of DPAD_UP / DPAD_DOWN / DPAD_LEFT / DPAD_RIGHT.
void walk_steps(ProControllerContext& context, uint16_t direction, uint16_t steps);


// ---------------------------------------------------------------------------
//  Detection-guided walking (walk until a screen state is detected)
// ---------------------------------------------------------------------------

//  Walk in `direction` until a short dialog box is detected, or `timeout_sec` elapses.
//  Throws OperationFailedException on timeout.
void walk_until_dialog(
    VideoStream& stream,
    ProControllerContext& context,
    uint16_t direction,
    uint16_t timeout_sec = 30
);

//  Walk in `direction` until a battle starts (StartBattleDetector fires),
//  or `timeout_sec` elapses.
//  Throws OperationFailedException on timeout.
void walk_until_battle(
    VideoStream& stream,
    ProControllerContext& context,
    uint16_t direction,
    uint16_t timeout_sec = 30
);


// ---------------------------------------------------------------------------
//  Post-battle recalibration
//
//  BDSP movement is deterministic on a grid, but a random encounter shifts
//  the player's position unpredictably (post-battle exit tile varies).
//  These functions restore the player to a known reference tile by navigating
//  to a fixed landmark (e.g. a building entrance).
// ---------------------------------------------------------------------------

//  After an unexpected battle, navigate to the nearest known landmark and
//  re-establish a known tile position.  The exact recovery sequence depends
//  on the calling segment and is passed as a lambda.
//  `recovery_action` should move the player from wherever they ended up back
//  to a defined starting tile for the current checkpoint.
void recalibrate_after_battle(
    const ProgramInfo& info,
    VideoStream& stream,
    ProControllerContext& context,
    std::function<void(ProControllerContext&)>&& recovery_action
);


}
}
}
#endif
