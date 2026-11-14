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

#include <array>
#include <functional>
#include <string>
#include <vector>

#include "CommonFramework/Tools/VideoStream.h"

#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"

#include "PokemonBDSP/Programs/AutoStory/PokemonBDSP_AutoStoryTools.h"

namespace PokemonAutomation {
struct ProgramInfo;
namespace NintendoSwitch {
namespace PokemonBDSP {


// ---------------------------------------------------------------------------
//  Scripted (expected) battles
// ---------------------------------------------------------------------------

//  Mash A through a battle (trainer or scripted wild) until EndBattleWatcher fires.
//  This does not attempt to catch — it simply presses A to select the first move
//  repeatedly until the battle ends.
//  After the black-screen transition and experience gain, returns to the caller.
void run_battle_press_A(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);


enum class BattleHPLayout{
    SINGLES,
    NPC_PARTNER,
    OWN_PARTNER,
};

struct BattleHPReading{
    int current = -1;
    int maximum = -1;
    size_t command_owner = 0;
    bool operator==(const BattleHPReading&) const = default;
};

// Exact numeric command-screen HP fraction, cross-checked against that slot's
// HP bar; -1 means unconfirmed. Separate raised/lowered left/right crops are
// enlarged before OCR. No threshold/grayscale conversion is used.
// Slots are battlefield positions (1=left, 2=right), not persistent identities.
// OWN_PARTNER requires both panels; switching/survivor identity is unverified.
// Call only immediately before a decision and require fresh matching readings.
double read_player_hp_ratio(const ImageViewRGB32& frame,
    BattleHPLayout layout = BattleHPLayout::SINGLES,
    size_t slot = 1,
    BattleHPReading* reading = nullptr);

// Current PP by move slot; -1 means absent or unreadable. Caller confirms across frames.
std::array<int, 4> read_all_pp(Logger& logger, const ImageViewRGB32& frame);

// Starts at trainer commands. The Bag target is an explicit PARTY slot (1-6),
// independent of the command owner. Own-party doubles can request both items
// for the same target from different command owners; targets must be confirmed.
// X items currently assume active party slots 1/2 with no switching; NPC allies
// are not party targets. Double targeting still requires live validation.
// Returns on narration or the next own-party command, without canceling it.
// after_battle retains the first-party-Pokemon exact-HP heal/recheck routine.
void use_item_in_battle(VideoStream& stream,
    ProControllerContext& context,
    Language language,
    const std::string& item_slug,
    bool after_battle = false,
    BattleHPLayout layout = BattleHPLayout::SINGLES,
    size_t target_party_slot = 1,
    size_t expected_command_owner = 0);


// ---------------------------------------------------------------------------
//  Unexpected battle handling
// ---------------------------------------------------------------------------

//  Run `action`. If a StartBattleDetector fires during `action`, the action
//  is aborted, the battle is resolved with run_battle_press_A(), and then
//  `recovery_action` is called to restore the player's position before
//  retrying `action`.
//  The whole sequence retries up to `max_attempts` times.
//  Throws OperationFailedException if max_attempts is exceeded.
void handle_unexpected_battle(const ProgramInfo& info,
    VideoStream& stream,
    ProControllerContext& context,
    std::function<void(const ProgramInfo&, VideoStream&, ProControllerContext&)>&& action,
    std::function<void(const ProgramInfo&, VideoStream&, ProControllerContext&)>&& recovery_action,
    size_t max_attempts = 5);


// ---------------------------------------------------------------------------
//  Outside-help battle handler
// ---------------------------------------------------------------------------

//  Not called by current story routes. Retained for possible reuse in future
//  unassisted battle planning; the assisted route uses established move lists.
//  Validate the required game data and level-100 Torterra assumptions before reuse.
//
//  Handle a scripted trainer battle with the configured outside-help Pokemon.
//  Each turn it:
//    1. OCRs current PP for all 4 configured moves.
//    2. Calculates damage against all on-field opponents.
//    3. Navigates to and selects the optimal move (guaranteed OHKO if possible;
//       highest total damage otherwise).
//    4. Mashes A to cover target selection and partner's move.
//    5. After XP gain, updates on-field opponents via OCR.
//  Returns when the battle ends and the player is back in the overworld.
void handle_battle_with_outside_help(const ProgramInfo& info,
    VideoStream& stream,
    ProControllerContext& context,
    const std::string& trainer_id,
    const AutoStoryOptions& options);


// ---------------------------------------------------------------------------
//  Expert battle handler (sequence-aware, per-turn status/HP check)
// ---------------------------------------------------------------------------

//  Not called by current story routes. Retained for future unassisted work on
//  move selection and PP planning. Its data, damage model, and level-100 Torterra
//  assumptions require validation and adaptation before enabling it.
//
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
void handle_expert_battle(const ProgramInfo& info,
    VideoStream& stream,
    ProControllerContext& context,
    const std::vector<Trainer>& all_trainers,
    Trainer current_trainer,
    const AutoStoryOptions& options);


}
}
}
#endif
