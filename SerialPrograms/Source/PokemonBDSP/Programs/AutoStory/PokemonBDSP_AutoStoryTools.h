/*  BDSP AutoStory Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_AutoStoryTools_H
#define PokemonAutomation_PokemonBDSP_AutoStoryTools_H

#include "CommonFramework/GlobalAutoPaths.h"
#include <array>
#include <functional>
#include <string>
#include <chrono>
#include <vector>
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_SelectionArrow.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Detect/PokemonBDSP_AutoStory_MenuCursorDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


// ---------------------------------------------------------------------------
//  Stats
// ---------------------------------------------------------------------------

struct AutoStoryStats : public StatsTracker{
    AutoStoryStats()
        : m_checkpoint(m_stats["Checkpoint"])
        , m_segment(m_stats["Segment"])
        , m_reset(m_stats["Reset"])
    {
        m_display_order.emplace_back("Checkpoint");
        m_display_order.emplace_back("Segment");
        m_display_order.emplace_back("Reset");
    }
    std::atomic<uint64_t>& m_checkpoint;
    std::atomic<uint64_t>& m_segment;
    std::atomic<uint64_t>& m_reset;
    bool m_first_run = true;
};


// ---------------------------------------------------------------------------
//  Options passed to every segment/checkpoint
// ---------------------------------------------------------------------------

enum class StarterChoice{
    TURTWIG,
    CHIMCHAR,
    PIPLUP,
};

enum class HelpMode{
    NO_OUTSIDE_HELP,
    OUTSIDE_HELP,
};

enum class OutsideHelpPokemon{
    TORTERRA,
};

enum class TorterraMove{
    EARTHQUAKE,
    WOOD_HAMMER,
    RAZOR_LEAF,
    CRUNCH,
    GIGA_DRAIN,
    IRON_TAIL,
    ROCK_TOMB,
    FACADE,
    STONE_EDGE,
    ROCK_SLIDE,
    BULLDOZE,
    STRENGTH,
    ROCK_SMASH,
    ROCK_CLIMB,
};

//  Returns the JSON slug for a TorterraMove (e.g. "earthquake").
const char* torterra_move_slug(TorterraMove move);

enum class Trainer {
    Veilstone_GalacticGrunts_2v2,
    // Extend with new trainer battle IDs as the AutoStory adds them.
};

//  Returns the trainers.json battle-ID string for a Trainer value.
const char* trainer_slug(Trainer t);

struct OutsideHelpConfig{
    OutsideHelpPokemon pokemon;
    TorterraMove attacks[4];
};

enum class CheckpointSaveMode { REAL, FAKE };
// Real saves retry one checkpoint. Fake-save runs own one retry loop around
// the selected range; inner checkpoints propagate failures to that owner.
enum class CheckpointRetryScope { CHECKPOINT, SELECTED_RANGE, DEFER_TO_SELECTED_RANGE };

struct AutoStoryOptions{
    Language language;
    StarterChoice starter_choice;
    EventNotificationOption& notif_status_update;
    HelpMode help_mode;
    OutsideHelpConfig outside_help;
    CheckpointSaveMode checkpoint_save_mode = CheckpointSaveMode::REAL;
    CheckpointRetryScope checkpoint_retry_scope = CheckpointRetryScope::CHECKPOINT;
};


// ---------------------------------------------------------------------------
//  Base classes
// ---------------------------------------------------------------------------

class AutoStory_Segment{
public:
    virtual ~AutoStory_Segment() = default;
    virtual std::string name()       const = 0;
    virtual std::string start_text() const = 0;
    virtual std::string end_text()   const = 0;
    virtual void run_segment(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        AutoStoryOptions options,
        AutoStoryStats& stats
    ) const = 0;
};

class AutoStory_Checkpoint{
public:
    virtual ~AutoStory_Checkpoint() = default;
    virtual std::string name()       const = 0;
    virtual std::string start_text() const = 0;
    virtual std::string end_text()   const = 0;
    virtual void run_checkpoint(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        AutoStoryOptions options,
        AutoStoryStats& stats
    ) const = 0;
};


// ---------------------------------------------------------------------------
//  Shared utility functions (implementations in .cpp)
// ---------------------------------------------------------------------------

//  Save the game from the overworld with retries. Returns true if the save was
//  verified via overworld detection, false if the blind fallback was used.
bool save_game(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context
);

// Fake save game to be replaced with save_game when program runs smoothly
void checkpoint_save(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context
);

//  Save the game from the overworld, increment the checkpoint counter,
//  update stats display, and send a status notification.
void checkpoint_save(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats,
    CheckpointSaveMode save_mode = CheckpointSaveMode::REAL
);

//  Reload the game from the most recent backup save (auto-save).
//  Called on retry when a checkpoint has no hard save (save=false).
//  TODO: implement this function.
void reload_backup_save(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context
);

//  If save=true (default): save the game before the first attempt, then run
//  `action(attempt_number)`. On failure, reset from HOME and retry.
//  If save=false: skip the hard save. On failure, reload from backup save and retry.
//  SELECTED_RANGE skips the entry save and owns HOME resets for the range.
//  DEFER_TO_SELECTED_RANGE lets an inner failure reach that outer retry loop.
//  Throws a fatal error after too many consecutive failures.
void checkpoint_reattempt_loop(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats,
    std::function<void(size_t attempt_number)>&& action,
    bool save = true,
    CheckpointSaveMode save_mode = CheckpointSaveMode::REAL,
    CheckpointRetryScope retry_scope = CheckpointRetryScope::CHECKPOINT
);

// Forward the explicit checkpoint save mode selected by AutoStory's options.
inline void checkpoint_reattempt_loop(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    AutoStoryOptions options, AutoStoryStats& stats,
    std::function<void(size_t attempt_number)>&& action, bool save = true
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        std::move(action), save, options.checkpoint_save_mode, options.checkpoint_retry_scope);
}

//  Execute every action in `actions` in order, repeated `times` times.
void repeat(std::initializer_list<std::function<void()>> actions, size_t times);

//  Tracks the last D-pad direction pressed. Pass one per navigation function
//  so each function gets a clean slate on each attempt.
struct DpadState {
    DpadPosition last_dir = DPAD_NONE;
};

//  Press `dir` on the D-pad `times` times (plus one extra if the direction
//  changed from the previous call, since the first press turns the character).
void repeat_dpad(
    ProControllerContext& context,
    DpadState& state,
    DpadPosition dir,
    Milliseconds press,
    Milliseconds hold,
    size_t times,
    bool include_last_dir = true
);

//  Wait (without pressing any buttons) until a short dialogue box appears.
//  Logs `label + " dialogue detected."` when found (omit label to skip logging).
//  Throws OperationFailedException if no dialogue appears within `timeout`.
void wait_for_dialogue(
    VideoStream& stream,
    ProControllerContext& context,
    const std::string& label = "",
    Milliseconds timeout = std::chrono::seconds(60)
);

//  Mash `button` (default B) until the game returns to the free-roaming overworld
//  (no dialogue, menu, or battle UI visible).
//  Throws OperationFailedException if the overworld is not reached within `timeout`.
void mash_until_dialogue_ends(
    VideoStream& stream,
    ProControllerContext& context,
    Button button = BUTTON_B,
    Milliseconds timeout = std::chrono::seconds(60)
);

// Scripted trainer battle: choose a 1-based move position each turn, then
// confirm the default target with A and clear narration with B. No target steering.
// A single position is reused for the whole battle. A list is consumed in
// order, then its final position is reused. Empty lists and slots outside 1-4 fail.
// post_battle_dialogue_cleanup must be chosen explicitly: false returns after
// the completed battle-exit transition; true also clears post-fight NPC dialogue.
// Examples:
//   fight_trainer_using_moves(stream, context, 2, "Grunt", false);
//   const std::vector<size_t> moves{2, 1, 3};
//   fight_trainer_using_moves(stream, context, moves, "Gym leader", true);
void fight_trainer_using_moves(VideoStream& stream, ProControllerContext& context,
    size_t move, const std::string& label, bool post_battle_dialogue_cleanup);
void fight_trainer_using_moves(VideoStream& stream, ProControllerContext& context,
    const std::vector<size_t>& moves, const std::string& label, bool post_battle_dialogue_cleanup);

// A full Bag screenshot with the desired item highlighted. Paths are relative
// to RESOURCE_PATH(); quantities and selection borders are excluded.
struct BagItemTemplate {
    std::string reference_path;
    std::string label;
    ImageFloatBox icon_box{0.428, 0.754, 0.029, 0.044};
    ImageFloatBox name_box{0.460, 0.755, 0.350, 0.045};
    double max_icon_rmsd = 25.0;
    double max_name_rmsd = 20.0;
    bool check_name = true;
    // Selected item row in the reference screenshot (1-7).
    size_t reference_row = 1;
};

// Screenshot-only helpers shared by live search and developer tests.
// Cursor 0 means unknown. Array index 0 corresponds to visible row 1.
struct BagItemRowMatch {
    double icon_rmsd = 1000.0;
    double name_rmsd = 1000.0;
    bool matched = false;
};

// Pocket positions 1-8: Medicine, Balls, Battle Items, Berries, Other Items,
// TMs, Treasures, Key Items. Zero means the selected tab is uncertain.
int bag_pocket_position(const ImageViewRGB32& frame);
// Count normal, unobscured pocket icons; NEW badges prevent a tab match.
size_t bag_clear_tab_count(const ImageViewRGB32& frame);
// Open bag, scan with 250ms Down / 100ms rest (3s cumulative Down per pocket)
// until all 8 tabs match,
// then restore the top of the item list and close the bag.
// Returns false after four bounded pocket passes.
bool clear_new_item_flags(VideoStream& stream, ProControllerContext& context, int num_icons=8);
bool bag_list_is_empty(const ImageViewRGB32& frame);
int bag_cursor_position(const ImageViewRGB32& frame);
std::array<BagItemRowMatch, 7> match_bag_item_rows(
    const ImageViewRGB32& frame,
    const ImageViewRGB32& reference,
    const BagItemTemplate& item
);

enum class BagMoveAction { UNKNOWN, CONFIRM, UP, DOWN, PAGE_DOWN, REWIND };
struct BagMovePlan {
    BagMoveAction action = BagMoveAction::UNKNOWN;
    size_t presses = 0;
};

// Describes the next input without sending it. REWIND means repeat Up and
// recheck until row 1; a screenshot cannot tell the total distance to the top.
BagMovePlan plan_bag_item_move(int cursor_row, int item_row);

enum class BagPocketAction { UNKNOWN, CONFIRM, LEFT, RIGHT };
struct BagPocketMovePlan {
    BagPocketAction action = BagPocketAction::UNKNOWN;
    size_t presses = 0;
};
// Circular eight-pocket route; ties prefer Right. Invalid positions send no input.
BagPocketMovePlan plan_bag_pocket_move(int current, int target);


// Starts in the overworld, opens the Bag, and leaves the requested item
// highlighted. Navigates to the requested pocket, then searches its item rows.
// Checks all seven visible rows and the cursor, then confirms the selected
// item in the green header. Makes up to three passes per pocket.
// Limits report an incomplete search, not absence.
// Returns false on missing frames, timeout, or no match; does not use an item.
bool find_bag_item(
    VideoStream& stream,
    ProControllerContext& context,
    const BagItemTemplate& item,
    int target_pocket,
    int num_icons = 8,
    size_t max_scrolls_per_pocket = 512,
    std::chrono::milliseconds search_timeout = std::chrono::minutes(10)
);

// Find Escape Rope, choose Use, and verify a transition back to the overworld.
bool use_escape_rope(
    VideoStream& stream,
    ProControllerContext& context,
    int num_icons = 8
);

// Reuse the tutorial's verified Jubilife Center alignment and entry.
void align_and_enter_jubilife_center(VideoStream& stream, ProControllerContext& context);

//  Open the bag, navigate to the repel compartment tab, and use a repel.
//  Returns false if the repel compartment tab could not be found.
//  num_icons: early/late menu layout (preserved for existing callers).
//  repel_type: 0=Repel, 1=Super Repel, 2=Max Repel. English item names.
bool activate_repel(
    VideoStream& stream,
    ProControllerContext& context,
    int num_icons = 8,
    int repel_type = 0
);

//  Press A until the any HM selection arrow appears, confirm with A, then mash B for 3 s.
void use_HM(
    VideoStream& stream,
    ProControllerContext& context,
    const std::string& label
);

//  Walk to the nurse, heal the party, and exit the Pokemon Center.
//  `label` names the center for log messages (e.g. "Veilstone").
//  Returns false if the exit black screen was not detected.
bool heal_pokemon(
    VideoStream& stream,
    ProControllerContext& context,
    const std::string& label
);

enum class FlyPoint {
    CanalavCity,
    CelesticTown,
    EternaCity,
    FloaromaTown,
    HearthomeCity,
    JubilifeCity,
    OreburghCity,
    PastoriaCity,
    PokemonLeagueLower,
    PokemonLeagueUpper,
    Route221,
    SandgemTown,
    SolaceonTown,
    SnowpointCity,
    SunyshoreCity,
    TwinleafTown,
    VeilstoneCity,
    CurrentCity,    // cursor already starts on the city; skip all map navigation
};

std::string fly_point_name(FlyPoint place);

// Open the map, move the cursor to either bottom left, bottom right or
// top left, then move the cursor a number of steps, then checks for
// correctness, and mashes A until a black screen is detected
bool fly_to(
    VideoStream& stream,
    ProControllerContext& context,
    FlyPoint place
);

// Opens the X-menu from the overworld, navigates the cursor to `target`, and presses A.
// num_icons: 5 (early-game layout) or 8 (late-game layout).
void open_menu(
    VideoStream& stream,
    ProControllerContext& context,
    MenuCursorPosition target,
    int num_icons
);

void use_potion_first_pokemon(
    VideoStream& stream,
    ProControllerContext& context
);

enum class GearLevel {
    Fast,
};


// ---------------------------------------------------------------------------
//  Mart buying
// ---------------------------------------------------------------------------

enum class ShopItem {
    FullRestore,
    MaxPotion,
    HyperPotion,
    SuperPotion,
    Potion,
    Antidote,
    ParalyzHeal,
    Awakening,
    Repel,
    SuperRepel,
    MaxRepel,
    EscapeRope,
};

// Pass as the `quantity` field to buy the maximum available amount.
static constexpr int BUY_MAX = -1;

struct ItemToBuy {
    ShopItem item;
    int      quantity;  // positive count to buy, or BUY_MAX
};

// Walk up to a vendor (already talking to them), navigate through the Buy menu,
// and purchase each item in `items` in order. The function presses DPAD_DOWN
// until the matching item name is found via OCR, so items may appear in any
// list order. Returns true on success.
bool buy_items(
    VideoStream& stream,
    ProControllerContext& context,
    const std::vector<ItemToBuy>& items,
    Language language = Language::English
);


// Mount the bicycle (press +) and confirm it is in the requested gear.
// Presses B to toggle gear and uses the Fast-gear spark visual effect to
// determine which gear was entered. If the wrong gear is entered, presses B
// again and re-checks. Player must be on foot when called.
// Returns false if gear state cannot be confirmed after two toggles.
bool get_on_bicycle(
    VideoStream& stream,
    ProControllerContext& context,
    GearLevel gear
);

}
}
}
#endif
