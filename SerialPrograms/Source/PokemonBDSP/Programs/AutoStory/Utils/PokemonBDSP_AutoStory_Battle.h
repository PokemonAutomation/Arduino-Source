/*  BDSP AutoStory - Battle Utilities
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Helpers for handling trainer battles and unexpected wild encounters.
 *  These wrap existing BDSP battle detectors:
 *    - BattleMenuDetector / BattleMenuWatcher    (PokemonBDSP_BattleMenuDetector.h)
 *    - EndBattleWatcher                          (PokemonBDSP_EndBattleDetector.h)
 *    - StartBattleDetector                       (PokemonBDSP_StartBattleDetector.h)
 *    - ExperienceGainWatcher                     (PokemonBDSP_ExperienceGainDetector.h)
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_AutoStory_Battle_H
#define PokemonAutomation_PokemonBDSP_AutoStory_Battle_H

#include <functional>
#include <string>
#include <vector>
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "PokemonBDSP/Programs/AutoStory/PokemonBDSP_AutoStoryTools.h"

namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonBDSP{


// ---------------------------------------------------------------------------
//  Scripted (expected) battles
// ---------------------------------------------------------------------------

//  Mash A through a battle (trainer or scripted wild) until EndBattleWatcher fires.
//  This does not attempt to catch — it simply presses A to select the first move
//  repeatedly until the battle ends.
//  After the black-screen transition and experience gain, returns to the caller.
void run_battle_press_A(
    const ProgramInfo& info,
    VideoStream& stream,
    ProControllerContext& context
);


// ---------------------------------------------------------------------------
//  Unexpected battle handling
// ---------------------------------------------------------------------------

//  Run `action`. If a StartBattleDetector fires during `action`, the action
//  is aborted, the battle is resolved with run_battle_press_A(), and then
//  `recovery_action` is called to restore the player's position before
//  retrying `action`.
//  The whole sequence retries up to `max_attempts` times.
//  Throws OperationFailedException if max_attempts is exceeded.
void handle_unexpected_battle(
    const ProgramInfo& info,
    VideoStream& stream,
    ProControllerContext& context,
    std::function<void(const ProgramInfo&, VideoStream&, ProControllerContext&)>&& action,
    std::function<void(const ProgramInfo&, VideoStream&, ProControllerContext&)>&& recovery_action,
    size_t max_attempts = 5
);


// ---------------------------------------------------------------------------
//  Outside-help battle handler
// ---------------------------------------------------------------------------

//  Handle a scripted trainer battle where a human helper (Torterra lv100) joins.
//  Each turn it:
//    1. OCRs current PP for all 4 configured moves.
//    2. Calculates damage against all on-field opponents.
//    3. Navigates to and selects the optimal move (guaranteed OHKO if possible;
//       highest total damage otherwise).
//    4. Mashes A to cover target selection and partner's move.
//    5. After XP gain, updates on-field opponents via OCR.
//  Returns when the battle ends and the player is back in the overworld.
void handle_battle_with_outside_help(
    const ProgramInfo& info,
    VideoStream& stream,
    ProControllerContext& context,
    const std::string& trainer_id,
    const AutoStoryOptions& options
);




// ---------------------------------------------------------------------------
//  Expert battle handler (sequence-aware, per-turn status/HP check)
// ---------------------------------------------------------------------------

//  Handle one scripted trainer battle with intelligent per-turn move selection.
//
//  `all_trainers` — the full upcoming sequence of trainer battles. Used to
//  pre-plan PP budget so moves are allocated efficiently across the sequence.
//  `current_trainer` — the trainer being fought in this call.
//
//  Each turn the function:
//    1. Checks player HP (<33%) and status condition; uses a Full Restore if needed.
//    2. OCRs current PP for all 4 configured moves.
//    3. Identifies the current opponent via restricted-dictionary OCR.
//    4. Tracks Intimidate activations and reduces effective ATK accordingly.
//    5. Selects the pre-planned optimal move and executes it.
//    6. After XP gain, OCRs the new opponent and loops.
//  Returns when the battle ends and the player is back in the overworld.
void handle_expert_battle(
    const ProgramInfo& info,
    VideoStream& stream,
    ProControllerContext& context,
    const std::vector<Trainer>& all_trainers,
    Trainer current_trainer,
    const AutoStoryOptions& options
);


}
}
}
#endif
