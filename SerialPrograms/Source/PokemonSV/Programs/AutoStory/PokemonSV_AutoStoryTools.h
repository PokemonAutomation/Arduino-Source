/*  AutostoryTools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoStoryTools_H
#define PokemonAutomation_PokemonSV_AutoStoryTools_H

#include <functional>
#include "ML/Inference/ML_YOLOv5Detector.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace ML;

static constexpr bool ENABLE_TEST  = false;

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



enum class ClearDialogMode{
    STOP_OVERWORLD,
    STOP_PROMPT,
    STOP_WHITEBUTTON,
    STOP_TIMEOUT,
    STOP_BATTLE,
    STOP_TUTORIAL,
    STOP_BATTLE_DIALOG_ARROW,
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



enum class NavigationStopCondition{
    STOP_DIALOG,
    STOP_MARKER,
    STOP_TIME,
    STOP_BATTLE,
};

// struct MinimumDetectedSize{
//     double width;
//     double height;
// };


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

class AutoStory_Checkpoint {
public:
    virtual ~AutoStory_Checkpoint() = default;
    virtual std::string name() const = 0;
    virtual std::string start_text() const = 0;
    virtual std::string end_text() const = 0;
    virtual void run_checkpoint(
        SingleSwitchProgramEnvironment& env, 
        ProControllerContext& context,
        AutoStoryOptions options,
        AutoStoryStats& stats) const = 0;
};


// press A to clear tutorial screens
// throw exception if tutorial screen never detected
void clear_tutorial(VideoStream& stream, ProControllerContext& context, uint16_t seconds_timeout = 5);

// spam the A button to clear dialog.
// stop depending on ClearDialogMode: stop when detect overworld, or dialog prompt, or A button prompt. Or if times out
// throw exception if times out, unless this is the intended stop condition.
// also throw exception if dialog is never detected.
// NOTE: seconds_timeout is rounded up to a multiple of 25, unless press_A is false or ClearDialogMode == STOP_TIMEOUT
void clear_dialog(VideoStream& stream, ProControllerContext& context,
    ClearDialogMode mode, uint16_t seconds_timeout = 75,
    std::vector<CallbackEnum> optional_callbacks = {},
    bool press_A = true
);


// return true if the destination marker is present within the minimap area
bool confirm_marker_present(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context
);

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
void realign_player(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    PlayerRealignMode realign_mode,
    uint8_t move_x = 0, uint8_t move_y = 0,
    Milliseconds move_duration = Milliseconds(0)
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

// confirm the moves for the Lead pokemon: Moonblast, Mystical Fire, Psychic, Misty Terrain
// start and end in the overworld
void confirm_lead_pokemon_moves(SingleSwitchProgramEnvironment& env, ProControllerContext& context, Language language);

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

// run the given `action`. if detect no minimap, stop the action. 
// wait 15 seconds and see if we find a battle. if so, throw Battle exception. if no battle, then throw OperationFailedException
void do_action_and_monitor_for_battles_early(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context,
    std::function<
        void(const ProgramInfo& info, 
        VideoStream& stream,
        ProControllerContext& context)
    >&& action
);

void do_action_until_dialog(
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

// run the given `action`. if detect the overworld, stop the action, and throw exception
void do_action_and_monitor_for_overworld(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context,
    std::function<void(const ProgramInfo& info, 
        VideoStream& stream,
        ProControllerContext& context)>&& action
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

void change_settings_prior_to_autostory_segment_mode(SingleSwitchProgramEnvironment& env, ProControllerContext& context, size_t current_segment_num, Language language);
void change_settings_prior_to_autostory_checkpoint_mode(SingleSwitchProgramEnvironment& env, ProControllerContext& context, size_t current_checkpoint_num, Language language);

// change the settings prior to Autostory
// Assumes that `current_segment` represents where we currently are in the story.
void change_settings_prior_to_autostory(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    int options_index,
    bool has_minimap,
    Language language
);

// from within the Settings/Options menu, change the settings
void change_settings(SingleSwitchProgramEnvironment& env, ProControllerContext& context, Language language, bool use_inference = true);


void checkpoint_save(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);


// place a marker on the map, not relative to the current player position, but based on a fixed landmark, such as a pokecenter
// How this works:
//  - cursor is moved to a point near the landmark, as per `move_cursor_near_landmark`
//  - move the cursor onto the landmark using `detect_closest_flypoint_and_move_map_cursor_there`.
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
    MoveCursor move_cursor_near_flypoint, 
    FlyPoint fly_point = FlyPoint::POKECENTER
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
    std::function<void(size_t attempt_number)>&& action,
    bool day_skip = true
);

void checkpoint_reattempt_loop_tutorial(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats,
    std::function<void(size_t attempt_number)>&& action
);


// walk forward forward_ticks. repeat this for num_rounds.
// if detect battle, kill the Pokemon. then continue. If we run into a battle, this round is considered to be done and will not be repeated.
// NOTE: mashing A and Let's go aren't compatible. you end up talking to your Let's go pokemon if you mash A.
void move_player_forward(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    uint8_t num_rounds, 
    std::function<void()>&& recovery_action,
    bool use_lets_go = false,
    bool mash_A = false,
    Milliseconds forward_duration = Milliseconds(800), 
    uint8_t y = 0, 
    Milliseconds delay_after_forward_move = Milliseconds(400), 
    Milliseconds delay_after_lets_go = Milliseconds(840)
);

// get the box of the target object
// if multiple objects have the same label, choose the one with the highest score
// return ImageFloatBox{-1, -1, -1, -1} if target object not found
ImageFloatBox get_yolo_box(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    VideoOverlaySet& overlays,
    YOLOv5Detector& yolo_detector, 
    const std::string& target_label
);

// move forward until detected object is a certain width and height on screen (min_size)
// walk forward forward_ticks each time
// if caught in battle, run recovery_action
// throw exception if forward_move_count is above 50.
// throw exception if never detected yolo object
void move_forward_until_yolo_object_above_min_size(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    YOLOv5Detector& yolo_detector, 
    const std::string& target_label,
    double min_width, double min_height,
    std::function<void()>&& recovery_action, 
    uint16_t forward_ticks = 100, 
    uint8_t y = 0, 
    uint16_t delay_after_forward_move = 50, 
    uint16_t delay_after_lets_go = 105
);

// walk forward forward_ticks each time
// walk until we find the target object.
// if caught in battle, run recovery_action
// throw exception if exceed max_rounds.
void move_player_until_yolo_object_detected(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    YOLOv5Detector& yolo_detector, 
    const std::string& target_label,
    std::function<void()>&& recovery_action, 
    uint16_t max_rounds, 
    uint16_t forward_ticks = 100, 
    uint8_t x = 128, 
    uint8_t y = 0, 
    uint16_t delay_after_forward_move = 50, 
    uint16_t delay_after_lets_go = 105
);

// walk forward forward_ticks each time
// walk until we no longer see the target object. and haven't seen `it times_not_seen_threshold` times.
// if caught in battle, run recovery_action
// throw exception if exceed max_rounds.
void move_forward_until_yolo_object_not_detected(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    YOLOv5Detector& yolo_detector, 
    const std::string& target_label,
    size_t times_not_seen_threshold,
    std::function<void()>&& recovery_action, 
    uint16_t forward_ticks = 100, 
    uint8_t y = 0, 
    uint16_t delay_after_forward_move = 50, 
    uint16_t delay_after_lets_go = 105
);

enum class CameraAxis{
    X,
    Y,
};

// move the camera along `axis` until the target object is aligned with target_line
// if caught in battle, run recovery_action
// throw exception if never detected yolo object
void move_camera_yolo(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    CameraAxis axis,
    YOLOv5Detector& yolo_detector, 
    const std::string& target_label,
    double target_line,
    std::function<void()>&& recovery_action
);

// move the player sideways until the target object is aligned with x_target
// throw exception if never detected yolo object
bool move_player_to_realign_via_yolo(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    YOLOv5Detector& yolo_detector, 
    const std::string& target_label,
    double x_target
);


// move the player sideways until the target object is aligned with x_target
// if failed. try recovery action once, then re-try.
void move_player_to_realign_via_yolo_with_recovery(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    YOLOv5Detector& yolo_detector, 
    const std::string& target_label,
    double x_target,
    std::function<void()>&& recovery_action
);

// move camera in X direction until we find the target object.
// if caught in battle, run recovery_action
// throw exception if exceed max_rounds.
void move_camera_until_yolo_object_detected(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    YOLOv5Detector& yolo_detector, 
    const std::string& target_label,
    uint8_t initial_x_move,
    Milliseconds initial_hold,
    uint16_t max_rounds = 50
);


void confirm_titan_battle(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

}
}
}
#endif
