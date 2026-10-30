/*  BDSP AutoStory - Segment 00
 *  From player's bedroom to pokedex acquisition.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <cmath>

#include <opencv2/imgproc.hpp>

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"

#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/VisualDetectors/ImageMatchDetector.h"

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManip.h"

#include "Pokemon/Pokemon_Notification.h"
#include "Pokemon/Pokemon_Strings.h"

#include "PokemonBDSP/Programs/AutoStory/PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP/Programs/AutoStory/Utils/PokemonBDSP_AutoStory_Segment_00_tools.h"
#include "PokemonBDSP/Programs/AutoStory/Utils/PokemonBDSP_OverworldMatchers.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameNavigation.h"
#include "PokemonBDSP/Programs/ShinyHunting/PokemonBDSP_StarterReset.h"

#include "PokemonBDSP_AutoStory_Segment_00.h"

using namespace std::chrono_literals;

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonBDSP {
// ---------------------------------------------------------------------------
// Starter selection
// ---------------------------------------------------------------------------

// Watch before movement starts, release on the fade, and hold the
// following recorded wait until the destination is visible again.
static void recorded_move_until_transition(VideoStream& stream,
    ProControllerContext& context,
    double x,
    double y,
    Milliseconds duration,
    const std::string& label){
    context.wait_for_all_requests();
    stream.log("[Tutorial transition] " + label + ": approaching fade.");
    BlackScreenWatcher black;
    int ret = run_until<ProControllerContext>(stream,
        context,
        [x, y, duration](ProControllerContext& ctx){
            pbf_move_left_joystick(ctx, {x, y}, duration, 0ms);
            // Keep observing a delayed fade after the recorded hold.
            pbf_wait(ctx, 5000ms);
        },
        {{black}});
    if (ret != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            label + ": Black transition not detected.",
            stream);
    }
    BlackScreenWatcher visible(COLOR_RED, {0.1, 0.1, 0.8, 0.8}, 100, 10, BlackScreenWatcher::FinderType::GONE, 500ms);
    if (wait_until(stream, context, 15s, {{visible}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            label + ": Transition did not finish.",
            stream);
    }
    stream.log("[Tutorial transition] " + label + ": destination visible.");
}

static void select_turtwig(VideoStream& stream, ProControllerContext& context){
    stream.log("[DEBUG] Turtwig Selection: Starting starter selection");

    pbf_press_button(context, BUTTON_ZL, 160ms, 240ms);
    context.wait_for_all_requests();

    ShortDialogWatcher watcher(COLOR_CYAN);
    int ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& ctx){
            ctx.wait_for(std::chrono::milliseconds(5000));
        },
        {watcher});
    if (ret != 0){
        ShortDialogWatcher watcher_red(COLOR_RED);
        ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& ctx){
                ctx.wait_for(std::chrono::milliseconds(5000));
            },
            {watcher_red});
    }
    stream.log(ret == 0 ? "[DEBUG] Turtwig Selection: Confirmation dialogue detected"
        : "[WARNING] Turtwig Selection: Dialogue detection failed, proceeding anyway");

    pbf_wait(context, 400ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 400ms);
    pbf_press_button(context, BUTTON_ZL, 80ms, 5000ms);
    context.wait_for_all_requests();
    stream.log("[DEBUG] Turtwig Selection: Selection complete");
}

static void select_chimchar(VideoStream& stream, ProControllerContext& context){
    stream.log("[DEBUG] Chimchar Selection: Starting starter selection");

    pbf_press_dpad(context, DPAD_RIGHT, 160ms, 840ms);
    pbf_press_button(context, BUTTON_ZL, 160ms, 240ms);
    context.wait_for_all_requests();

    ShortDialogWatcher watcher(COLOR_CYAN);
    int ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& ctx){
            ctx.wait_for(std::chrono::milliseconds(5000));
        },
        {watcher});
    if (ret != 0){
        ShortDialogWatcher watcher_red(COLOR_RED);
        ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& ctx){
                ctx.wait_for(std::chrono::milliseconds(5000));
            },
            {watcher_red});
    }
    stream.log(ret == 0 ? "[DEBUG] Chimchar Selection: Confirmation dialogue detected"
        : "[WARNING] Chimchar Selection: Dialogue detection failed, proceeding anyway");

    pbf_wait(context, 400ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 400ms);
    pbf_press_button(context, BUTTON_ZL, 80ms, 5000ms);
    context.wait_for_all_requests();
    stream.log("[DEBUG] Chimchar Selection: Selection complete");
}

static void select_piplup(VideoStream& stream, ProControllerContext& context){
    stream.log("[DEBUG] Piplup Selection: Starting starter selection");

    pbf_press_dpad(context, DPAD_RIGHT, 160ms, 840ms);
    pbf_press_dpad(context, DPAD_RIGHT, 160ms, 840ms);
    pbf_press_button(context, BUTTON_ZL, 160ms, 240ms);
    context.wait_for_all_requests();

    ShortDialogWatcher watcher(COLOR_CYAN);
    int ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& ctx){
            ctx.wait_for(std::chrono::milliseconds(5000));
        },
        {watcher});
    if (ret != 0){
        ShortDialogWatcher watcher_red(COLOR_RED);
        ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& ctx){
                ctx.wait_for(std::chrono::milliseconds(5000));
            },
            {watcher_red});
    }
    stream.log(ret == 0 ? "[DEBUG] Piplup Selection: Confirmation dialogue detected"
        : "[WARNING] Piplup Selection: Dialogue detection failed, proceeding anyway");

    pbf_wait(context, 400ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 400ms);
    pbf_press_button(context, BUTTON_ZL, 80ms, 5000ms);
    context.wait_for_all_requests();
    stream.log("[DEBUG] Piplup Selection: Selection complete");
}

// Navigation functions for each part of the tutorial. These functions are meant to be used
// in order and rely on the state of the game being what it is at the end of the previous function.
// They may also be modified or removed without warning.

// ---------------------------------------------------------------------------
// Segment 00-01 begins
// Navigates from player's room to Route 201 through Mom/Barry dialogues.
// ---------------------------------------------------------------------------

static void
tutorial_part_1_navigation(SingleSwitchProgramEnvironment& env, VideoStream& stream, ProControllerContext& context){
    DateTime t{2026, 1, 1, 0, 0, 0};
    set_time_fast(env, context, t);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 527ms, 0ms);
    pbf_move_left_joystick(context, {1.000000, 1.000000}, 456ms, 0ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 915ms, 0ms);
    pbf_move_left_joystick(context, {1.000000, 1.000000}, 642ms, 0ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 557ms, 0ms);
    pbf_wait(context, 978ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 1112ms, 0ms);
    pbf_wait(context, 4967ms);

    wait_for_dialogue(stream, context, "Phase 1 (Mom)");

    pbf_mash_button(context, BUTTON_A, 5000ms);
    pbf_wait(context, 300ms);

    pbf_move_left_joystick(context, {0.000000, -1.000000}, 1662ms, 0ms);
    pbf_move_left_joystick(context, {-1.000000, -1.000000}, 49ms, 0ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1215ms, 0ms);
    pbf_wait(context, 1419ms);

    wait_for_dialogue(stream, context, "Phase 2 (Mom Warning)");

    pbf_mash_button(context, BUTTON_A, 2500ms);
    pbf_wait(context, 300ms);
    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {0.000000, -1.000000}, 680ms, 0ms);

    pbf_wait(context, 4000ms);

    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1748ms, 0ms);
    pbf_move_left_joystick(context, {-1.000000, 1.000000}, 18ms, 0ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 2763ms, 0ms);
    pbf_move_left_joystick(context, {-1.000000, 1.000000}, 58ms, 0ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1364ms, 0ms);
    pbf_wait(context, 4000ms);
    context.wait_for_all_requests();

    wait_for_dialogue(stream, context, "Phase 3 (Barry 1st)");
    pbf_mash_button(context, BUTTON_A, 6000ms);
    pbf_wait(context, 800ms);
    wait_for_dialogue(stream, context, "Phase 3 (Barry 1st)");
    pbf_mash_button(context, BUTTON_A, 6000ms);
    pbf_wait(context, 800ms);

    pbf_wait(context, 1306ms);
    context.wait_for_all_requests();
    stream.log("[Barry house] Entering; waiting for entry fade.");
    BlackScreenWatcher entry_black;
    int entry_ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& ctx){
            pbf_move_left_joystick(ctx, {0, 1}, 5000ms, 0ms);
        },
        {{entry_black}});
    if (entry_ret != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Barry house: Entry fade not detected.",
            stream);
    }
    // Release the joystick during loading; resume indoor movement only
    // after the screen has stayed non-black for half a second.
    BlackScreenWatcher entry_visible(COLOR_RED,
        {0.1, 0.1, 0.8, 0.8},
        100,
        10,
        BlackScreenWatcher::FinderType::GONE,
        500ms);
    if (wait_until(stream, context, 15s, {{entry_visible}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Barry house: Entry fade did not finish.",
            stream);
    }
    pbf_wait(context, 500ms);
    context.wait_for_all_requests();
    stream.log("[Barry house] Entry settled; starting stair approach.");
    // Align left after entry before the next upward stair approach.
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 400ms, 0ms);
    pbf_move_left_joystick(context, {-1.000000, 0.000000}, 1600ms, 0ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 2722ms, 0ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1600ms, 0ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 501ms, 0ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 652ms, 0ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 1817ms, 0ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 975ms, 0ms);
    pbf_wait(context, 1422ms);

    wait_for_dialogue(stream, context, "Phase 8 (Barry 2nd dialog)");
    pbf_mash_button(context, BUTTON_A, 10000ms);

    pbf_wait(context, 856ms);


    // Recorded route from the top of Barry's stairs. Moves rounded to
    // 100ms, waits to 50ms; transition waits gate both floor changes.
    recorded_move_until_transition(stream, context, -1.0, 0.0, 900ms, "Barry house downstairs");
    pbf_wait(context, 750ms);
    pbf_move_left_joystick(context, {-1.0, 0.0}, 2000ms, 0ms);
    pbf_wait(context, 250ms);
    pbf_move_left_joystick(context, {0.0, -1.0}, 2000ms, 0ms);
    pbf_wait(context, 150ms);
    pbf_move_left_joystick(context, {1.0, 0.0}, 1400ms, 0ms);
    pbf_wait(context, 500ms);
    recorded_move_until_transition(stream, context, 0.0, -1.0, 1200ms, "Barry house exit");
    // First-run video placed the player east of the northbound path.
    // Shorten the rightward approach; tune further from the next recording.
    stream.log("[Route 201] Aligning east for 2300ms before northbound approach.");
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 2300ms, 0ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 5861ms, 0ms);
    pbf_wait(context, 134ms);

    wait_for_dialogue(stream, context, "Phase 9 (Barry Route 201)");

    pbf_mash_button(context, BUTTON_B, 6000ms);
    pbf_wait(context, 800ms);

    pbf_move_left_joystick(context, {0.000000, 1.000000}, 700ms, 0ms);
    pbf_move_left_joystick(context, {-1.000000, 1.000000}, 279ms, 0ms);
    pbf_move_left_joystick(context, {-1.000000, 0.000000}, 10ms, 0ms);
    pbf_move_left_joystick(context, {-1.000000, 1.000000}, 145ms, 0ms);
    pbf_move_left_joystick(context, {-1.000000, 0.000000}, 7703ms, 0ms);
    pbf_move_left_joystick(context, {-1.000000, 1.000000}, 140ms, 0ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 1700ms, 0ms);
    pbf_move_left_joystick(context, {-1.000000, 1.000000}, 39ms, 0ms);
    pbf_move_left_joystick(context, {-1.000000, 0.000000}, 544ms, 1000ms);

    pbf_wait(context, 1000ms);

    // save_game(stream, context);  // Disabled: checkpoint save mode controls saving.
    context.wait_for_all_requests();
}
// ---------------------------------------------------------------------------
// End of Segment 00 - 01
// Standing in front lake varity ready to enter lake Verity for starter selection.
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//  Segment 00-02 begins
//  Navigates into lake Verity to recieve starter
// ---------------------------------------------------------------------------
static void starter_nav(VideoStream& stream, ProControllerContext& context){
    // re-sync control
    pbf_press_button(context, BUTTON_B, 20ms, 200ms);
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();

    // continue movement
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 300ms, 0ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 300ms, 0ms);

    wait_for_dialogue(stream, context, "Phase 10 (Barry before the starter selection)");

    pbf_mash_button(context, BUTTON_A, 1000ms);

    pbf_wait(context, 500ms);
    std::shared_ptr<const ImageRGB32> briefcase =
        std::make_shared<const ImageRGB32>(RESOURCE_PATH() + "PokemonBDSP/StarterBriefcase.png");

    //  Mash B until we see the briefcase.
    ImageMatchWatcher detector(briefcase, {0.5, 0.1, 0.5, 0.7}, 100, true);

    int ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_B, 120000ms);
        },
        {{detector}});

    if (ret == 0){
        stream.log("Detected briefcase!");
    }else{
        stream.log("Timed out waiting for briefcase.");
        throw OperationFailedExceptionWithScreenshot(ErrorReportMode::SEND_ERROR_REPORT,
            "Failed to detect briefcase in Segment 00.",
            stream);
    }

    //  Wait for briefcase to fully open.
    stream.log("Mashing B for briefcase to fully open.");
    pbf_mash_button(context, BUTTON_B, 2000ms);
}
static void post_starly_clenaup(VideoStream& stream, ProControllerContext& context){
    pbf_mash_button(context, BUTTON_B, 60000ms);
}
// ---------------------------------------------------------------------------
// End of Segment 00 - 02
// Standing in front lake varity after starter selection.
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//  Segment 00-03 begins
//  Navigates from Exit of lake Verity to receipt of the Pokedex
// ---------------------------------------------------------------------------

static void
tutorial_part_2_navigation(SingleSwitchProgramEnvironment& env, VideoStream& stream, ProControllerContext& context){
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();

    pbf_wait(context, 563ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 1700ms, 0ms);
    pbf_move_left_joystick(context, {1.000000, -1.000000}, 510ms, 0ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 50ms, 0ms);
    pbf_move_left_joystick(context, {1.000000, -1.000000}, 310ms, 0ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 5500ms, 0ms);
    pbf_wait(context, 3963ms);
    context.wait_for_all_requests();

    wait_for_dialogue(stream, context, "Phase 10 (Professor dialog to mom)");
    pbf_mash_button(context, BUTTON_B, 40000ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 820ms, 0ms);
    pbf_wait(context, 4000ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 800ms, 0ms);
    pbf_wait(context, 555ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 50ms, 0ms);
    pbf_wait(context, 23ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 5020ms, 0ms);
    pbf_wait(context, 169ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 745ms, 0ms);
    DateTime t{2026, 1, 1, 0, 0, 50};
    set_time_fast(env, context, t);
    walk_through_grass(stream, context, 1, 0, 20, "right through grass to path", BattleType::STARTER);
    walk_through_grass(stream, context, 0, 1, 18, "up through grass 2", BattleType::STARTER);
    walk_through_grass(stream, context, 1, 0, 10, "right through grass 2", BattleType::STARTER);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 730ms, 0ms);
    pbf_wait(context, 941ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 1950ms, 0ms);
    wait_for_dialogue(stream, context, "Phase 11 (dawn conversation)");
    pbf_mash_button(context, BUTTON_B, 50000ms);
    wait_for_dialogue(stream, context, "Phase 12 (Nickname reject)");
    pbf_mash_button(context, BUTTON_B, 40000ms);
    wait_for_dialogue(stream, context, "Phase 13 (Pokedex accept)");
    pbf_mash_button(context, BUTTON_A, 40000ms);


    // pokedex received
}


// ---------------------------------------------------------------------------
// End of Segment 00 - 03
// Standing in front of the professor after receiving the pokedex.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Begin Segment 00 - 04
// Will heal at Sandgem pokeCenter to reliably catch a second pokemon for Home
// transfer.
// Collects poketech and stands in front of the PC to end the tutorial section
// and send the starter to pokemon HOME.
// ---------------------------------------------------------------------------

static void
tutorial_part_3_navigation(SingleSwitchProgramEnvironment& env, VideoStream& stream, ProControllerContext& context){
    // Arm exit detection before movement and finish the fade before
    // starting the dialogue watcher, which cancels queued commands.
    recorded_move_until_transition(stream, context, 0.0, -1.0, 2010ms, "Rowan lab exit");
    context.wait_for_all_requests();

    wait_for_dialogue(stream, context, "Phase 14 (dawn tour)");

    pbf_mash_button(context, BUTTON_B, 25000ms);

    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 5125ms, 0ms);
    pbf_wait(context, 336ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 1700ms, 0ms);
    pbf_wait(context, 213ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 2325ms, 0ms);
    pbf_move_left_joystick(context, {-1.000000, 1.000000}, 950ms, 0ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 2360ms, 0ms);
    pbf_wait(context, 253ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 4610ms, 0ms);
    pbf_wait(context, 954ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 220ms, 0ms);
    pbf_wait(context, 587ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 1375ms, 0ms);
    pbf_wait(context, 464ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 775ms, 0ms);
    pbf_wait(context, 1655ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 640ms, 0ms);
    pbf_wait(context, 3128ms);
    pbf_press_dpad(context, DPAD_UP, 180ms, 0ms);
    pbf_wait(context, 472ms);
    pbf_press_dpad(context, DPAD_UP, 130ms, 0ms);
    pbf_wait(context, 1621ms);
    pbf_press_dpad(context, DPAD_RIGHT, 130ms, 0ms);
    pbf_press_button(context, BUTTON_A, 180ms, 0ms);

    wait_for_dialogue(stream, context, "Phase 15 (Mom conversation to accept parcel)");

    pbf_mash_button(context, BUTTON_B, 40000ms);

    context.wait_for_all_requests();
    pbf_wait(context, 1500ms);
    pbf_press_dpad(context, DPAD_LEFT, 150ms, 0ms);
    pbf_wait(context, 1143ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 875ms, 0ms);
    pbf_wait(context, 1169ms);
    pbf_press_button(context, BUTTON_A, 180ms, 0ms);
    pbf_wait(context, 946ms);
    pbf_press_button(context, BUTTON_A, 180ms, 0ms);
    pbf_wait(context, 9092ms);
    pbf_press_button(context, BUTTON_A, 180ms, 0ms);
    pbf_wait(context, 740ms);
    pbf_press_button(context, BUTTON_A, 180ms, 0ms);
    pbf_wait(context, 2998ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 240ms, 0ms);
    pbf_wait(context, 893ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 5300ms, 0ms);
    pbf_wait(context, 282ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 260ms, 0ms);
    context.wait_for_all_requests();
    pbf_wait(context, 1500ms);

    DateTime t{2026, 1, 1, 0, 0, 50};
    set_time_fast(env, context, t);
    walk_through_grass(stream, context, 1, 0, 20, "right through grass to path", BattleType::STARTER);
    walk_through_grass(stream, context, 0, 1, 18, "up through grass 2", BattleType::STARTER);
    walk_through_grass(stream, context, 1, 0, 10, "right through grass 2", BattleType::STARTER);
    context.wait_for_all_requests();
    pbf_wait(context, 1500ms);

    pbf_move_left_joystick(context, {0.000000, -1.000000}, 560ms, 0ms);
    pbf_wait(context, 1500ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 5000ms, 0ms);
    pbf_wait(context, 1500ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1000ms, 0ms);
    pbf_wait(context, 1500ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 1855ms, 0ms);
    pbf_wait(context, 1500ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 430ms, 0ms);
    pbf_wait(context, 1500ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 850ms, 0ms);
    pbf_wait(context, 1500ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1120ms, 0ms);

    wait_for_dialogue(stream, context, "Phase 16 (catch tutorial)");

    pbf_mash_button(context, BUTTON_B, 70000ms);
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    catch_1_pokemon(stream, context);
    pbf_wait(context, 1500ms);
    context.wait_for_all_requests();
}

// Align to a route reference before continuing recorded navigation.
// Matcher offsets and thresholds are fractions of screen width/height.
static void align_to_tutorial_landmarks(VideoStream& stream,
    ProControllerContext& context,
    OverworldMatchResult (*matcher)(const ImageRGB32&),
    const std::string& label){
    context.wait_for_all_requests();
    pbf_wait(context, 700ms);
    context.wait_for_all_requests();
    size_t confirmations = 0, coarse_confirmations = 0;
    double previous_dx = 0, previous_dy = 0;
    for (size_t attempt = 0; attempt < 60; attempt++){
        const auto frame = stream.video().snapshot();
        if (!frame){
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                label + ": No video frame.",
                stream);
        }
        const auto result = matcher(*frame.frame);
        VideoOverlaySet overlays(stream.overlay());
        for (const auto& landmark : result.landmarks){
            overlays.add(COLOR_CYAN, landmark.search);
            if (landmark.accepted){
                overlays.add(COLOR_GREEN, landmark.found);
            }
            stream.log("[" + label + "] " + landmark.name + " score=" + std::to_string(landmark.score) +
                " margin=" + std::to_string(landmark.margin) + " accepted=" + std::to_string(landmark.accepted) +
                " screen-relative offset=(" + std::to_string(landmark.dx) + "," + std::to_string(landmark.dy) + ")");
        }
        stream.log("[" + label + "] located=" + std::to_string(result.located) +
            " mode=" + (result.coarse ? "vertical acquisition" : "fine alignment") +
            " reference screen-relative offset=(" + std::to_string(result.dx) + "," + std::to_string(result.dy) + ")");
        if (result.coarse && result.located){
            if (coarse_confirmations && std::abs(result.dx - previous_dx) <= 0.00625 &&
                std::abs(result.dy - previous_dy) <= 1.0 / 90){
                coarse_confirmations++;
            }else{
                coarse_confirmations = 1;
            }
            previous_dx = result.dx;
            previous_dy = result.dy;
        }else{
            coarse_confirmations = 0;
        }
        if (!result.located){
            confirmations = 0;
            pbf_wait(context, 250ms);
        }else if (result.coarse && coarse_confirmations < 3){
            confirmations = 0;
            stream.log("[" + label + "] Waiting for consistent vertical acquisition.");
            pbf_wait(context, 250ms);
        }else if (result.at_target){
            if (++confirmations >= 3){
                stream.log("[" + label + "] Reference position confirmed.");
                return;
            }
            pbf_wait(context, 300ms);
        }else{
            confirmations = 0;
            const bool horizontal = !result.coarse && std::abs(result.dx) >= std::abs(result.dy) * 9.0 / 16;
            const double error = horizontal ? result.dx : result.dy * 9.0 / 16;
            const auto duration = std::abs(error) > 1.0 / 24 ? 80ms : std::abs(error) > 0.015625 ? 40ms : 16ms;
            stream.log("[" + label + "] Correcting " + std::string(horizontal ? "horizontal" : "vertical") + " axis " +
                (horizontal ? (result.dx > 0 ? "right" : "left") : (result.dy < 0 ? "up" : "down")) + " for " +
                std::to_string(duration.count()) + "ms.");
            // Reuse confirmed-Run encounter handling if correction enters grass.
            walk_through_grass(stream,
                context,
                horizontal ? (result.dx > 0 ? 1 : -1) : 0,
                horizontal ? 0 : (result.dy < 0 ? 1 : -1),
                1,
                label,
                BattleType::STANDARD,
                duration);
        }
        context.wait_for_all_requests();
    }
    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
        label + ": Reference position not confirmed; route movement stopped.",
        stream);
}

static void align_to_sandgem_junction(VideoStream& stream, ProControllerContext& context){
    align_to_tutorial_landmarks(stream, context, match_sandgem_junction, "Sandgem junction");
}

// Resumes from the save made after catching the second Pokemon.
static void tutorial_part_3_after_catch_navigation(SingleSwitchProgramEnvironment& env,
    VideoStream& stream,
    ProControllerContext& context){
    pbf_wait(context, 1500ms);
    //--------------------------------------------------------
    // To get potions
    // -------------------------------------------------------
    walk_through_grass(stream,
        context,
        1,
        0,
        10,
        "Route 202 grass exit",
        BattleType::STANDARD,
        200ms,
        match_route202_grass_exit);
    // Recording: the exit reference was reached after two 200ms steps.
    // Eight further steps (1600ms) reached the eastern tree boundary.
    // Continue outside grass to preserve the next down movement's start.
    stream.log("[Tutorial] Route 202 grass exit confirmed; continuing right to the tree boundary.");
    pbf_move_left_joystick(context, {1.0, 0.0}, 1600ms, 0ms);
    pbf_wait(context, 700ms);
    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 1421ms, 0ms);
    pbf_wait(context, 52ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1013ms, 0ms);
    pbf_wait(context, 142ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 1112ms, 0ms);
    pbf_wait(context, 45ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 3206ms, 0ms);
    pbf_wait(context, 912ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 856ms, 0ms);
    context.wait_for_all_requests();
    pbf_wait(context, 1500ms);
    walk_through_grass(stream, context, -1, 0, 18, "left through grass 3");
    pbf_press_button(context, BUTTON_A, 180ms, 0ms);

    wait_for_dialogue(stream, context, "Phase 17 (accept potions)");

    pbf_mash_button(context, BUTTON_B, 10000ms);
    context.wait_for_all_requests();
    // Catching the second Pokemon can damage the starter. Verify healing
    // once Potions are available, before continuing toward Tristan.
    stream.log("[Tutorial] Checking starter HP after the first Potion pickup.");
    use_potion_first_pokemon(stream, context);
    context.wait_for_all_requests();
    // Complete one training encounter before Tristan. The helper waits
    // through XP/automatic move learning and heals after the fight.
    battle_1_pokemon(stream, context);

    // Stop on the verified Route 201 reference instead of overshooting
    // with ten fixed steps. The limit also allows small final corrections.
    walk_through_grass(stream,
        context,
        1,
        0,
        20,
        "Route 201 Potion exit",
        BattleType::STANDARD,
        200ms,
        match_route201_potion_exit);
    context.wait_for_all_requests();
    pbf_wait(context, 1500ms);

    // ---------------------------------------------------------
    // Navigate back to catch tutorial grass
    // ---------------------------------------------------------
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 560ms, 0ms);
    pbf_wait(context, 1500ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 5000ms, 0ms);
    pbf_wait(context, 1500ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1000ms, 0ms);
    pbf_wait(context, 1500ms);
    align_to_sandgem_junction(stream, context);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 1855ms, 0ms);
    pbf_wait(context, 1500ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 430ms, 0ms);
    pbf_wait(context, 1500ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 850ms, 0ms);
    pbf_wait(context, 1500ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1120ms, 0ms);

    // Continue on to jubilife
    walk_through_grass(stream, context, -1, 0, 18, "left through grass 3");
    walk_through_grass(stream, context, 0, 1, 18, "up through grass 3");
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 1000ms, 0ms);
    wait_for_dialogue(stream, context, "Phase 18 (tristan encounter dialog)");

    pbf_mash_button(context, BUTTON_B, 5000ms);
    fight_tristan(stream, context);

    use_potion_first_pokemon(stream, context);
    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 1366ms, 0ms);
    pbf_wait(context, 467ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 1018ms, 0ms);
    pbf_wait(context, 311ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 353ms, 0ms);
    pbf_wait(context, 660ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 580ms, 0ms);
    pbf_wait(context, 1672ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 1344ms, 0ms);

    wait_for_dialogue(stream, context, "Phase 19 (natalie encounter dialog)");

    pbf_mash_button(context, BUTTON_B, 5000ms);

    fight_Natalie(stream, context);
    context.wait_for_all_requests();
    use_potion_first_pokemon(stream, context);
    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {1.000000, -0.000000}, 1301ms, 0ms);
    walk_through_grass(stream, context, 0, 1, 15, "up through grass");
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 392ms, 0ms);
    pbf_wait(context, 1261ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 815ms, 0ms);

    wait_for_dialogue(stream, context, "Phase 20 (logan encounter dialog)");

    pbf_mash_button(context, BUTTON_B, 5000ms);
    fight_Natalie(stream, context);
    context.wait_for_all_requests();
    use_potion_first_pokemon(stream, context);
    context.wait_for_all_requests();
    walk_through_grass(stream, context, -1, 0, 8, "left through grass");

    pbf_move_left_joystick(context, {0.000000, 1.000000}, 2469ms, 0ms);
    wait_for_dialogue(stream, context, "Phase 21 (dawn Jubilife dialog)");

    pbf_mash_button(context, BUTTON_B, 5000ms);
}
// ---------------------------------------------------------------------------
// End of Segment 00 - 04
// Standing in Jubilife after speaking to dawn.
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// Begin Segment 00 - 05
// Standing at the entrance to jubilife after speaking to dawn, will navigate to the poketech challenge.
// Collects poketech and stands in front of the PC to end the tutorial section
// and send the starter to pokemon HOME.
// ---------------------------------------------------------------------------

// Stop the approach as soon as buildings can guide it, instead of
// overshooting south and then blindly walking back north.
static void approach_poketch_landmarks(VideoStream& stream, ProControllerContext& context){
    context.wait_for_all_requests();
    Milliseconds walked = 0ms;
    while (walked < 1900ms){
        auto frame = stream.video().snapshot();
        if (!frame){
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Poketch approach: No video frame.",
                stream);
        }
        const auto result = match_poketch_landmarks(*frame.frame);
        VideoOverlaySet overlays(stream.overlay());
        for (const auto& landmark : result.landmarks){
            overlays.add(COLOR_CYAN, landmark.search);
            if (landmark.accepted){
                overlays.add(COLOR_GREEN, landmark.found);
            }
            stream.log("[Poketch approach] " + landmark.name + " score=" + std::to_string(landmark.score) +
                " margin=" + std::to_string(landmark.margin));
        }
        if (result.located){
            stream.log("[Poketch approach] Landmarks acquired after " + std::to_string(walked.count()) +
                "ms; ending downward approach.");
            return;
        }
        const Milliseconds step = std::min(200ms, 1900ms - walked);
        pbf_move_left_joystick(context, {0, -1}, step, 0ms);
        walked += step;
        pbf_wait(context, 700ms);
        context.wait_for_all_requests();
    }
    stream.log("[Poketch approach] Reached 1900ms downward limit; handing off to locator.");
}

static void align_and_talk_to_poketch_man(VideoStream& stream, ProControllerContext& context){
    context.wait_for_all_requests();
    pbf_wait(context, 700ms);
    context.wait_for_all_requests();
    size_t confirmed = 0, coarse_confirmed = 0;
    double previous_dx = 0, previous_dy = 0;
    size_t acquisition_misses = 0, vertical_probe = 0;
    // Two small excursions around the approach position. This is
    // visibility recovery, not permission to interact without a match.
    const int vertical_probes[] = {-80, 160, -240, 320, -160, -120, 240, -360, 480, -240};
    for (size_t attempt = 0; attempt < 80; attempt++){
        auto frame = stream.video().snapshot();
        if (!frame){
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Poketch interaction: No video frame.",
                stream);
        }
        const OverworldMatchResult result = match_poketch_landmarks(*frame.frame);
        stream.log(result.coarse ? "[Poketch landmarks] Coarse acquisition." : "[Poketch landmarks] Fine alignment.");
        VideoOverlaySet overlays(stream.overlay());
        for (const auto& landmark : result.landmarks){
            overlays.add(COLOR_CYAN, landmark.search);
            if (landmark.accepted){
                overlays.add(COLOR_GREEN, landmark.found);
            }
            stream.log("[Poketch landmarks] " + landmark.name + " score=" + std::to_string(landmark.score) +
                " margin=" + std::to_string(landmark.margin) + " accepted=" + std::to_string(landmark.accepted) +
                " screen-relative offset=(" + std::to_string(landmark.dx) + "," + std::to_string(landmark.dy) + ")");
        }
        if (result.located){
            acquisition_misses = 0;
        }
        if (result.coarse && result.located){
            if (coarse_confirmed && std::abs(result.dx - previous_dx) <= 0.00625 &&
                std::abs(result.dy - previous_dy) <= 1.0 / 90){
                coarse_confirmed++;
            }else{
                coarse_confirmed = 1;
            }
            previous_dx = result.dx;
            previous_dy = result.dy;
        }else{
            coarse_confirmed = 0;
        }
        if (!result.located){
            confirmed = 0;
            if (++acquisition_misses >= 4 && vertical_probe < sizeof(vertical_probes) / sizeof(vertical_probes[0])){
                int probe = vertical_probes[vertical_probe++];
                acquisition_misses = 0;
                stream.log("[Poketch landmarks] Bounded vertical visibility probe " + std::to_string(vertical_probe) +
                    "/10: " + (probe < 0 ? "down " : "up ") + std::to_string(std::abs(probe)) + "ms.");
                pbf_move_left_joystick(context, {0, probe < 0 ? -1.0 : 1.0}, Milliseconds(std::abs(probe)), 0ms);
                pbf_wait(context, 700ms);
            }else{
                stream.log("[Poketch landmarks] No trustworthy acquisition; waiting for another frame.");
                pbf_wait(context, 250ms);
            }
        }else if (result.coarse && coarse_confirmed < 3){
            confirmed = 0;
            stream.log("[Poketch landmarks] Waiting for consistent coarse offset before movement.");
            pbf_wait(context, 250ms);
        }else if (result.at_target){
            if (++confirmed >= 3){
                stream.log("[Poketch landmarks] Above-NPC reference confirmed; turning down.");
                pbf_press_dpad(context, DPAD_DOWN, 40ms, 400ms);
                context.wait_for_all_requests();
                pbf_press_button(context, BUTTON_A, 80ms, 500ms);
                context.wait_for_all_requests();
                wait_for_dialogue(stream, context, "Phase 27 (poketech challenge completed dialog)", 10s);
                return;
            }
            pbf_wait(context, 300ms);
        }else{
            confirmed = 0;
            const double dx = result.dx, dy = result.dy;
            stream.log("[Poketch landmarks] Reference screen-relative offset=(" + std::to_string(result.dx) + "," +
                std::to_string(result.dy) + ").");
            const bool horizontal =
                std::abs(dx) > 7.0 / 960 && (std::abs(dy) <= 7.0 / 540 || std::abs(dx) >= std::abs(dy) * 9.0 / 16);
            double error = horizontal ? dx : dy * 9.0 / 16;
            auto duration = std::abs(error) > 0.03125 ? 80ms : std::abs(error) > 0.015625 ? 40ms : 16ms;
            if (std::abs(dx) <= 7.0 / 960 && std::abs(dy) <= 7.0 / 540){
                stream.log("[Poketch landmarks] Coarse position near target; waiting for fine confirmation.");
                pbf_wait(context, 300ms);
                context.wait_for_all_requests();
                continue;
            }
            stream.log("[Poketch landmarks] Correcting " + std::string(horizontal ? "horizontal" : "vertical") +
                " axis for " + std::to_string(duration.count()) + "ms.");
            pbf_move_left_joystick(context,
                {horizontal ? (dx > 0 ? 1.0 : -1.0) : 0.0, horizontal ? 0.0 : (dy < 0 ? 1.0 : -1.0)},
                duration,
                0ms);
            pbf_wait(context, 700ms);
        }
        context.wait_for_all_requests();
    }
    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
        "Poketch interaction: Building landmarks did not confirm the reference position; A not pressed.",
        stream);
}

void align_and_enter_jubilife_center(VideoStream& stream, ProControllerContext& context){
    context.wait_for_all_requests();
    pbf_wait(context, 700ms);
    context.wait_for_all_requests();
    size_t confirmed = 0;
    for (size_t attempt = 0; attempt < 60; attempt++){
        const auto frame = stream.video().snapshot();
        if (!frame){
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Jubilife Center alignment: No video frame.",
                stream);
        }
        const auto result = match_jubilife_center_entrance(*frame.frame);
        VideoOverlaySet overlays(stream.overlay());
        for (const auto& landmark : result.landmarks){
            overlays.add(COLOR_CYAN, landmark.search);
            if (landmark.accepted){
                overlays.add(COLOR_GREEN, landmark.found);
            }
            stream.log("[Jubilife Center alignment] score=" + std::to_string(landmark.score) +
                " margin=" + std::to_string(landmark.margin) + " screen-relative offset=(" +
                std::to_string(landmark.dx) + "," + std::to_string(landmark.dy) + ")");
        }
        if (!result.located){
            confirmed = 0;
            pbf_wait(context, 250ms);
        }else if (result.at_target){
            if (++confirmed >= 3){
                stream.log("[Jubilife Center alignment] Door position confirmed; entering.");
                recorded_move_until_transition(stream, context, 0, 1, 2000ms, "Jubilife Pokemon Center entry");
                pbf_wait(context, 700ms);
                context.wait_for_all_requests();
                return;
            }
            pbf_wait(context, 300ms);
        }else{
            confirmed = 0;
            bool horizontal = std::abs(result.dx) >= std::abs(result.dy) * 9.0 / 16;
            double error = horizontal ? result.dx : result.dy * 9.0 / 16;
            const auto duration = std::abs(error) > 1.0 / 24 ? 80ms : std::abs(error) > 0.015625 ? 40ms : 16ms;
            pbf_move_left_joystick(context,
                {horizontal ? (result.dx > 0 ? 1.0 : -1.0) : 0.0, horizontal ? 0.0 : (result.dy < 0 ? 1.0 : -1.0)},
                duration,
                0ms);
            pbf_wait(context, 700ms);
        }
        context.wait_for_all_requests();
    }
    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
        "Jubilife Center alignment: Door reference not confirmed; entry not attempted.",
        stream);
}

static void
tutorial_part_4_navigation(SingleSwitchProgramEnvironment& env, VideoStream& stream, ProControllerContext& context){
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 2580ms, 0ms);
    pbf_wait(context, 504ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 830ms, 0ms);
    pbf_wait(context, 617ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 800ms, 0ms);
    pbf_wait(context, 509ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 2000ms, 0ms);
    pbf_wait(context, 598ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 200ms, 0ms);
    pbf_wait(context, 581ms);
    pbf_press_button(context, BUTTON_A, 150ms, 0ms);
    pbf_wait(context, 795ms);

    wait_for_dialogue(stream, context, "Phase 22 (barry town map dialog)");

    pbf_mash_button(context, BUTTON_B, 15000ms);
    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 1521ms, 0ms);
    pbf_wait(context, 602ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 1100ms, 0ms);
    pbf_wait(context, 864ms);
    pbf_press_dpad(context, DPAD_RIGHT, 138ms, 0ms);
    pbf_wait(context, 2398ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 1018ms, 0ms);
    pbf_wait(context, 3811ms);

    wait_for_dialogue(stream, context, "Phase 23 (poketech challenge dialog)");

    pbf_mash_button(context, BUTTON_B, 5000ms);
    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {1.000000, -0.000000}, 192ms, 0ms);
    pbf_wait(context, 820ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 980ms, 0ms);
    pbf_wait(context, 543ms);
    pbf_press_dpad(context, DPAD_UP, 203ms, 0ms);
    pbf_wait(context, 593ms);
    pbf_press_dpad(context, DPAD_UP, 115ms, 0ms);
    pbf_wait(context, 566ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 1567ms, 0ms);
    pbf_wait(context, 538ms);
    pbf_press_dpad(context, DPAD_LEFT, 157ms, 0ms);
    pbf_wait(context, 735ms);
    pbf_press_dpad(context, DPAD_LEFT, 164ms, 0ms);
    pbf_wait(context, 564ms);
    pbf_press_dpad(context, DPAD_LEFT, 250ms, 0ms);
    pbf_wait(context, 513ms);
    pbf_press_dpad(context, DPAD_LEFT, 250ms, 0ms);
    pbf_wait(context, 513ms);
    pbf_press_dpad(context, DPAD_DOWN, 163ms, 0ms);
    pbf_wait(context, 609ms);
    pbf_press_dpad(context, DPAD_DOWN, 193ms, 0ms);
    pbf_wait(context, 584ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 151ms, 0ms);
    context.wait_for_all_requests();

    //phase 24 Clown 1 dialog
    clown_dialog_helper(stream, context, DPAD_RIGHT);
    context.wait_for_all_requests();

    pbf_press_dpad(context, DPAD_UP, 187ms, 0ms);
    pbf_wait(context, 106ms);
    pbf_press_dpad(context, DPAD_UP, 32ms, 0ms);
    pbf_wait(context, 671ms);
    pbf_press_dpad(context, DPAD_UP, 262ms, 0ms);
    pbf_wait(context, 684ms);
    pbf_press_dpad(context, DPAD_UP, 372ms, 0ms);
    pbf_wait(context, 392ms);
    pbf_press_dpad(context, DPAD_UP, 419ms, 0ms);
    pbf_wait(context, 658ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 2013ms, 0ms);
    pbf_wait(context, 369ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1448ms, 0ms);
    pbf_wait(context, 2435ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 1077ms, 0ms);
    pbf_wait(context, 1084ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1280ms, 0ms);
    pbf_wait(context, 1895ms);
    pbf_press_dpad(context, DPAD_LEFT, 88ms, 0ms);

    context.wait_for_all_requests();

    //phase 25 Clown 2 dialog

    clown_dialog_helper(stream, context, DPAD_UP);

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 511ms, 0ms);
    pbf_wait(context, 1318ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 140ms, 0ms);
    pbf_wait(context, 1502ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 2093ms, 0ms);
    pbf_wait(context, 2327ms);
    pbf_press_dpad(context, DPAD_LEFT, 151ms, 0ms);
    pbf_wait(context, 549ms);
    pbf_press_dpad(context, DPAD_LEFT, 118ms, 0ms);
    pbf_wait(context, 999ms);
    pbf_press_dpad(context, DPAD_UP, 206ms, 0ms);
    pbf_wait(context, 474ms);
    pbf_press_dpad(context, DPAD_UP, 249ms, 0ms);
    pbf_wait(context, 333ms);
    pbf_press_dpad(context, DPAD_UP, 212ms, 0ms);
    pbf_wait(context, 427ms);
    pbf_press_dpad(context, DPAD_UP, 210ms, 0ms);
    context.wait_for_all_requests();
    //phase 26 Clown 3 dialog

    clown_dialog_helper(stream, context, DPAD_LEFT);

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 1098ms, 0ms);
    pbf_wait(context, 2779ms);
    pbf_press_dpad(context, DPAD_LEFT, 177ms, 0ms);
    pbf_wait(context, 488ms);
    pbf_press_dpad(context, DPAD_LEFT, 200ms, 0ms);
    pbf_wait(context, 649ms);
    pbf_press_dpad(context, DPAD_LEFT, 160ms, 0ms);
    pbf_wait(context, 4285ms);
    pbf_press_dpad(context, DPAD_DOWN, 307ms, 0ms);
    pbf_wait(context, 639ms);
    pbf_press_dpad(context, DPAD_DOWN, 463ms, 0ms);
    pbf_wait(context, 2521ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 3600ms, 0ms);
    pbf_wait(context, 610ms);
    approach_poketch_landmarks(stream, context);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 192ms, 0ms);
    pbf_wait(context, 3238ms);
    align_and_talk_to_poketch_man(stream, context);

    pbf_mash_button(context, BUTTON_B, 15000ms);
    context.wait_for_all_requests();

    // poketech acquired
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 1194ms, 0ms);
    pbf_wait(context, 1984ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 832ms, 0ms);
    pbf_wait(context, 1522ms);
    pbf_press_dpad(context, DPAD_RIGHT, 144ms, 0ms);
    pbf_wait(context, 1767ms);
    align_and_enter_jubilife_center(stream, context);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 1099ms, 0ms);
    pbf_wait(context, 1222ms);
    pbf_press_dpad(context, DPAD_RIGHT, 228ms, 0ms);
    pbf_wait(context, 625ms);
    pbf_press_dpad(context, DPAD_RIGHT, 238ms, 0ms);
    pbf_wait(context, 537ms);
    pbf_press_dpad(context, DPAD_RIGHT, 210ms, 0ms);
    pbf_wait(context, 566ms);
    pbf_press_dpad(context, DPAD_RIGHT, 179ms, 0ms);
    pbf_wait(context, 692ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 642ms, 0ms);
}
// ---------------------------------------------------------------------------
// Segment 00-05 ends
// Standing in front of the Jubilife City Pokemon Center PC.
// Player will need to deposit pokemon in the pc and transfer to Pokemon Home
// and level to 100 before starting the next segment.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  Segment / Checkpoint classes
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  Segment wrappers
// ---------------------------------------------------------------------------

std::string AutoStory_Segment_00::name() const {
    return "00: Intro to Poketch";
}
std::string AutoStory_Segment_00::start_text() const {
    return "Start: Game launched, Standing in the player's room after tv stops playing";
}
std::string AutoStory_Segment_00::end_text() const {
    return "End: Poketch received; standing at the Jubilife Pokemon Center PC, ready to deposit the starter for the HOME transfer.";
}
void AutoStory_Segment_00::run_segment(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_0000().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0001().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0002().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0003().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0004().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0005().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0006().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}

// ---------------------------------------------------------------------------
//  Checkpoint wrappers
// ---------------------------------------------------------------------------

std::string AutoStory_Checkpoint_0000::name() const {
    return "00.00 - Verify Settings";
}
std::string AutoStory_Checkpoint_0000::start_text() const {
    return "Standing in the initial bedroom before beginning the tutorial.";
}
std::string AutoStory_Checkpoint_0000::end_text() const {
    return "Required game settings verified and saved; ready for 00.01.";
}
void AutoStory_Checkpoint_0000::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0000(env, context, options, stats);
}

void checkpoint_0000(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions /*options*/,
    AutoStoryStats& /*stats*/){
    // Before any route movement or checkpoint save: fail visibly if
    // options cannot be read, rather than navigate with unknown settings.
    verify_tutorial_settings(env.console, context);
    context.wait_for_all_requests();
}

