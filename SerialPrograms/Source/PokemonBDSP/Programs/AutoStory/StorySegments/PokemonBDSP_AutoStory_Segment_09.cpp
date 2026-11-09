/*  BDSP AutoStory - Segment 09
 *  From after acquiring the eighth badge until defeating the Elite Four and Champion.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"

#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/VisualDetectors/ImageMatchDetector.h"

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"

#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_MarkFinder.h"
#include "PokemonBDSP/Inference/PokemonBDSP_ShopDetectors.h"
#include "PokemonBDSP/Inference/PokemonBDSP_TransitionWatchers.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameEntry.h"
#include "PokemonBDSP/Programs/AutoStory/PokemonBDSP_AutoStory.h"
#include "PokemonBDSP/Programs/AutoStory/PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP/Programs/AutoStory/Utils/PokemonBDSP_AutoStory_Shopping.h"
#include "PokemonBDSP/Programs/AutoStory/Utils/PokemonBDSP_AutoStory_Battle.h"
#include "PokemonBDSP/Programs/AutoStory/Utils/PokemonBDSP_AutoStory_Dialog.h"

#include "PokemonBDSP_AutoStory_Segment_02.h"
#include "PokemonBDSP_AutoStory_Segment_09.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonBDSP {

// ---------------------------------------------------------------------------
//  Helpers
// ---------------------------------------------------------------------------

static bool handle_battle(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats, const std::string& trainerid){
    fight_trainer_using_moves(stream, context, stats, trainerid, true);
    return true;
}

static bool heal_and_exit(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats, const std::string& label){
    return heal_pokemon(stream, context, label, &stats);
}

// ---------------------------------------------------------------------------
//  Named route navigation blocks
// ---------------------------------------------------------------------------

static bool
enter_sunyshore_pokemon_center(VideoStream& stream, ProControllerContext& context, AutoStoryOptions options){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("enter_sunyshore_pokemon_center: flying to the Pokemon Center and entering...");

    if (!fly_to(stream, context, FlyPoint::CurrentCity)){
        stream.log("Flight to the current city failed; stopping before the next route block.", COLOR_RED);
        return false;
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("enter_sunyshore_pokemon_center: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("enter_sunyshore_pokemon_center: entered the Pokemon Center.", COLOR_GREEN);
    }

    return true;
}

static bool
go_to_victory_road_pokemon_center(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats, AutoStoryOptions /*options*/){
    DpadState dpad;
    context.wait_for_all_requests();
    stream.log("Route 223: following the recorded eastern bypass to Victory Road.");

    // Start at the Sunyshore Center doorway. Clear the woman beside the
    // Center before heading north, then go around the west side of the Mart.
    // Counts are walking steps; repeat_dpad supplies the facing press.
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 5);
    pbf_move_left_joystick(context, {0, +1}, 3400ms, 100ms);
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 4);
    pbf_move_left_joystick(context, {0, +1}, 1200ms, 100ms);
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 5);
    context.wait_for_all_requests();
    {
        MarkDetector jasmine_mark(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
        ShortDialogWatcher jasmine_dialog(COLOR_YELLOW);
        int ret = run_until<ProControllerContext>(stream, context,
            [](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {0, +1}, 6800ms, 100ms);
                pbf_wait(ctx, 3000ms);
            },
            {{jasmine_mark}, {jasmine_dialog}});
        if (ret < 0){
            stream.log("Route 223: Jasmine dialogue not detected.", COLOR_RED);
            return false;
        }
        wait_for_dialog(stream, context, "Jasmine", 15s);
        clear_dialog(stream, context, BUTTON_B, 6000ms);
    }

    // Join the recorded Surf lane two walking tiles west of this approach.
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 2);
    pbf_move_left_joystick(context, {0, +1}, 1034ms, 100ms);
    context.wait_for_all_requests();
    // The second manual route reaches the Center on one Super Repel.
    // Battle/menu time does not consume steps; expire it on the upper shore.
    if (!activate_repel(stream, context, 8, 1)){
        stream.log("Route 223: Could not activate a Super Repel.", COLOR_RED);
        return false;
    }
    use_HM(stream, context, "Surf");
    pbf_wait(context, 500ms); // Let the Surf mounting animation finish.
    context.wait_for_all_requests();

    // Follow the western rocks around Miranda, then pass east of Oscar.
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 1);
    pbf_move_left_joystick(context, {-1, 0}, 1300ms, 100ms);
    pbf_move_left_joystick(context, {0, +1}, 1300ms, 100ms);
    pbf_move_left_joystick(context, {-1, 0}, 1250ms, 100ms);
    pbf_move_left_joystick(context, {0, +1}, 1100ms, 100ms);
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 2);
    pbf_move_left_joystick(context, {0, +1}, 4000ms, 100ms);
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 2);
    context.wait_for_all_requests();
    {
        MarkDetector mark(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
        ShortDialogWatcher dialogue(COLOR_YELLOW);
        BattleMenuWatcher battle(BattleType::TRAINER, COLOR_GREEN);
        int ret = run_until<ProControllerContext>(stream, context,
            [](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {0, +1}, 1479ms, 100ms);
                pbf_wait(ctx, 3000ms);
            },
            {{mark}, {dialogue}, {battle}});
        if (ret < 0){
            stream.log("Route 223: Oscar encounter not detected.", COLOR_RED);
            return false;
        }
        fight_trainer_using_moves(stream, context, stats, "route_223_swimmer_oscar", true);
    }

    // North into the island corner below Paige, then east into the rock.
    // Go around the eastern side instead of triggering Colton and Paige.
    pbf_move_left_joystick(context, {0, +1}, 4500ms, 100ms);
    pbf_move_left_joystick(context, {+1, 0}, 3700ms, 100ms);
    // Alternative route retained for reverting the eastern bypass.
    /*
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 3);
    pbf_move_left_joystick(context, {-1, 0}, 1702ms, 100ms);
    context.wait_for_all_requests();

    // Check Ricardo during a fixed approach, then use the corresponding
    // northward leg instead of resuming an interrupted movement budget.
    {
        MarkDetector mark(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
        ShortDialogWatcher dialogue(COLOR_YELLOW);
        BattleMenuWatcher battle(BattleType::TRAINER, COLOR_GREEN);
        int ret = run_until<ProControllerContext>(stream, context,
            [](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {0, +1}, 1200ms, 100ms);
                // Allow a trainer who noticed the approach to reach us.
                pbf_wait(ctx, 3000ms);
            },
            {{mark}, {dialogue}, {battle}});
        if (ret >= 0){
            fight_trainer_using_moves(stream, context, stats, "route_223_swimmer_ricardo", true);
            pbf_move_left_joystick(context, {0, +1}, 700ms, 100ms);
        }else{
            pbf_move_left_joystick(context, {0, +1}, 500ms, 100ms);
        }
    }
    pbf_move_left_joystick(context, {-1, 0}, 1868ms, 100ms);
    context.wait_for_all_requests();

    // Wesley approaches from the west while we swim into the north island
    // wall. He may face away; continue after the encounter window if he does.
    // If he battles, finish the wall stop afterward before crossing east.
    bool wesley_defeated = false;
    for (;;){
        MarkDetector mark(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
        ShortDialogWatcher dialogue(COLOR_YELLOW);
        BattleMenuWatcher battle(BattleType::TRAINER, COLOR_GREEN);
        int ret = run_until<ProControllerContext>(stream, context,
            [](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {0, +1}, 4846ms, 100ms);
                pbf_wait(ctx, 3000ms);
            },
            {{mark}, {dialogue}, {battle}});
        if (ret < 0){
            if (!wesley_defeated){
                stream.log("Route 223: Wesley did not engage; continuing east.");
            }
            break;
        }
        if (wesley_defeated){
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Route 223 Wesley approach: Repeated trainer interruption.", stream);
        }
        fight_trainer_using_moves(stream, context, stats, "route_223_swimmer_wesley", true);
        wesley_defeated = true;
    }

    // One facing tap plus eight walking taps lines up with the land gap.
    // Crystal can interrupt the crossing. After her battle, renew the Repel
    // and use Wesley as the west stop before replaying all nine taps.
    for (size_t encounters = 0;;){
        context.wait_for_all_requests();
        MarkDetector mark(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
        ShortDialogWatcher dialogue(COLOR_YELLOW);
        BattleMenuWatcher battle(BattleType::TRAINER, COLOR_GREEN);
        int ret = run_until<ProControllerContext>(stream, context,
            [](ProControllerContext& ctx){
                DpadState crossing;
                crossing.last_dir = DPAD_LEFT;
                repeat_dpad(ctx, crossing, DPAD_RIGHT, 80ms, 300ms, 8);
                pbf_wait(ctx, 3000ms);
            },
            {{mark}, {dialogue}, {battle}});
        if (ret < 0){
            break;
        }
        if (++encounters > 1){
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Route 223 east crossing: Repeated trainer interruption after Crystal.", stream);
        }
        fight_trainer_using_moves(stream, context, stats, "route_223_swimmer_crystal", true);
        force_repel_expiry(stream, context, RepelRenewalChoice::YES, false, true);
        // Restore the west stop. If Wesley was bypassed, he may engage
        // during the return; finish his battle and then complete the wall hold.
        for (;;){
            MarkDetector return_mark(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
            ShortDialogWatcher return_dialogue(COLOR_YELLOW);
            BattleMenuWatcher return_battle(BattleType::TRAINER, COLOR_GREEN);
            int return_ret = run_until<ProControllerContext>(stream, context,
                [](ProControllerContext& ctx){
                    pbf_move_left_joystick(ctx, {-1, 0}, 4000ms, 100ms);
                    pbf_wait(ctx, 3000ms);
                },
                {{return_mark}, {return_dialogue}, {return_battle}});
            if (return_ret < 0){
                break;
            }
            if (wesley_defeated){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Route 223 west recovery: Unexpected trainer interruption after Wesley.", stream);
            }
            fight_trainer_using_moves(stream, context, stats, "route_223_swimmer_wesley", true);
            wesley_defeated = true;
        }
        context.wait_for_all_requests();
    }

    // The two narrow land crossings require separate Surf activations.
    pbf_move_left_joystick(context, {0, +1}, 712ms, 100ms);
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 2);
    pbf_move_left_joystick(context, {0, +1}, 583ms, 100ms);
    */

    // Recorded eastern bypass from the rock to the shallow-water landing.
    // Keep the recorded holds and pauses while validating the new route.
    stream.log("Route 223: bypassing Ricardo, Wesley, and Crystal on the east.");
    pbf_move_left_joystick(context, {0, +1}, 400ms, 0ms);
    pbf_wait(context, 517ms);
    pbf_move_left_joystick(context, {+1, 0}, 800ms, 0ms);
    pbf_wait(context, 404ms);
    pbf_move_left_joystick(context, {0, +1}, 450ms, 0ms);
    pbf_wait(context, 305ms);
    pbf_move_left_joystick(context, {+1, 0}, 1800ms, 0ms);
    pbf_wait(context, 281ms);
    pbf_move_left_joystick(context, {0, +1}, 3250ms, 0ms);
    pbf_wait(context, 454ms);
    pbf_move_left_joystick(context, {-1, 0}, 350ms, 0ms);
    pbf_wait(context, 401ms);
    context.wait_for_all_requests();

    // Crystal can notice us on the north approach or as we turn northwest.
    // Use fixed route cases; battle time never becomes remaining travel time.
    bool crystal_defeated = false;
    {
        MarkDetector mark(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
        ShortDialogWatcher dialogue(COLOR_YELLOW);
        BattleMenuWatcher battle(BattleType::TRAINER, COLOR_GREEN);
        int ret = run_until<ProControllerContext>(stream, context,
            [](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {0, +1}, 2200ms, 0ms);
                pbf_wait(ctx, 673ms);
                pbf_move_left_joystick(ctx, {-1, 0}, 200ms, 0ms);
                pbf_wait(ctx, 413ms);
                pbf_move_left_joystick(ctx, {0, +1}, 1175ms, 0ms);
                pbf_wait(ctx, 3000ms);
            },
            {{mark}, {dialogue}, {battle}});
        if (ret >= 0){
            stream.log("Route 223: Crystal interrupted the north approach; restoring the island corner.");
            fight_trainer_using_moves(stream, context, stats, "route_223_swimmer_crystal", true);
            crystal_defeated = true;
            // Return to the east wall, then the north corner before replaying
            // the short west turn. These recovery holds need live validation.
            pbf_move_left_joystick(context, {+1, 0}, 350ms, 100ms);
            pbf_move_left_joystick(context, {0, +1}, 2200ms, 100ms);
            pbf_move_left_joystick(context, {-1, 0}, 200ms, 0ms);
            pbf_wait(context, 413ms);
            pbf_move_left_joystick(context, {0, +1}, 1175ms, 0ms);
        }else{
            stream.log("Route 223: North approach clear; continuing the recorded northwest slide.");
        }
    }
    context.wait_for_all_requests();

    {
        MarkDetector mark(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
        ShortDialogWatcher dialogue(COLOR_YELLOW);
        BattleMenuWatcher battle(BattleType::TRAINER, COLOR_GREEN);
        int ret = run_until<ProControllerContext>(stream, context,
            [](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {-1, +1}, 6100ms, 0ms);
                pbf_wait(ctx, 3000ms);
            },
            {{mark}, {dialogue}, {battle}});
        if (ret >= 0){
            if (crystal_defeated){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Route 223 northwest slide: Unexpected interruption after Crystal.", stream);
            }
            stream.log("Route 223: Crystal interrupted the northwest slide; using the fixed landing approach.");
            fight_trainer_using_moves(stream, context, stats, "route_223_swimmer_crystal", true);
            // Replay the recorded wall slide after the encounter rather than
            // estimating unused movement time. Verify this landing case live.
            pbf_move_left_joystick(context, {-1, +1}, 6100ms, 0ms);
        }else{
            stream.log("Route 223: Northwest slide clear; continuing from the landing.");
        }
    }
    pbf_move_left_joystick(context, {0, +1}, 39ms, 0ms);
    pbf_wait(context, 500ms); // Dismount here before moving west along the shore.
    pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms);
    pbf_move_left_joystick(context, {0, -1}, 175ms, 100ms);
    context.wait_for_all_requests();
    use_HM(stream, context, "Surf");
    pbf_wait(context, 500ms); // Let the Surf mounting animation finish.
    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {-1, 0}, 609ms, 100ms);
    pbf_move_left_joystick(context, {0, +1}, 1988ms, 100ms);
    pbf_wait(context, 500ms); // Finish dismounting before the next shore input.
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);
    pbf_move_left_joystick(context, {0, +1}, 2850ms, 100ms);
    context.wait_for_all_requests();
    use_HM(stream, context, "Surf");
    pbf_wait(context, 500ms); // Let the Surf mounting animation finish.
    context.wait_for_all_requests();

    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 7);
    context.wait_for_all_requests();
    {
        MarkDetector mark(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
        ShortDialogWatcher dialogue(COLOR_YELLOW);
        BattleMenuWatcher battle(BattleType::TRAINER, COLOR_GREEN);
        int ret = run_until<ProControllerContext>(stream, context,
            [](ProControllerContext& ctx){
                DpadState approach;
                approach.last_dir = DPAD_RIGHT;
                repeat_dpad(ctx, approach, DPAD_UP, 80ms, 300ms, 4);
                // Gabrielle can face away even when we are directly below her.
                pbf_mash_button(ctx, BUTTON_A, 3000ms);
            },
            {{mark}, {dialogue}, {battle}});
        if (ret < 0){
            stream.log("Route 223: Gabrielle encounter not detected.", COLOR_RED);
            return false;
        }
        fight_trainer_using_moves(stream, context, stats, "route_223_swimmer_gabrielle", true);
    }

    // Pass Gabrielle on the west and use the northern rocks as a stop.
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 1);
    pbf_move_left_joystick(context, {0, +1}, 650ms, 100ms);
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 1);
    context.wait_for_all_requests();
    for (size_t encounters = 0;;){
        MarkDetector mark(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
        ShortDialogWatcher dialogue(COLOR_YELLOW);
        BattleMenuWatcher battle(BattleType::TRAINER, COLOR_GREEN);
        int ret = run_until<ProControllerContext>(stream, context,
            [](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {0, +1}, 4400ms, 100ms);
                pbf_wait(ctx, 3000ms);
            },
            {{mark}, {dialogue}, {battle}});
        if (ret < 0){
            break;
        }
        if (++encounters > 1){
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Route 223 Troy approach: Repeated trainer interruption.", stream);
        }
        fight_trainer_using_moves(stream, context, stats, "route_223_swimmer_troy", true);
    }
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 3);
    pbf_move_left_joystick(context, {0, +1}, 1700ms, 100ms);
    context.wait_for_all_requests();
    use_HM(stream, context, "Waterfall");
    pbf_wait(context, 3000ms);
    pbf_move_left_joystick(context, {0, +1}, 3000ms, 100ms);
    pbf_wait(context, 500ms); // Finish dismounting before the next shore input.
    context.wait_for_all_requests();

    // Safely ashore beside the Center. Decline renewal, then use the cliff
    // west of the Center to restore position after the expiry wiggle.
    force_repel_expiry(stream, context, RepelRenewalChoice::NO, false, true);
    pbf_move_left_joystick(context, { 0, -1 }, 1000ms, 100ms);
    pbf_move_left_joystick(context, {-1, 0}, 2200ms, 100ms);
    context.wait_for_all_requests();
    // Slide northeast along the Center's front wall into the doorway.
    // Stop the hold when the entry fade starts.
    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream, context,
            [](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {+1, +1}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("Route 223: Pokemon Center entry transition not detected.", COLOR_RED);
            return false;
        }
    }
    // The checkpoint's existing heal_and_exit establishes the next start.
    return true;
}

