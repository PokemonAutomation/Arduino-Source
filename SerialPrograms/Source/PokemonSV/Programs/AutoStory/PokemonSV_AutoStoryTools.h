/*  AutostoryTools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoStoryTools_H
#define PokemonAutomation_PokemonSV_AutoStoryTools_H

#include <functional>
#include <unordered_set>
#include "CommonFramework/Language.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
// #include "PokemonSV/Programs/PokemonSV_Navigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


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
};    


enum class BattleStopCondition{
    STOP_OVERWORLD,
    STOP_DIALOG,
};

enum class ClearDialogMode{
    STOP_OVERWORLD,
    STOP_PROMPT,
    STOP_WHITEBUTTON,
    STOP_TIMEOUT,
    STOP_BATTLE,
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
};

enum class StartPoint{
    INTRO_CUTSCENE,
    PICK_STARTER,
    NEMONA_FIRST_BATTLE,
    CATCH_TUTORIAL,
    LEGENDARY_RESCUE,
    ARVEN_FIRST_BATTLE,
    LOS_PLATOS,
    MESAGOZA_SOUTH,
};

enum class StarterChoice{
    SPRIGATITO,
    FUECOCO,
    QUAXLY,
};

enum class PlayerRealignMode{
    REALIGN_NEW_MARKER,
    REALIGN_OLD_MARKER,
    REALIGN_NO_MARKER,
};

enum class NavigationStopCondition{
    STOP_DIALOG,
    STOP_MARKER,
    STOP_TIME,
    STOP_BATTLE,
};

enum class NavigationMovementMode{
    DIRECTIONAL_ONLY,
    DIRECTIONAL_SPAM_A,
    CLEAR_WITH_LETS_GO,
};

struct AutoStoryOptions{
    Language language;
    StarterChoice starter_choice;
    EventNotificationOption& notif_status_update;
};

class AutoStory_Segment {
public:
    virtual ~AutoStory_Segment() = default;
    virtual std::string name() const = 0;
    virtual std::string start_text() const = 0;
    virtual std::string end_text() const = 0;
    virtual void run_segment(
        SingleSwitchProgramEnvironment& env, 
        ProControllerContext& context,
        AutoStoryOptions options,
        AutoStoryStats& stats) const = 0;
};

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

// press A to clear tutorial screens
// throw exception if tutorial screen never detected
void clear_tutorial(VideoStream& stream, ProControllerContext& context, uint16_t seconds_timeout = 5);

// spam the A button to clear dialog.
// stop depending on ClearDialogMode: stop when detect overworld, or dialog prompt, or A button prompt. Or if times out
// throw exception if times out, unless this is the intended stop condition.
// also throw exception if dialog is never detected.
void clear_dialog(VideoStream& stream, ProControllerContext& context,
    ClearDialogMode mode, uint16_t seconds_timeout = 60,
    std::vector<CallbackEnum> optional_callbacks = {}
);


// return true if the destination marker is present within the minimap area
bool confirm_marker_present(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context
);

// move character with ssf left joystick, as per given x, y, until 
// stop_condition is met (e.g. Dialog detected). 
// throw exception if reaches timeout before detecting stop condition
void overworld_navigation(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
    NavigationStopCondition stop_condition,
    NavigationMovementMode movement_mode,
    uint8_t x, uint8_t y,
    uint16_t seconds_timeout = 60, uint16_t seconds_realign = 60,
    bool auto_heal = false,
    bool detect_wipeout = false
);

void config_option(ProControllerContext& context, int change_option_value);

// enter menu and swap the first and third moves for your starter
void swap_starter_moves(SingleSwitchProgramEnvironment& env, ProControllerContext& context, Language language);

// run the given `action`. if detect a battle, stop the action, and throw exception
void do_action_and_monitor_for_battles(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context,
    std::function<
        void(const ProgramInfo& info, 
        VideoStream& stream,
        ProControllerContext& context)
    >&& action
);

// catch any UnexpectedBattle exceptions from `action`. then use run_battle_press_A until overworld, and re-try the `action`.
void handle_unexpected_battles(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context,
    std::function<
        void(const ProgramInfo& info, 
        VideoStream& stream,
        ProControllerContext& context)
    >&& action
);

// if stationary in overworld for an amount of time (seconds_stationary), run `recovery_action` then try `action` again
// return once successfully completed `action`
// throw exception if fails to complete `action` within a certain amount of time (minutes_timeout).
// NOTE: if using this function to wrap overworld_navigation(), keep in mind that 
// confirm_marker_present() will keep the player still for 5 seconds before moving. Therefore, seconds_stationary should be greater than 5 seconds in this case.
void handle_when_stationary_in_overworld(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context,
    std::function<
        void(const ProgramInfo& info, 
        VideoStream& stream,
        ProControllerContext& context)
    >&& action,
    std::function<
        void(const ProgramInfo& info, 
        VideoStream& stream,
        ProControllerContext& context)
    >&& recovery_action,
    size_t seconds_stationary = 6,
    uint16_t minutes_timeout = 5,
    size_t max_attempts = 2
);

// do action. if error is thrown, catch the error and try the recovery action
void handle_failed_action(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context,
    std::function<
        void(const ProgramInfo& info, 
        VideoStream& stream,
        ProControllerContext& context)
    >&& action,
    std::function<
        void(const ProgramInfo& info, 
        VideoStream& stream,
        ProControllerContext& context)
    >&& recovery_action,
    size_t max_failures
);

void wait_for_gradient_arrow(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context, 
    ImageFloatBox box_area_to_check,
    uint16_t seconds_timeout
);

void wait_for_overworld(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context, 
    uint16_t seconds_timeout = 30
);

void press_A_until_dialog(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    uint16_t seconds_between_button_presses
);

// return true if ride is active. i.e. if you are on your ride
bool is_ride_active(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

void get_on_ride(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

void get_off_ride(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

void get_on_or_off_ride(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context, bool get_on);


// change the settings prior to Autostory
// Assumes that `current_segment` represents where we currently are in the story.
void change_settings_prior_to_autostory(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    size_t current_segment_num,
    Language language
);

// from within the Settings/Options menu, change the settings
void change_settings(SingleSwitchProgramEnvironment& env, ProControllerContext& context, Language language, bool use_inference = true);


void checkpoint_save(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);

enum class ZoomChange{
    ZOOM_IN,
    ZOOM_IN_TWICE,
    ZOOM_OUT,
    ZOOM_OUT_TWICE,
    KEEP_ZOOM,
};

struct MoveCursor{
    ZoomChange zoom_change;
    uint8_t move_x;
    uint8_t move_y;
    uint16_t move_duration;
};

// place a marker on the map, not relative to the current player position, but based on a fixed landmark, such as a pokecenter
// How this works:
//  - cursor is moved to a point near the landmark, as per `move_cursor_near_landmark`
//  - move the cursor onto the landmark using `detect_closest_pokecenter_and_move_map_cursor_there`.
//  - confirm that the pokecenter is centered within cursor. If not, close map app, and re-try.
//  - cursor is moved to target location, as per `move_cursor_to_target`. A marker is placed down here.
void realign_player_from_landmark(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context,
    MoveCursor move_cursor_near_landmark,
    MoveCursor move_cursor_to_target
);

// confirm that the cursor is centered on the pokecenter, within the map app
// else throw exception
void confirm_cursor_centered_on_pokecenter(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

// open map, then move cursor to a point near a flypoint as per `move_cursor_near_flypoint`
// then fly to the closest pokecenter near the cursor
void move_cursor_towards_flypoint_and_go_there(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context,
    MoveCursor move_cursor_near_flypoint
);


void check_num_sunflora_found(SingleSwitchProgramEnvironment& env, ProControllerContext& context, int expected_number);

// run given action, with max_attempts number of attempts
// save prior to first attempt
// throw exception if we try to exceed max_attempts.
void checkpoint_reattempt_loop(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats,
    std::function<void(size_t attempt_number)>&& action
);

void checkpoint_reattempt_loop_tutorial(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats,
    std::function<void(size_t attempt_number)>&& action
);



}
}
}
#endif