std::string AutoStory_Checkpoint_0001::name() const {
    return "00.01 - After TV";
}
std::string AutoStory_Checkpoint_0001::start_text() const {
    return "Game launched, Standing in the player's room after tv stops playing";
}
std::string AutoStory_Checkpoint_0001::end_text() const {
    return "Standing outside Lake Verity before picking starter.";
}
void AutoStory_Checkpoint_0001::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0001(env, context, options, stats);
}

void checkpoint_0001(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            // Segment 00-00: Intro and starter selection
            tutorial_part_1_navigation(env, env.console, context);
        },
        true);
}

std::string AutoStory_Checkpoint_0002::name() const {
    return "00.02 - Outside Verity before starter";
}
std::string AutoStory_Checkpoint_0002::start_text() const {
    return AutoStory_Checkpoint_0001().end_text();
}
std::string AutoStory_Checkpoint_0002::end_text() const {
    return "Standing outside Lake Verity after picking starter.";
}
void AutoStory_Checkpoint_0002::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0002(env, context, options, stats);
}

void checkpoint_0002(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            // Segment 00-01: Starter acquisition
            starter_nav(env.console, context);
            // Segment 00-00 starter selection
            switch (options.starter_choice){
            case StarterChoice::TURTWIG:
                select_turtwig(env.console, context);
                break;
            case StarterChoice::CHIMCHAR:
                select_chimchar(env.console, context);
                break;
            case StarterChoice::PIPLUP:
                select_piplup(env.console, context);
                break;
            }
            fight_starly(env.console, context);
            post_starly_clenaup(env.console, context);
        },
        true);
}