static bool enter_victory_road_cave(VideoStream& stream, ProControllerContext& context, AutoStoryOptions options){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("enter_victory_road_cave: entering the cave...");

    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 9);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("enter_victory_road_cave: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("enter_victory_road_cave: entered the Victory Road cave.", COLOR_GREEN);
    }

    return true;
}

static bool go_through_victory_road_cave(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats, AutoStoryOptions options){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_through_victory_road_cave: going through Victory Road's cave and heading to the Elite 4...");

    if (!activate_repel(stream, context, 8, 1)){
        stream.log("go_through_victory_road_cave: Could not activate a Super Repel.", COLOR_RED);
        return false;
    }
    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {0, +1}, 3200ms, 100ms); // 16+
    pbf_move_left_joystick(context, {+1, 0}, 3800ms, 100ms); // 19+
    pbf_move_left_joystick(context, {0, +1}, 3000ms, 100ms); // 15+
    pbf_move_left_joystick(context, {+1, 0}, 600ms, 100ms);  // 2+
    pbf_move_left_joystick(context, {0, -1}, 400ms, 100ms);  // 1+

    use_HM(stream, context, "Rock Climb");

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    pbf_move_left_joystick(context, {0, -1}, 400ms, 100ms);  // 1+
    pbf_move_left_joystick(context, {-1, 0}, 2200ms, 100ms); // 11+
    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 2);
    // TODO(live verification): verify west/east platform clearance before the long Mariah approach.
    // Planned before the next leg exceeds the 22s Super Repel movement budget.
    force_repel_expiry(stream, context, RepelRenewalChoice::YES, false, true);
    // Recover the horizontal position against the west wall after the expiry wiggle.
    pbf_move_left_joystick(context, {-1, 0}, 6000ms, 100ms);
    dpad.last_dir = DPAD_LEFT;
    // Preliminary stair alignment: six east presses total, including the turn.
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 6, false);
    pbf_move_left_joystick(context, {0, +1}, 2600ms, 100ms); // 13+

    {
        MarkDetector victory_road_ace_mariah_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 5800ms, 100ms); // 29+
            },
            {{victory_road_ace_mariah_fight}});
        if (ret == 0){
            context.wait_for_all_requests();
            pbf_wait(context, 2000ms);

            BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            ret = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 200000ms);
                },
                {{black_screen}});
            if (ret < 0){
                stream.log("go_through_victory_road_cave, victory_road_ace_mariah: black screen not detected!",
                    COLOR_RED);
                return false;
            }else if (ret == 0){
                handle_battle(stream, context, stats, "victory_road_ace_mariah");
            }
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);
    pbf_move_left_joystick(context, {-1, 0}, 400ms, 100ms); // 1+

    // Keep the current Super Repel through the stairs and Strength puzzle.
    // Renew after Omar; battles and blocked movement do not consume Repel steps.
    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, -1}, 2000ms, 100ms); // 10+
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 7);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_victory_road_cave: black screen not detected (1)!", COLOR_RED);
            return false;
        }
        stream.log("go_through_victory_road_cave: entered the second part of the cave.", COLOR_GREEN);
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {-1, 0}, 1000ms, 100ms); // 4+
    pbf_move_left_joystick(context, {0, -1}, 600ms, 100ms);  // 2+

    context.wait_for_all_requests();
    use_HM(stream, context, "Rock Smash");

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, -1}, 800ms, 100ms);  // 3+
    pbf_move_left_joystick(context, {+1, 0}, 400ms, 100ms);  // 1+
    pbf_move_left_joystick(context, {0, -1}, 1200ms, 100ms); // 6+

    context.wait_for_all_requests();
    use_HM(stream, context, "Strength");

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, -1}, 1500ms, 100ms); // 1++ (move boulders)
    pbf_move_left_joystick(context, {-1, 0}, 1500ms, 100ms); // 1++ (move boulders)
    pbf_move_left_joystick(context, {0, -1}, 4000ms, 100ms); // 3++ (move boulders)

    {
        MarkDetector victory_road_ace_omar_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 5800ms, 100ms); // 29+
            },
            {{victory_road_ace_omar_fight}});
        if (ret == 0){
            context.wait_for_all_requests();
            pbf_wait(context, 2000ms);

            BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            ret = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 200000ms);
                },
                {{black_screen}});
            if (ret < 0){
                stream.log("go_through_victory_road_cave, victory_road_ace_omar: black screen not detected!",
                    COLOR_RED);
                return false;
            }else if (ret == 0){
                handle_battle(stream, context, stats, "victory_road_ace_omar");
            }
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    // Renew after the Strength puzzle and Omar before continuing to Sydney.
    stream.log("[Victory Road] Renewing Super Repel after Omar.");
    force_repel_expiry(stream, context, RepelRenewalChoice::YES, false, true);
    // Recover against the west wall so the south push uses the western boulder.
    pbf_move_left_joystick(context, {-1, 0}, 1500ms, 100ms);
    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {0, -1}, 1000ms, 100ms); // 1++ (move boulders)
    pbf_move_left_joystick(context, {-1, 0}, 600ms, 100ms);  // 2+
    pbf_move_left_joystick(context, {0, -1}, 800ms, 100ms);  // 3+
    pbf_move_left_joystick(context, {+1, 0}, 600ms, 100ms);  // 2+
    pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms);  // 1+

    context.wait_for_all_requests();
    use_HM(stream, context, "Rock Smash");

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, +1}, 4000ms, 100ms); // 2++ (move boulders)
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 1);
    pbf_move_left_joystick(context, {+1, 0}, 1500ms, 100ms); // 4++ (move after boulder moving)
    pbf_move_left_joystick(context, {0, -1}, 600ms, 100ms);  // 2+

    context.wait_for_all_requests();
    use_HM(stream, context, "Rock Smash");

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, -1}, 600ms, 100ms); // 2+
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 2);

    {
        MarkDetector victory_road_ace_sydney_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 5800ms, 100ms); // 29+
            },
            {{victory_road_ace_sydney_fight}});
        if (ret == 0){
            context.wait_for_all_requests();
            pbf_wait(context, 2000ms);

            BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            ret = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 200000ms);
                },
                {{black_screen}});
            if (ret < 0){
                stream.log("go_through_victory_road_cave, victory_road_ace_sydney: black screen not detected!",
                    COLOR_RED);
                return false;
            }else if (ret == 0){
                handle_battle(stream, context, stats, "victory_road_ace_sydney");
            }
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    // Carry Omar's renewal through Sydney and the next boulder approach.
    // The confined lane beside Sydney is unsuitable for forced expiration.

    pbf_move_left_joystick(context, {0, +1}, 600ms, 100ms);  // 2+
    pbf_move_left_joystick(context, {+1, 0}, 2000ms, 100ms); // 10+

    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 1);
    pbf_move_left_joystick(context, {+1, 0}, 1500ms, 100ms); // 2++ (move boulders)

    {
        MarkDetector victory_road_veteran_clayton_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 5800ms, 100ms); // 29+
            },
            {{victory_road_veteran_clayton_fight}});
        if (ret == 0){
            context.wait_for_all_requests();
            pbf_wait(context, 2000ms);

            BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            ret = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 200000ms);
                },
                {{black_screen}});
            if (ret < 0){
                stream.log("go_through_victory_road_cave, victory_road_veteran_clayton: black screen not detected!",
                    COLOR_RED);
                return false;
            }else if (ret == 0){
                fight_trainer_using_moves(stream, context, stats,
                    "victory_road_veteran_clayton", true, false, options.language);
            }
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {-1, 0}, 400ms, 100ms); // 1+

    // Omar's renewal carries through Sydney and Clayton before this reset.
    stream.log("[Victory Road] Renewing Super Repel after Clayton.");
    force_repel_expiry(stream, context, RepelRenewalChoice::YES, true, true);

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, +1}, 1800ms, 100ms); // 9+

    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 1);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_victory_road_cave: black screen not detected (2)!", COLOR_RED);
            return false;
        }
        stream.log("go_through_victory_road_cave: entered the third part of the cave.", COLOR_GREEN);
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {+1, 0}, 3600ms, 100ms); // 18+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 3);
    pbf_move_left_joystick(context, {0, -1}, 400ms, 100ms); // 1+

    context.wait_for_all_requests();
    use_HM(stream, context, "Rock Climb");

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    // Keep this reset before the combined Miles/Valencia leg; carrying Clayton's
    // renewal through that entire leg would exceed the 22s movement budget.
    stream.log("[Victory Road] Renewing Super Repel before Miles and Valencia.");
    force_repel_expiry(stream, context, RepelRenewalChoice::YES, false, true);

    pbf_move_left_joystick(context, {-1, 0}, 1200ms, 100ms); // 6+
    pbf_move_left_joystick(context, {0, +1}, 2400ms, 100ms); // 12+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 1);

    {
        MarkDetector victory_road_blackbelt_miles_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 5800ms, 100ms); // 29+
            },
            {{victory_road_blackbelt_miles_fight}});
        if (ret == 0){
            context.wait_for_all_requests();
            pbf_wait(context, 2000ms);

            BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            ret = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 200000ms);
                },
                {{black_screen}});
            if (ret < 0){
                stream.log("go_through_victory_road_cave, victory_road_blackbelt_miles: black screen not detected!",
                    COLOR_RED);
                return false;
            }else if (ret == 0){
                handle_battle(stream, context, stats, "victory_road_blackbelt_miles");
            }
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_victory_road_cave: black screen not detected (3)!", COLOR_RED);
            return false;
        }
        stream.log("go_through_victory_road_cave: entered the fourth part of the cave.", COLOR_GREEN);
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    // Carry the Rock Climb landing renewal through Miles and Valencia.
    // Do not wiggle at this entrance: it displaces the following Surf approach.

    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 1);
    pbf_move_left_joystick(context, {+1, 0}, 2400ms, 100ms); // 12+
    pbf_move_left_joystick(context, {0, +1}, 1000ms, 100ms); // 4+

    context.wait_for_all_requests();
    use_HM(stream, context, "Surf");

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, +1}, 1000ms, 100ms); // 4+
    pbf_move_left_joystick(context, {+1, 0}, 800ms, 100ms);  // 3+

    {
        MarkDetector victory_road_psychic_valencia_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 5800ms, 100ms); // 29+
            },
            {{victory_road_psychic_valencia_fight}});
        if (ret == 0){
            context.wait_for_all_requests();
            pbf_wait(context, 2000ms);

            BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            ret = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 200000ms);
                },
                {{black_screen}});
            if (ret < 0){
                stream.log("go_through_victory_road_cave, victory_road_psychic_valencia: black screen not detected!",
                    COLOR_RED);
                return false;
            }else if (ret == 0){
                handle_battle(stream, context, stats, "victory_road_psychic_valencia");
            }
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms); // 1+

    context.wait_for_all_requests();
    use_HM(stream, context, "Surf");

    context.wait_for_all_requests();

    force_repel_expiry(stream, context, RepelRenewalChoice::YES, false, true);

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {-1, 0}, 2000ms, 100ms); // 10+
    pbf_move_left_joystick(context, {0, +1}, 1200ms, 100ms); // 6+
    pbf_move_left_joystick(context, {+1, 0}, 1200ms, 100ms); // 6+
    pbf_move_left_joystick(context, {0, +1}, 2200ms, 100ms); // 11+

    context.wait_for_all_requests();
    use_HM(stream, context, "Waterfall");

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);
    pbf_move_left_joystick(context, {0, +1}, 1200ms, 100ms); // 6+

    {
        MarkDetector victory_road_dragontamer_andre_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 5800ms, 100ms); // 29+
            },
            {{victory_road_dragontamer_andre_fight}});
        if (ret == 0){
            context.wait_for_all_requests();
            pbf_wait(context, 2000ms);

            BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            ret = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 200000ms);
                },
                {{black_screen}});
            if (ret < 0){
                stream.log("go_through_victory_road_cave, victory_road_dragontamer_andre: black screen not detected!",
                    COLOR_RED);
                return false;
            }else if (ret == 0){
                handle_battle(stream, context, stats, "victory_road_dragontamer_andre");
            }
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {-1, 0}, 400ms, 100ms); // 1+

    context.wait_for_all_requests();
    use_HM(stream, context, "Surf");

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {-1, 0}, 1200ms, 100ms); // 6+
    pbf_move_left_joystick(context, {0, -1}, 5000ms, 100ms); // 17++ (getting out of the water)

    // Renew north/south on shore, away from the west doorway.
    stream.log("[Victory Road] Renewing Super Repel on shore before Edgar.");
    force_repel_expiry(stream, context, RepelRenewalChoice::YES, true, true);
    // Return against the south rock before taking the east step to the stairs.
    pbf_move_left_joystick(context, {0, -1}, 1000ms, 100ms);
    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {+1, 0}, 600ms, 100ms);  // 2+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 2);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_victory_road_cave: black screen not detected (4)!", COLOR_RED);
            return false;
        }
        stream.log("go_through_victory_road_cave: entered the fifth part of the cave.", COLOR_GREEN);
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms); // 1+

    // Keep the shore renewal through Edgar to the next Rock Climb landing.
    // A second wiggle here crosses back through the doorway.
    pbf_move_left_joystick(context, {-1, 0}, 3000ms, 100ms); // 15+
    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 3);

    {
        MarkDetector victory_road_veteran_edgar_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, -1}, 5800ms, 100ms); // 29+
            },
            {{victory_road_veteran_edgar_fight}});
        if (ret == 0){
            context.wait_for_all_requests();
            pbf_wait(context, 2000ms);

            BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            ret = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 200000ms);
                },
                {{black_screen}});
            if (ret < 0){
                stream.log("go_through_victory_road_cave, victory_road_veteran_edgar: black screen not detected!",
                    COLOR_RED);
                return false;
            }else if (ret == 0){
                handle_battle(stream, context, stats, "victory_road_veteran_edgar");
            }
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {0, -1}, 2600ms, 100ms); // 13+
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 1);
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 9);
    pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms); // 1+

    context.wait_for_all_requests();
    use_HM(stream, context, "Rock Climb");

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    // TODO(live verification): verify west/east clearance on the final Rock Climb landing before Clinton.
    // Planned before the next leg exceeds the 22s Super Repel movement budget.
    force_repel_expiry(stream, context, RepelRenewalChoice::YES, false, true);

    {
        MarkDetector victory_road_dragontamer_clinton_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 5800ms, 100ms); // 29+
            },
            {{victory_road_dragontamer_clinton_fight}});
        if (ret == 0){
            context.wait_for_all_requests();
            pbf_wait(context, 2000ms);

            BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            ret = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 200000ms);
                },
                {{black_screen}});
            if (ret < 0){
                stream.log("go_through_victory_road_cave, victory_road_dragontamer_clinton: black screen not detected!",
                    COLOR_RED);
                return false;
            }else if (ret == 0){
                fight_trainer_using_moves(stream, context, stats,
                    "victory_road_dragontamer_clinton", true, false, options.language);
            }
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {0, +1}, 5600ms, 100ms); // 28+

    // TODO(live verification): measure the final exit approach and verify wiggle clearance.
    // Keep this reset for now: Clinton, the full exit hold, and the League approach
    // total about 25s of commanded movement since the previous renewal.
    stream.log("[Victory Road] Renewing Super Repel before the final cave exit.");
    force_repel_expiry(stream, context, RepelRenewalChoice::YES, false, true);
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 1);

    {
        WhiteScreenOverWatcher white_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 10000ms, 100ms);
            },
            {{white_screen}});
        if (ret < 0){
            stream.log("go_through_victory_road_cave: white screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_victory_road_cave: exited Victory Road's cave.", COLOR_GREEN);
    }

    return true;
}

