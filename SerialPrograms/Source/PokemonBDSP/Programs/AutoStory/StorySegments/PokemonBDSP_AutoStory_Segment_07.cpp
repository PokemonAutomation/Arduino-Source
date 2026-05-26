/*  BDSP AutoStory - Segment 07
 *  From after acquiring the sixth badge until after acquiring the seventh badge.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Globals.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/VisualDetectors/ImageMatchDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_MarkFinder.h"
#include "../PokemonBDSP_AutoStoryTools.h"
#include "../Utils/PokemonBDSP_AutoStory_Battle.h"
#include "../PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP_AutoStory_Segment_07.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


std::string AutoStory_Segment_07::name()       const{ return "07: Sixth Badge to Seventh Badge"; }
std::string AutoStory_Segment_07::start_text() const{ return "Start: Sixth Gym Badge acquired (Canalave City)."; }
std::string AutoStory_Segment_07::end_text()   const{ return "End: Seventh Gym Badge acquired (Snowpoint City)."; }

void AutoStory_Segment_07::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_020().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}


std::string AutoStory_Checkpoint_020::name()       const{ return "007 - " + AutoStory_Segment_07().name(); }
std::string AutoStory_Checkpoint_020::start_text() const{ return "Sixth Gym Badge acquired (Canalave City)."; }
std::string AutoStory_Checkpoint_020::end_text()   const{ return "Seventh Gym Badge acquired (Snowpoint City)."; }

void AutoStory_Checkpoint_020::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_020(env, context, options, stats);
}

static bool leave_gym(
    VideoStream& stream,
    ProControllerContext& context
){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("leave_gym: leaving the gym...");

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, -1}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("leave_gym: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("leave_gym: exited gym", COLOR_GREEN);
    };
    return true;
}

static bool go_to_the_library(
    VideoStream& stream,
    ProControllerContext& context
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_to_the_library: talking to Clint and going to the Library...");

    {
        ShortDialogWatcher talk_to_clint(COLOR_BLUE);
        int ret = wait_until(stream, context, std::chrono::seconds(30), {{talk_to_clint}});
        if (ret < 0){
            stream.log("go_to_the_library_talk_to_clint: dialog box not detected!", COLOR_RED);
            return false;
        } else if (ret == 0) {
            stream.log("go_to_the_library_talk_to_clint: dialog box detected.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 7000ms);
            context.wait_for_all_requests();
        }
    }

    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 2);
    pbf_move_left_joystick(context, {-1, 0}, 1600ms, 100ms); // 8+
    pbf_move_left_joystick(context, {0, +1}, 3800ms, 100ms); // 19+
    pbf_move_left_joystick(context, {+1, 0}, 600ms, 100ms); // 2+
    pbf_move_left_joystick(context, {0, -1}, 1000ms, 100ms); // 4+
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 3);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("go_to_the_library: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_to_the_library: entered the library", COLOR_GREEN);
    };
    return true;
}

static bool drama_at_the_library(
    VideoStream& stream,
    ProControllerContext& context
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("drama_at_the_library: going to professor Rowan...");

    pbf_move_left_joystick(context, {0, +1}, 1400ms, 100ms); // 7+

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("drama_at_the_library: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("drama_at_the_library: moved to the first floor", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    pbf_move_left_joystick(context, {0, -1}, 600ms, 100ms); // 3+
    pbf_move_left_joystick(context, {+1, 0}, 1400ms, 100ms); // 7
    pbf_move_left_joystick(context, {0, +1}, 800ms, 100ms); // 3+

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("drama_at_the_library: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("drama_at_the_library: moved to the second floor", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    {
        ShortDialogWatcher talk_to_clint(COLOR_BLUE);
        int ret = wait_until(stream, context, std::chrono::seconds(30), {{talk_to_clint}});
        if (ret < 0){
            stream.log("drama_at_the_library_talk_to_clint: dialog box not detected!", COLOR_RED);
            return false;
        } else if (ret == 0) {
            stream.log("drama_at_the_library_talk_to_clint: dialog box detected.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 45000ms);
            context.wait_for_all_requests();
        }
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {0, +1}, 600ms, 100ms); // 2+
    pbf_move_left_joystick(context, {-1, 0}, 700ms, 100ms); // 5
    pbf_move_left_joystick(context, {0, +1}, 600ms, 100ms); // 2+

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("drama_at_the_library: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("drama_at_the_library: moved back to the first floor", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 2);
    pbf_move_left_joystick(context, {-1, 0}, 1400ms, 100ms); // 7
    pbf_move_left_joystick(context, {0, +1}, 800ms, 100ms); // 3+

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("drama_at_the_library: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("drama_at_the_library: moved back to the ground floor", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, -1}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("drama_at_the_library: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("drama_at_the_library: moved out of the library", COLOR_GREEN);
    };
    return true;
}

static bool talk_fly_and_enter_mount_coronet(
    VideoStream& stream,
    ProControllerContext& context
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("talk_fly_and_enter_mount_coronet: talking, flying and becoming a speleologist...");

    {
        ShortDialogWatcher talk_to_rowan(COLOR_BLUE);
        int ret = wait_until(stream, context, std::chrono::seconds(30), {{talk_to_rowan}});
        if (ret < 0){
            stream.log("talk_fly_and_enter_mount_coronet_talk_to_rowan: dialog box not detected!", COLOR_RED);
            return false;
        } else if (ret == 0) {
            stream.log("talk_fly_and_enter_mount_coronet_talk_to_rowan: dialog box detected.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_B, 21000ms);
            context.wait_for_all_requests();
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    fly_to(stream, context, FlyPoint::CelesticTown);

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    stream.log("talk_fly_and_enter_mount_coronet: going towards Mt. Coronet...");

    pbf_move_left_joystick(context, {-1, 0}, 2000ms, 100ms); // 10+
    pbf_move_left_joystick(context, {0, -1}, 800ms, 100ms); // 3+
    pbf_move_left_joystick(context, {-1, 0}, 1800ms, 100ms); // 9+
    pbf_move_left_joystick(context, {0, +1}, 800ms, 100ms); // 3+
    pbf_move_left_joystick(context, {-1, 0}, 600ms, 100ms); // 2+

    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 11);
    pbf_move_left_joystick(context, {-1, 0}, 3200ms, 100ms); // 16+
    context.wait_for_all_requests();
    activate_repel(stream, context);
    context.wait_for_all_requests();

    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 3);
    pbf_move_left_joystick(context, {-1, 0}, 2400ms, 100ms); // 12+
    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 4);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("talk_fly_and_enter_mount_coronet: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("talk_fly_and_enter_mount_coronet: entered Mt. Coronet", COLOR_GREEN);
    };
    return true;
}

static bool go_through_mount_coronet(
    VideoStream& stream,
    ProControllerContext& context
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_through_mount_coronet: going through Mt. Coronet...");

    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 4);
    pbf_move_left_joystick(context, {+1, 0}, 400ms, 100ms); // 1+
    pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms); // 1+

    use_strength(stream, context);

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, +1}, 4000ms, 100ms); // 10++ (strength)
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);
    pbf_move_left_joystick(context, {0, +1}, 2400ms, 100ms); // 12+

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, -1}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("go_through_mount_coronet: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_mount_coronet: finished first part of Mt. Coronet", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    stream.log("go_through_mount_coronet: continuing through Mt. Coronet...");

    pbf_move_left_joystick(context, {+1, 0}, 1000ms, 100ms); // 4+
    pbf_move_left_joystick(context, {0, +1}, 600ms, 100ms); // 2+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);
    pbf_move_left_joystick(context, {0, +1}, 11000ms, 100ms); // 55+

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("go_through_mount_coronet: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_mount_coronet: finished second part of Mt. Coronet", COLOR_GREEN);
    };

    pbf_move_left_joystick(context, {+1, 0}, 1200ms, 100ms); // 6+
    pbf_move_left_joystick(context, {0, +1}, 1600ms, 100ms); // 8+

    return true;
}

void checkpoint_020(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            /*if (!leave_gym(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "leave_gym: transition not detected.", env.console);
            }
            if (!go_to_the_library(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_to_the_library: transition not detected.", env.console);
            }
            if (!drama_at_the_library(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "drama_at_the_library: transition not detected.", env.console);
            }
            if (!talk_fly_and_enter_mount_coronet(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "talk_fly_and_enter_mount_coronet: transition not detected.", env.console);
            }*/
            if (!go_through_mount_coronet(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_through_mount_coronet: transition not detected.", env.console);
            }
        }
    );
}


}
}
}