std::string AutoStory_Checkpoint_0003::name() const {
    return "00.03 - Outside Verity after starter";
}
std::string AutoStory_Checkpoint_0003::start_text() const {
    return AutoStory_Checkpoint_0002().end_text();
}
std::string AutoStory_Checkpoint_0003::end_text() const {
    return "Standing in front of The professor after recieving the Pokedex";
}
void AutoStory_Checkpoint_0003::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0003(env, context, options, stats);
}

void checkpoint_0003(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            // Segment 00-002: Pokedex acquisition
            tutorial_part_2_navigation(env, env.console, context);
        },
        true);
}

std::string AutoStory_Checkpoint_0004::name() const {
    return "00.04 - After Pokedex";
}
std::string AutoStory_Checkpoint_0004::start_text() const {
    return AutoStory_Checkpoint_0003().end_text();
}
std::string AutoStory_Checkpoint_0004::end_text() const {
    return "On Route 202 after catching the second Pokemon, with the catch sequence finished.";
}
void AutoStory_Checkpoint_0004::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0004(env, context, options, stats);
}

void checkpoint_0004(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            // Segment 00-003: Poketech acquisition
            tutorial_part_3_navigation(env, env.console, context);
        },
        true);
}

std::string AutoStory_Checkpoint_0005::name() const {
    return "00.05 - Route 202 after catch to Jubilife";
}
std::string AutoStory_Checkpoint_0005::start_text() const {
    return AutoStory_Checkpoint_0004().end_text();
}
std::string AutoStory_Checkpoint_0005::end_text() const {
    return "In Jubilife after speaking to Dawn, ready to deliver the Town Map to Barry.";
}
void AutoStory_Checkpoint_0005::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0005(env, context, options, stats);
}

void checkpoint_0005(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            tutorial_part_3_after_catch_navigation(env, env.console, context);
        },
        true);
}

std::string AutoStory_Checkpoint_0006::name() const {
    return "00.06 - Jubilife to Poketch";
}
std::string AutoStory_Checkpoint_0006::start_text() const {
    return AutoStory_Checkpoint_0005().end_text();
}
std::string AutoStory_Checkpoint_0006::end_text() const {
    return "Standing at the Jubilife Pokemon Center PC with the Poketch received, ready to deposit the starter.";
}
void AutoStory_Checkpoint_0006::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0006(env, context, options, stats);
}

void checkpoint_0006(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            // Segment 00-004: Poketech acquisition
            tutorial_part_4_navigation(env, env.console, context);
        },
        true);
}

}
}
}