static bool enter_elite_4_pokemon_center(VideoStream& stream, ProControllerContext& context, AutoStoryOptions options){
    DpadState dpad;
    context.wait_for_all_requests();
    //pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("enter_elite_4_pokemon_center: Entering the Elite 4 building...");

    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 6);
    pbf_move_left_joystick(context, {0, +1}, 2000ms, 100ms); // 10+

    context.wait_for_all_requests();
    use_HM(stream, context, "Surf");

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, +1}, 600ms, 100ms); // 2+
    context.wait_for_all_requests();
    use_HM(stream, context, "Waterfall");

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    // TODO(live verification): verify north/south clearance below the League doorway; keep the door column fixed.
    // Planned before the next leg exceeds the 22s Super Repel movement budget.
    force_repel_expiry(stream, context, RepelRenewalChoice::NO, true, true);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("enter_elite_4_pokemon_center: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("enter_elite_4_pokemon_center: entered the Elite 4 building.", COLOR_GREEN);
        // Let the entrance fade settle before the next checkpoint opens the save menu.
        pbf_wait(context, 1000ms);
    }

    return true;
}

static bool ready_in_front_of_elite_4(VideoStream& stream,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    if (options.help_mode != HelpMode::OUTSIDE_HELP){
        throw FatalProgramException(ErrorReportMode::NO_ERROR_REPORT,
            "09.04 unassisted League preparation is not implemented yet.", stream);
    }

    const VeilstoneShoppingList* shopping = nullptr;
    switch (options.starter_choice){
    case StarterChoice::CHIMCHAR:
        shopping = &ShoppingLists::ASSISTED_INFERNAPE_LEAGUE;
        break;
    case StarterChoice::PIPLUP:
        shopping = &ShoppingLists::ASSISTED_EMPOLEON_LEAGUE;
        break;
    case StarterChoice::TURTWIG:
        shopping = &ShoppingLists::ASSISTED_TORTERRA_LEAGUE;
        break;
    default:
        throw FatalProgramException(ErrorReportMode::NO_ERROR_REPORT,
            "09.04 has no shopping plan for the selected starter.", stream);
    }
    const bool visit_store = !shopping->floor3_top.empty() || !shopping->floor3_bottom.empty() ||
        !shopping->floor2_top.empty() || !shopping->floor2_middle.empty() || !shopping->floor2_bottom.empty();

    enum class PreparationStage {
        HEAL_BEFORE_RIVAL,
        RIVAL,
        HEAL_AFTER_RIVAL,
        MEDICINE,
        EXIT_LEAGUE,
        FLY_VEILSTONE,
        DEPARTMENT_STORE,
        CLEAR_PURCHASE_FLAGS,
        RETURN_TO_LEAGUE,
        DONE,
    };
    // Stages are local to this attempt. Checkpoint recovery restores the save,
    // inventory and PP before restarting; never resume a partial shopping trip.
    PreparationStage stage = PreparationStage::HEAL_BEFORE_RIVAL;
    DpadState dpad;
    while (stage != PreparationStage::DONE){
        context.wait_for_all_requests();
        switch (stage){
        case PreparationStage::HEAL_BEFORE_RIVAL:
            stream.log("League preparation: heal before rival.");
            // victoryroadprep counts include facing presses.
            repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 8, false);
            if (!heal_pokemon(stream, context, "League before rival", &stats, false)){
                return false;
            }
            stage = PreparationStage::RIVAL;
            break;
        case PreparationStage::RIVAL:
            stream.log("League preparation: rival battle.");
            // Clear the west statue even when healing finishes one tile west of the nurse.
            repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 8, false);
            repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 3, false);
            fight_trainer_using_moves(stream, context, stats, "pokemon_league_rival", true, false, options.language);
            stage = PreparationStage::HEAL_AFTER_RIVAL;
            break;
        case PreparationStage::HEAL_AFTER_RIVAL:
            stream.log("League preparation: heal after rival.");
            // Clear the south edge of the seats before crossing to the nurse.
            repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 5, false);
            repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 7, false);
            if (!heal_pokemon(stream, context, "League after rival", &stats, false)){
                return false;
            }
            stage = PreparationStage::MEDICINE;
            break;
        case PreparationStage::MEDICINE:
            stream.log("League preparation: stock Full Restores at the League mart.");
            // Reuse the recorded nurse-to-mart crossing after the second heal.
            repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 13, false);
            repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 2, false);
            if (!buy_items(stream, context, ShoppingLists::ASSISTED_LEAGUE_MEDICINE, options.language)){
                return false;
            }
            // Reverse the same crossing to restore the recorded nurse-side exit.
            repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 13, false);
            repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 2, false);
            stage = PreparationStage::EXIT_LEAGUE;
            break;
        case PreparationStage::EXIT_LEAGUE:
        {
            stream.log("League preparation: exit the League building.");
            pbf_move_left_joystick(context, {0, -1}, 1086ms, 300ms);
            pbf_move_left_joystick(context, {1, 0}, 954ms, 300ms);
            context.wait_for_all_requests();
            BlackScreenOverWatcher black(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            int ret = run_until<ProControllerContext>(stream, context,
                [](ProControllerContext& ctx){
                    // The timed east crossing can end just east of the door.
                    // Slide west along the south wall until the doorway fade.
                    pbf_move_left_joystick(ctx, {-1, -1}, 2500ms, 0ms);
                    pbf_wait(ctx, 10000ms);
                },
                {{black}});
            if (ret < 0){
                stream.log("League preparation: exit transition missing.", COLOR_RED);
                return false;
            }
            pbf_wait(context, 500ms);
            stage = visit_store ? PreparationStage::FLY_VEILSTONE : PreparationStage::CLEAR_PURCHASE_FLAGS;
            break;
        }
        case PreparationStage::FLY_VEILSTONE:
            stream.log("League preparation: fly to Veilstone for starter supplies.");
            if (!fly_to(stream, context, FlyPoint::VeilstoneCity)){
                return false;
            }
            stage = PreparationStage::DEPARTMENT_STORE;
            break;
        case PreparationStage::DEPARTMENT_STORE:
            stream.log("League preparation: purchase the selected starter's store list.");
            if (!shop_veilstone_department_store(stream, context, *shopping, options.language)){
                return false;
            }
            stage = PreparationStage::CLEAR_PURCHASE_FLAGS;
            break;
        case PreparationStage::CLEAR_PURCHASE_FLAGS:
            stream.log("League preparation: clear NEW flags after all purchases.");
            // Medicine and battle items have short lists. The saved test
            // inventory still has an uncleared TM; skip that pocket once clear.
            if (!clear_new_item_flags(stream, context, 8,
                {4s, 4s, 4s, 4s, 6s, 30s, 6s, 10s})){
                return false;
            }
            stage = PreparationStage::RETURN_TO_LEAGUE;
            break;
        case PreparationStage::RETURN_TO_LEAGUE:
            stream.log("League preparation: fly to the upper Pokemon League entrance.");
            // Torterra also uses Fly to establish the same admission start position.
            if (!fly_to(stream, context, FlyPoint::PokemonLeagueUpper)){
                return false;
            }
            stage = PreparationStage::DONE;
            break;
        case PreparationStage::DONE:
            break;
        }
    }
    context.wait_for_all_requests();
    return true;
}

