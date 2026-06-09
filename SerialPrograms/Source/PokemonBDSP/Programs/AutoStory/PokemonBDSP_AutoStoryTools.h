/*  BDSP AutoStory Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_AutoStoryTools_H
#define PokemonAutomation_PokemonBDSP_AutoStoryTools_H

#include <functional>
#include <string>
#include <chrono>
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

struct OutsideHelpConfig{
    OutsideHelpPokemon pokemon;
    TorterraMove attacks[4];
};

struct AutoStoryOptions{
    Language language;
    StarterChoice starter_choice;
    EventNotificationOption& notif_status_update;
    HelpMode help_mode;
    OutsideHelpConfig outside_help;
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
    AutoStoryStats& stats
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
//  Throws a fatal error after too many consecutive failures.
void checkpoint_reattempt_loop(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats,
    std::function<void(size_t attempt_number)>&& action,
    bool save = true
);

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

//  Open the bag, navigate to the repel compartment tab, and use a repel.
//  Returns false if the repel compartment tab could not be found.
//  num_icons: 5 (early-game layout) or 8 (late-game layout).
bool activate_repel(
    VideoStream& stream,
    ProControllerContext& context,
    int num_icons = 8
);

//  Press A until the Strength selection arrow appears, confirm with A, then mash B for 3 s.
void use_strength(
    VideoStream& stream,
    ProControllerContext& context
);

//  Press A until the Rock Smash selection arrow appears, confirm with A, then mash B for 3 s.
void use_rock_smash(
    VideoStream& stream,
    ProControllerContext& context
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
