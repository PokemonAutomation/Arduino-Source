/*  Auto Story Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_AutoStoryTools_H
#define PokemonAutomation_PokemonLZA_AutoStoryTools_H

#include <array>
#include <chrono>
#include <string>
#include "Common/Cpp/Time.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/VisualDetectors/ImageMatchDetector.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "VideoGames/PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

class BattleAIOption;

enum class StarterChoice{
    CHIKORITA,
    TOTODILE,
    TEPIG,
};

struct AutoStoryOptions{
    StarterChoice starter_choice;
    BattleAIOption& battle_ai;
};

struct AutoStoryStats : public StatsTracker{
    AutoStoryStats()
        : m_segment(m_stats["Segment"])
        , m_checkpoint(m_stats["Checkpoint"])
    {
        m_display_order.emplace_back("Segment");
        m_display_order.emplace_back("Checkpoint");
    }

    std::atomic<uint64_t>& m_segment;
    std::atomic<uint64_t>& m_checkpoint;
};

class AutoStory_Segment{
public:
    virtual ~AutoStory_Segment() = default;
    virtual std::string name() const = 0;
    virtual std::string start_text() const = 0;
    virtual std::string end_text() const = 0;
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
    virtual std::string name() const = 0;
    virtual std::string start_text() const = 0;
    virtual std::string end_text() const = 0;
    virtual void run_checkpoint(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        AutoStoryOptions options,
        AutoStoryStats& stats
    ) const = 0;
};

// STOP_BLACK_SCREEN waits for a black screen to appear and then finish
// STOP_WHITE_SCREEN stops when a white screen appears, before it finishes
// STOP_BATTLE waits for the opponent-party icons to appear in a trainer battle
// STOP_CHOICES stops at a right-pointing choice arrow without confirming it
// OverworldPartySelectionWatcher detects the overworld
enum class ClearDialogMode{
    STOP_OVERWORLD,
    STOP_BLACK_SCREEN,
    STOP_BATTLE,
    STOP_CHOICES,
    STOP_WHITE_SCREEN,
};

// Mash A (or B) until the selected stop condition and throws on timeout
// STOP_CHOICES waits 1500 ms before the first press, then uses 100 ms presses with 1500 ms releases to wait on choice arrows
// All modes except STOP_CHOICES confirm highlighted choices with A, even when mashing B through other dialogue
// All modes advance tutorial windows with A, with or without page arrows
void clear_dialog(
    ConsoleHandle& console, ProControllerContext& context,
    ClearDialogMode mode = ClearDialogMode::STOP_OVERWORLD,
    Button button = BUTTON_A,
    Milliseconds timeout = std::chrono::minutes(4)
);

// Hold the requested button continuously until any supported dialogue appears
// Releases the button when detection stops the hold, throws on timeout
void hold_button_until_dialog(
    ConsoleHandle& console, ProControllerContext& context, Button button,
    Milliseconds timeout = std::chrono::minutes(2)
);

// Repeat the requested press/release cadence until any supported dialog appear
void press_button_until_dialog(
    ConsoleHandle& console, ProControllerContext& context, Button button,
    Milliseconds hold = std::chrono::milliseconds(100),
    Milliseconds release = std::chrono::milliseconds(100),
    Milliseconds timeout = std::chrono::minutes(2)
);

// Mash the requested button until the requested on-screen icon appears, throws on timeout.
void mash_button_until_button(
    ConsoleHandle& console, ProControllerContext& context,
    Button mash_button, ButtonType button_type, const ImageFloatBox& button_box,
    Milliseconds timeout = std::chrono::minutes(2)
);

// Mash the requested button until a doorway/entry-arrow icon is detected, throws on timeout.
void mash_button_until_door_entry(
    ConsoleHandle& console, ProControllerContext& context,
    Button mash_button = BUTTON_A,
    Milliseconds timeout = std::chrono::minutes(2)
);

// Match filled opponent-party icons at the six top HUD slots
// Any slot can signal a trainer battle independent of whether we can run from the battle
std::array<ImageMatchWatcher, 6> make_trainer_battle_watchers(Milliseconds hold_duration);

// Tutorial pagers use the right-button icon in either light or dark colors
// Search across the lower half of the screen to cover different page lengths
class TutorialPageWatcher : public DetectorToFinder<ButtonDetector>{
public:
    TutorialPageWatcher(
        Color color, VideoOverlay* overlay = nullptr,
        Milliseconds hold_duration = std::chrono::milliseconds(100)
    );

    virtual bool detect(const ImageViewRGB32& screen) override;
};

enum class BattleStopCondition{
    STOP_OVERWORLD,
    STOP_BLACK_SCREEN,
};

// Advance dialogue with A and use the existing battle AI until the selected condition
// STOP_BLACK_SCREEN stops when black appears, before it finishes
// Advance tutorial windows with A, including pages without navigation arrows
// Close normal/tutorial maps with B when detected
// Confirm highlighted dialogue options with A when their selection arrow appears
// Keep one battle state throughout the fight, including lock recovery
// Throws if the entire sequence exceeds timeout
void run_battle_until_condition(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    BattleAIOption& battle_ai, BattleStopCondition stop_condition,
    Milliseconds timeout = std::chrono::minutes(5)
);

enum class WildBattleStopCondition{
    STOP_DIALOG,
};

// Use the configured battle AI with a Pokemon already out until the selected condition, throws on timeout
// STOP_DIALOG requires white dialogue for 1.5 continuous seconds to prevent matching with the background or moving Pokemon
void run_wild_battle_until_condition(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    BattleAIOption& battle_ai, WildBattleStopCondition stop_condition,
    Milliseconds timeout = std::chrono::minutes(1)
);

// Turn the camera in short increments until facing slightly left of the news booth
// Requires the booth to be visible; throws on lost detection or alignment timeout
void align_camera_left_of_booth(ConsoleHandle& console, ProControllerContext& context);

// Search for the event marker, with or without its green flag, and turn the camera with corrections until centered for 400 ms
// Recover a temporarily clipped marker in its last known direction (up to 4 turns)
// Detection excludes the minimap, throws on failed recovery or a 30-second timeout
void center_event_marker(ConsoleHandle& console, ProControllerContext& context);

// Cycle through the overworld party with the existing selection watcher
// Returns whether slot count (1-6) is selectable, restoring the starting selection
// Throws if party selection cannot be read
bool party_has_at_least(ConsoleHandle& console, ProControllerContext& context, uint8_t count);

// Throw the currently selected ball using Stats Reset's ZL + ZR sequence
void run_catch(
    ProControllerContext& context,
    Milliseconds post_throw_wait = std::chrono::seconds(6)
);

enum class NavigationMovementMode{
    DIRECTIONAL_ONLY,
    DIRECTIONAL_SPAM_A,
};

enum class NavigationStopCondition{
    STOP_DIALOG,
    STOP_BUTTON,
    STOP_DOOR_ENTRY,
    STOP_TIME,
    STOP_BATTLE,
    STOP_BLACK_SCREEN,
};

// Move in the requested direction, optionally pressing A with 160 ms hold and 40 ms release
// All inputs stop when the selected condition is met
// STOP_BUTTON uses button_type and button_box
// STOP_DOOR_ENTRY matches the doorway/entry-arrow icon in the central screen area
// STOP_BATTLE matches the opponent-party icons at the top of the screen
// STOP_BLACK_SCREEN waits for a black screen to appear
// STOP_TIME completes normally after timeout
void overworld_navigation(
    ConsoleHandle& console, ProControllerContext& context,
    NavigationStopCondition stop_condition, NavigationMovementMode movement_mode,
    double direction_x, double direction_y,
    Milliseconds timeout = std::chrono::seconds(60),
    bool sprint = false,
    ButtonType button_type = ButtonType::ButtonA,
    const ImageFloatBox& button_box = {0.3, 0.2, 0.4, 0.7}
);

}
}
}
#endif