static bool get_into_the_elite_4_entrance(VideoStream& stream,
    ProControllerContext& context,
    AutoStoryOptions /*options*/){
    // Two northward transitions: building entry, then admission to the first lift vestibule.
    for (size_t transition = 0; transition < 2; transition++){
        stream.log(transition == 0 ? "League admission: enter the building."
            : "League admission: enter the first lift vestibule.");
        context.wait_for_all_requests();
        BlackScreenWatcher black(COLOR_RED);
        int ret = run_until<ProControllerContext>(stream, context,
            [](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {0, +1}, 10000ms, 0ms);
            },
            {{black}});
        if (ret < 0){
            stream.log("League admission: northward transition did not start.", COLOR_RED);
            return false;
        }
        // Stop movement at the fade so it cannot carry into the destination room.
        BlackScreenWatcher visible(COLOR_RED, {0.1, 0.1, 0.8, 0.8}, 100, 10,
            BlackScreenWatcher::FinderType::GONE, 500ms);
        if (wait_until(stream, context, 15s, {{visible}}) < 0){
            stream.log("League admission: transition did not finish.", COLOR_RED);
            return false;
        }
        pbf_wait(context, 500ms);
        if (transition == 0){
            stream.log("League admission: approach and clear the attendant dialogue.");
            pbf_move_left_joystick(context, {0, +1}, 5000ms, 0ms);
            pbf_press_button(context, BUTTON_A, 80ms, 300ms);
            // Badge inspection hides the text for about seven seconds before
            // the attendant steps aside and delivers the final admission line.
            clear_dialog(stream, context, ClearDialogMode::STOP_OVERWORLD, 60, BUTTON_A, 10000ms);
        }
    }
    context.wait_for_all_requests();
    stream.log("League admission: inside the first lift vestibule.", COLOR_GREEN);
    return true;
}


// e4r1 begins in the admission lift vestibule. e4r2 begins directly
// south of a defeated member, facing north, with the rear door unlocked.
static bool approach_elite_four_member(VideoStream& stream,
    ProControllerContext& context,
    bool first_member,
    bool champion = false){
    DpadState dpad;
    dpad.last_dir = DPAD_UP;
    if (!first_member){
        stream.log("Elite Four: pass the defeated member on the west and enter the next lift.");
        // e4r2: one tile west, three north, then one east to the doorway.
        // Counts exclude the facing press supplied by repeat_dpad.
        repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);
        repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 3);
        repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 1);
        context.wait_for_all_requests();
        BlackScreenWatcher black(COLOR_RED);
        int ret = run_until<ProControllerContext>(stream, context,
            [](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {0, +1}, 3000ms, 0ms);
            },
            {{black}});
        if (ret < 0){
            stream.log("Elite Four: rear doorway transition did not start.", COLOR_RED);
            return false;
        }
        BlackScreenWatcher visible(COLOR_RED, {0.1, 0.1, 0.8, 0.8}, 100, 10,
            BlackScreenWatcher::FinderType::GONE, 500ms);
        if (wait_until(stream, context, 15s, {{visible}}) < 0){
            stream.log("Elite Four: lift vestibule transition did not finish.", COLOR_RED);
            return false;
        }
        pbf_wait(context, 500ms);
        dpad.last_dir = DPAD_UP;
    }

    stream.log("Elite Four: board the lift and wait for its ascent.");
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 3);
    // The recording shows a continuous lift animation, not a black transition.
    // e4r1 waits 6378ms here; no upper-landing input may run during the ascent.
    pbf_wait(context, 6500ms);
    context.wait_for_all_requests();
    {
        BlackScreenWatcher black(COLOR_RED);
        int ret = run_until<ProControllerContext>(stream, context,
            [champion](ProControllerContext& ctx){
                // Cynthia has a longer corridor above the lift. Cancel all
                // remaining presses at the doorway fade in either layout.
                for (size_t step = 0; step < (champion ? 24u : 8u); step++){
                    pbf_press_dpad(ctx, DPAD_UP, 80ms, 300ms);
                }
                pbf_wait(ctx, 3000ms);
            },
            {{black}});
        if (ret < 0){
            stream.log("Elite Four: upper doorway transition did not start.", COLOR_RED);
            return false;
        }
        BlackScreenWatcher visible(COLOR_RED, {0.1, 0.1, 0.8, 0.8}, 100, 10,
            BlackScreenWatcher::FinderType::GONE, 500ms);
        if (wait_until(stream, context, 15s, {{visible}}) < 0){
            stream.log("Elite Four: member room transition did not finish.", COLOR_RED);
            return false;
        }
    }
    if (champion){
        // The doorway triggers the camera approach and dialogue automatically.
        // No landing-room movement or A press belongs after this transition.
        context.wait_for_all_requests();
        return true;
    }
    // The room is visible before the entrance door finishes closing.
    // e4r1 pauses 3866ms at entry; keep movement out of that cutscene.
    pbf_wait(context, 4000ms);
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, first_member ? 3 : 4);
    context.wait_for_all_requests();
    return true;
}


static bool deal_with_aaron_elite_4(VideoStream& stream,
    ProControllerContext& context,
    AutoStoryStats& stats,
    Language language){
    if (!approach_elite_four_member(stream, context, true)){
        return false;
    }
    pbf_press_button(context, BUTTON_A, 80ms, 300ms);
    wait_for_dialog(stream, context, "Elite Four Aaron", 15s);
    // The battle handler clears the introduction and camera cutscene with B,
    // stopping on the battle menu before choosing a move.
    fight_trainer_using_moves(stream, context, stats, "elite_four_aaron", false, false, language);
    clear_dialog(stream, context, ClearDialogMode::STOP_OVERWORLD, 60, BUTTON_B, 6000ms);
    context.wait_for_all_requests();
    use_item_in_battle(stream, context, language, "full-restore", true);
    return true;
}

static bool deal_with_bertha_elite_4(VideoStream& stream,
    ProControllerContext& context,
    AutoStoryStats& stats,
    Language language){
    if (!approach_elite_four_member(stream, context, false)){
        return false;
    }
    pbf_press_button(context, BUTTON_A, 80ms, 300ms);
    wait_for_dialog(stream, context, "Elite Four Bertha", 15s);
    // The battle handler clears the introduction and camera cutscene with B,
    // stopping on the battle menu before choosing a move.
    fight_trainer_using_moves(stream, context, stats, "elite_four_bertha", false, false, language);
    clear_dialog(stream, context, ClearDialogMode::STOP_OVERWORLD, 60, BUTTON_B, 6000ms);
    context.wait_for_all_requests();
    use_item_in_battle(stream, context, language, "full-restore", true);
    return true;
}

static bool deal_with_flint_elite_4(VideoStream& stream,
    ProControllerContext& context,
    AutoStoryStats& stats,
    Language language){
    if (!approach_elite_four_member(stream, context, false)){
        return false;
    }
    pbf_press_button(context, BUTTON_A, 80ms, 300ms);
    wait_for_dialog(stream, context, "Elite Four Flint", 15s);
    // The battle handler clears the introduction and camera cutscene with B,
    // stopping on the battle menu before choosing a move.
    fight_trainer_using_moves(stream, context, stats, "elite_four_flint", false, false, language);
    clear_dialog(stream, context, ClearDialogMode::STOP_OVERWORLD, 60, BUTTON_B, 6000ms);
    context.wait_for_all_requests();
    use_item_in_battle(stream, context, language, "full-restore", true);
    return true;
}

static bool deal_with_lucian_elite_4(VideoStream& stream,
    ProControllerContext& context,
    AutoStoryStats& stats,
    Language language){
    if (!approach_elite_four_member(stream, context, false)){
        return false;
    }
    pbf_press_button(context, BUTTON_A, 80ms, 300ms);
    wait_for_dialog(stream, context, "Elite Four Lucian", 15s);
    // The battle handler clears the introduction and camera cutscene with B,
    // stopping on the battle menu before choosing a move.
    fight_trainer_using_moves(stream, context, stats, "elite_four_lucian", false, false, language);
    clear_dialog(stream, context, ClearDialogMode::STOP_OVERWORLD, 60, BUTTON_B, 6000ms);
    context.wait_for_all_requests();
    use_item_in_battle(stream, context, language, "full-restore", true);
    return true;
}

static bool deal_with_cynthia_and_finish(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    auto& stream = env.console;
    if (!approach_elite_four_member(stream, context, false, true)){
        return false;
    }
    // cynthiaapproach: room entry starts the dialogue without interaction.
    wait_for_dialog(stream, context, "Champion Cynthia automatic introduction", 20s);
    fight_trainer_using_moves(stream, context, stats, "champion_cynthia", false, false, options.language);
    // 2026-11-06 16-50-50: battle exit ~5:26, last dialogue ~6:51,
    // first scrolling credits ~7:40. Mash through the recorded ending for
    // 138 seconds plus 2 seconds into the credits; no ending detector needed.
    stream.log("[AutoStory] Clearing the ending and Hall of Fame; mashing A for 140 seconds.");
    pbf_mash_button(context, BUTTON_A, 140000ms);
    context.wait_for_all_requests();

    // The Hall of Fame has saved. A failure beyond this point must not
    // rerun the Champion route against the newly written postgame save.
    try {
        go_home(env.console, context);
        if (!reset_game_from_home(env, env.console, context, true, 1000ms)){
            throw FatalProgramException(ErrorReportMode::NO_ERROR_REPORT,
                "Autostory ending: game reentry failed after the Hall of Fame save.", stream);
        }
        if (!save_game(env, context)){
            throw FatalProgramException(ErrorReportMode::NO_ERROR_REPORT,
                "Autostory ending: the final game save could not be verified.", stream);
        }
    } catch (OperationFailedException& e){
        throw FatalProgramException(ErrorReportMode::NO_ERROR_REPORT,
            "Autostory ending stopped after the Hall of Fame save: " + e.message(), stream);
    }
    throw FatalProgramException(ErrorReportMode::NO_ERROR_REPORT,
        "Congratulations! Autostory Complete!", stream);
}

// ---------------------------------------------------------------------------
//  Segment wrappers
// ---------------------------------------------------------------------------

std::string AutoStory_Segment_09::name() const {
    return "09: Sunyshore to Champion";
}

std::string AutoStory_Segment_09::start_text() const {
    return AutoStory_Checkpoint_0901().start_text();
}

std::string AutoStory_Segment_09::end_text() const {
    return AutoStory_Checkpoint_0909().end_text();
}

void AutoStory_Segment_09::run_segment(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);
    AutoStory_Checkpoint_0901().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0902().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0903().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0904().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0905().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0906().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0907().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0908().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0909().run_checkpoint(env, context, options, stats);
    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}

// ---------------------------------------------------------------------------
//  Checkpoint wrappers
// ---------------------------------------------------------------------------

std::string AutoStory_Checkpoint_0901::name() const {
    return "09.01 - Sunyshore recovery";
}

std::string AutoStory_Checkpoint_0901::start_text() const {
    return "Outside Sunyshore Gym at the doorway, facing south, with eighth-badge rewards and dialogue finished.";
}

std::string AutoStory_Checkpoint_0901::end_text() const {
    return "Healed and outside Sunyshore Pokemon Center.";
}

void AutoStory_Checkpoint_0901::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0901(env, context, options, stats);
}

void checkpoint_0901(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            {
                if (!enter_sunyshore_pokemon_center(env.console, context, options)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "enter_sunyshore_pokemon_center: transition not detected.",
                        env.console);
                }
            }
            {
                if (!heal_and_exit(env.console, context, stats, "Sunyshore City")){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "heal_and_exit_1: transition not detected.",
                        env.console);
                }
            }
            context.wait_for_all_requests();
        },
        true);
}

std::string AutoStory_Checkpoint_0902::name() const {
    return "09.02 - Route 223 and Victory Road";
}

std::string AutoStory_Checkpoint_0902::start_text() const {
    return "Healed and outside Sunyshore Pokemon Center at the doorway.";
}

std::string AutoStory_Checkpoint_0902::end_text() const {
    return "Healed and outside Victory Road Pokemon Center after Jasmine and Route 223, Repel inactive.";
}

void AutoStory_Checkpoint_0902::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0902(env, context, options, stats);
}

void checkpoint_0902(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            {
                if (!go_to_victory_road_pokemon_center(env.console, context, stats, options)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "go_to_victory_road_pokemon_center: transition not detected.",
                        env.console);
                }
            }
            {
                if (!heal_and_exit(env.console, context, stats, "Victory Road")){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "heal_and_exit_2: transition not detected.",
                        env.console);
                }
            }
            context.wait_for_all_requests();
        },
        true);
}

std::string AutoStory_Checkpoint_0903::name() const {
    return "09.03 - Victory Road traversal";
}

std::string AutoStory_Checkpoint_0903::start_text() const {
    return "Healed and outside Victory Road Pokemon Center at the doorway, Repel inactive.";
}

std::string AutoStory_Checkpoint_0903::end_text() const {
    return "Inside the Pokemon League building at its arrival position, Repel inactive.";
}

void AutoStory_Checkpoint_0903::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0903(env, context, options, stats);
}

void checkpoint_0903(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            {
                if (!enter_victory_road_cave(env.console, context, options)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "enter_victory_road_cave: transition not detected.",
                        env.console);
                }
            }
            {
                if (!go_through_victory_road_cave(env.console, context, stats, options)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "go_through_victory_road_cave: transition not detected.",
                        env.console);
                }
            }
            {
                if (!enter_elite_4_pokemon_center(env.console, context, options)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "enter_elite_4_pokemon_center: transition not detected.",
                        env.console);
                }
            }
            context.wait_for_all_requests();
        },
        true);
}

std::string AutoStory_Checkpoint_0904::name() const {
    return "09.04 - League preparation and admission";
}

std::string AutoStory_Checkpoint_0904::start_text() const {
    return "Inside the Pokemon League building at its arrival position, Repel inactive.";
}

std::string AutoStory_Checkpoint_0904::end_text() const {
    return "Assisted route: rival defeated, healed, starter shopping completed, inside the first lift vestibule. Unassisted preparation is pending.";
}

void AutoStory_Checkpoint_0904::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0904(env, context, options, stats);
}

void checkpoint_0904(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            {
                if (!ready_in_front_of_elite_4(env.console, context, options, stats)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "League preparation failed; see the stage log.",
                        env.console);
                }
            }
            {
                if (!get_into_the_elite_4_entrance(env.console, context, options)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "get_into_the_elite_4_entrance: transition not detected.",
                        env.console);
                }
            }
            context.wait_for_all_requests();
        },
        true);
}

std::string AutoStory_Checkpoint_0905::name() const {
    return "09.05 - Aaron";
}

std::string AutoStory_Checkpoint_0905::start_text() const {
    return "Inside the first lift vestibule after League admission, facing north.";
}

std::string AutoStory_Checkpoint_0905::end_text() const {
    return "Aaron defeated, facing north in front of him, with dialogue and the door-opening cutscene finished.";
}

void AutoStory_Checkpoint_0905::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0905(env, context, options, stats);
}

void checkpoint_0905(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            {
                if (!deal_with_aaron_elite_4(env.console, context, stats, options.language)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "deal_with_aaron_elite_4: transition not detected.",
                        env.console);
                }
            }
            context.wait_for_all_requests();
        },
        true);
}

std::string AutoStory_Checkpoint_0906::name() const {
    return "09.06 - Bertha";
}

std::string AutoStory_Checkpoint_0906::start_text() const {
    return "Facing north in front of Aaron after his battle and dialogue.";
}

std::string AutoStory_Checkpoint_0906::end_text() const {
    return "Bertha defeated, facing north in front of her, with dialogue and the door-opening cutscene finished.";
}

void AutoStory_Checkpoint_0906::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0906(env, context, options, stats);
}

void checkpoint_0906(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            {
                if (!deal_with_bertha_elite_4(env.console, context, stats, options.language)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "deal_with_bertha_elite_4: transition not detected.",
                        env.console);
                }
            }
            context.wait_for_all_requests();
        },
        true);
}

std::string AutoStory_Checkpoint_0907::name() const {
    return "09.07 - Flint";
}

std::string AutoStory_Checkpoint_0907::start_text() const {
    return "Facing north in front of Bertha after her battle and dialogue.";
}

std::string AutoStory_Checkpoint_0907::end_text() const {
    return "Flint defeated, facing north in front of him, with dialogue and the door-opening cutscene finished.";
}

void AutoStory_Checkpoint_0907::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0907(env, context, options, stats);
}

void checkpoint_0907(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            {
                if (!deal_with_flint_elite_4(env.console, context, stats, options.language)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "deal_with_flint_elite_4: transition not detected.",
                        env.console);
                }
            }
            context.wait_for_all_requests();
        },
        true);
}

std::string AutoStory_Checkpoint_0908::name() const {
    return "09.08 - Lucian";
}

std::string AutoStory_Checkpoint_0908::start_text() const {
    return "Facing north in front of Flint after his battle and dialogue.";
}

std::string AutoStory_Checkpoint_0908::end_text() const {
    return "Lucian defeated, facing north in front of him, with dialogue and the door-opening cutscene finished.";
}

void AutoStory_Checkpoint_0908::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0908(env, context, options, stats);
}

void checkpoint_0908(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            {
                if (!deal_with_lucian_elite_4(env.console, context, stats, options.language)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "deal_with_lucian_elite_4: transition not detected.",
                        env.console);
                }
            }
            context.wait_for_all_requests();
        },
        true);
}

std::string AutoStory_Checkpoint_0909::name() const {
    return "09.09 - Cynthia and Hall of Fame";
}

std::string AutoStory_Checkpoint_0909::start_text() const {
    return "Facing north in front of Lucian after his battle and dialogue.";
}

std::string AutoStory_Checkpoint_0909::end_text() const {
    return "Cynthia defeated, Hall of Fame sequence advanced into credits, game restarted, and final save verified.";
}

void AutoStory_Checkpoint_0909::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0909(env, context, options, stats);
}

void checkpoint_0909(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            {
                if (!deal_with_cynthia_and_finish(env, context, options, stats)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "deal_with_cynthia_and_finish: transition not detected.",
                        env.console);
                }
            }
            context.wait_for_all_requests();
        },
        true);
}


std::string AutoStory_Checkpoint_0910::name() const {
    return "09.10 - Fly destination test";
}

std::string AutoStory_Checkpoint_0910::start_text() const {
    return "Outdoors with Fly and all 16 tested destinations unlocked. Select only 09.10; Route 221 is skipped.";
}

std::string AutoStory_Checkpoint_0910::end_text() const {
    return "Outside the upper Pokemon League entrance after all 16 flights. No checkpoint save or automatic retry.";
}

void AutoStory_Checkpoint_0910::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0910(env, context, options, stats);
}

void checkpoint_0910(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions /*options*/,
    AutoStoryStats& stats){
    // Manual diagnostic: preserve the failed flight for inspection instead of resetting.
    if (!test_fly_locations(env.console, context)){
        throw FatalProgramException(ErrorReportMode::NO_ERROR_REPORT,
            "09.10: Fly destination test failed; see the destination in the log.", env.console);
    }
    context.wait_for_all_requests();
    stats.m_checkpoint++;
    env.update_stats();
}

}
}
}
