/*  BDSP AutoStory - Segment 03
 *  From after acquiring the second badge until after acquiring the third badge.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include <chrono>
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_EndBattleDetector.h"
#include "../Detect/PokemonBDSP_AutoStory_OverworldDetector.h"
#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "../Utils/PokemonBDSP_AutoStory_Segment_00_tools.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "../PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP_AutoStory_Segment_03.h"

using namespace std::chrono_literals;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
            static void segment03_route_battle(VideoStream& stream, ProControllerContext& context,
                const std::string& label, bool post_battle_dialogue_cleanup = false) {
                fight_trainer_using_moves(stream,context,2,label,post_battle_dialogue_cleanup);
            }

    static void segment03_route_press(VideoStream& stream, ProControllerContext& context,
        Button button, Milliseconds hold, Milliseconds release) {
        context.wait_for_all_requests();
        auto frame=stream.video().snapshot();
        BattleMenuDetector menu(BattleType::TRAINER);
        if (frame && menu.detect(*frame.frame)) {
            segment03_route_battle(stream,context,"Recorded route encounter");
            return;
        }
        pbf_press_button(context,button,hold,release);
    }

    // Stop at the completed doorway transition, rather than spending the
    // remaining movement budget inside the next map.
    static void segment03_doorway(VideoStream& stream, ProControllerContext& context,
        bool north, std::chrono::milliseconds duration, const std::string& label, double horizontal = 0) {
        context.wait_for_all_requests();
        // Horizontal HQ stairs must cancel at fade-out, before loading can
        // swallow or carry movement onto the next floor. Vertical door callers
        // retain their existing transition behavior.
        if (horizontal != 0) {
            BlackScreenWatcher entering;
            if (run_until<ProControllerContext>(stream,context,
                [&](ProControllerContext& ctx) {
                    pbf_move_left_joystick(ctx,{horizontal,0},duration,0ms);
                    pbf_wait(ctx,3000ms);
                },{{entering}}) != 0)
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    label+": Stair transition did not start.",stream);
            BlackScreenWatcher visible(COLOR_RED,{0.1,0.1,0.8,0.8},
                100,10,BlackScreenWatcher::FinderType::GONE,500ms);
            if (wait_until(stream,context,30s,{{visible}}) != 0)
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    label+": Stair transition did not finish.",stream);
            pbf_wait(context,500ms);
            context.wait_for_all_requests();
            return;
        }
        BlackScreenOverWatcher transition(COLOR_RED,{0.1,0.1,0.8,0.8});
        int result=run_until<ProControllerContext>(stream,context,
            [&](ProControllerContext& ctx) {
                pbf_move_left_joystick(ctx,{0,north ? 1.0 : -1.0},duration,0ms);
                pbf_wait(ctx,3000ms);
            },{{transition}});
        if (result!=0) OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,label+": Doorway transition not confirmed.",stream);
        pbf_wait(context,500ms);
        context.wait_for_all_requests();
    }

    void eterna_gym_to_jupiter(VideoStream& stream, ProControllerContext& context) {
        auto fight=[&](const std::string& label, bool post_battle_dialogue_cleanup = false) {
            segment03_route_battle(stream,context,label+" (Flamethrower)",post_battle_dialogue_cleanup);
        };
                pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1060ms, 0ms);
                pbf_wait(context, 225ms);
                pbf_move_left_joystick(context, {0.000000, 1.000000}, 4232ms, 0ms);
                pbf_wait(context, 269ms);
                pbf_move_left_joystick(context, {-1.000000, -0.000000}, 287ms, 0ms);
                pbf_wait(context, 1777ms);
                pbf_move_left_joystick(context, {1.000000, -0.000000}, 367ms, 0ms);
                pbf_wait(context, 237ms);
                pbf_move_left_joystick(context, {0.000000, 1.000000}, 1140ms, 0ms);
                pbf_wait(context, 2621ms);
        if (!heal_pokemon(stream,context,"Eterna before Galactic HQ")) OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,"Eterna Center exit was not confirmed.",stream);
                pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1895ms, 0ms);
                pbf_wait(context, 130ms);
                pbf_move_left_joystick(context, {0.000000, 1.000000}, 710ms, 0ms);
                pbf_wait(context, 921ms);
                pbf_move_left_joystick(context, {0.000000, 1.000000}, 129ms, 0ms);
                pbf_wait(context, 354ms);
                pbf_move_left_joystick(context, {1.000000, -0.000000}, 813ms, 0ms);
                pbf_wait(context, 565ms);
                pbf_move_left_joystick(context, {0.000000, 1.000000}, 618ms, 0ms);
                pbf_wait(context, 483ms);
        context.wait_for_all_requests();
        // Align with the sign and Center emblem before trying Cut. Never search blindly or
        // keep pushing north against the hedge when the tree is east of us.
        size_t hq_cut_confirmations=0,hq_cut_missing=0;
        for (size_t correction=0;correction<20;correction++) {
            const auto frame=stream.video().snapshot();
            const auto position=frame ? match_galactic_hq_cut(*frame.frame) : OverworldMatchResult{};
            for (const auto& anchor:position.landmarks)
                stream.log("[Galactic HQ Cut] "+anchor.name+" score="+std::to_string(anchor.score)+
                    ", margin="+std::to_string(anchor.margin)+", offset=("+
                    std::to_string(anchor.dx)+","+std::to_string(anchor.dy)+")");
            if (!position.located) {
                hq_cut_confirmations=0;
                if (++hq_cut_missing>6)
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Galactic HQ: Front bush sign/Center alignment not confirmed.",stream);
                // Let a passing shadow clear; never search by moving blindly.
                pbf_wait(context,500ms);
                context.wait_for_all_requests();
                continue;
            }
            hq_cut_missing=0;
            if (std::abs(position.dx)>.20)
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Galactic HQ: Front bush alignment offset exceeds bounds.",stream);
            if (position.at_target) {
                if (++hq_cut_confirmations>=3) break;
            } else {
                hq_cut_confirmations=0;
                pbf_move_left_joystick(context,{position.dx>0 ? 1.0 : -1.0,0},
                    std::abs(position.dx)>1.0/24 ? 80ms : 40ms,0ms);
            }
            pbf_wait(context,250ms);
            context.wait_for_all_requests();
        }
        if (hq_cut_confirmations<3)
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Galactic HQ: Front bush column did not converge.",stream);
        // Some attempts reach the bush before the interaction accepts input.
        // Confirm its dialogue while stationary, then finish the Cut animation.
        ShortDialogWatcher hq_cut_dialogue;
        if (run_until<ProControllerContext>(stream,context,
            [](ProControllerContext& ctx) {
                for (size_t press=0; press<30; press++) {
                    pbf_move_left_joystick(ctx,{0,1},80ms,100ms);
                    pbf_press_button(ctx,BUTTON_A,150ms,500ms);
                }
            },{{hq_cut_dialogue}}) != 0)
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Galactic HQ: Front bush interaction did not produce dialogue.",stream);
        clear_post_battle_dialogue(stream,context,BUTTON_A,6000ms);
        context.wait_for_all_requests();
        // Cut succeeds, but the Center exit can leave us one tile west of
        // the HQ doorway. Do not replay the interior route against its wall.
        BlackScreenWatcher hq_entering;
        const int hq_entry = run_until<ProControllerContext>(stream,context,
            [&](ProControllerContext& ctx) {
                pbf_move_left_joystick(ctx,{0,1},1507ms,193ms);
                ctx.wait_for_all_requests();
                for (size_t correction=0; correction<4; correction++) {
                    stream.log("[Galactic HQ entry] No transition; correcting east toward doorway, attempt "+
                        std::to_string(correction+1)+"/4.");
                    pbf_move_left_joystick(ctx,{1,0},80ms,100ms);
                    pbf_move_left_joystick(ctx,{0,1},350ms,250ms);
                    ctx.wait_for_all_requests();
                }
                pbf_wait(ctx,1500ms);
            },{{hq_entering}});
        if (hq_entry != 0) OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            "Galactic HQ: Doorway transition not detected after bounded east correction.",stream);
        stream.log("[Galactic HQ entry] Doorway transition detected; waiting for interior.");
        BlackScreenWatcher hq_visible(COLOR_RED,{0.1,0.1,0.8,0.8},
            100,10,BlackScreenWatcher::FinderType::GONE,500ms);
        if (wait_until(stream,context,30s,{{hq_visible}}) != 0)
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Galactic HQ: Entry transition did not finish.",stream);
        pbf_wait(context,500ms);
        context.wait_for_all_requests();
        // galacticHQ.txt starts at the ground-floor entrance mat.
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 1247ms, 0ms);
        pbf_wait(context, 237ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 1246ms, 0ms);
        pbf_wait(context, 2989ms);
        pbf_press_button(context,BUTTON_A,150ms,250ms);
        fight("Galactic HQ entrance grunts",false);
        // With one usable Pokemon these grunts battle consecutively. The next
        // intro can outlast the ordinary dialogue quiet period. Keep movement
        // stopped and reuse the battle handler only if its command menu appears.
        // A party that fought both together needs no second battle handler.
        BattleMenuWatcher hq_next_grunt(BattleType::TRAINER);
        const int hq_next_battle = run_until<ProControllerContext>(stream,context,
            [](ProControllerContext& ctx) { pbf_mash_button(ctx,BUTTON_B,30000ms); },
            {{hq_next_grunt}});
        if (hq_next_battle == 0) {
            stream.log("[Galactic HQ] Second entrance grunt detected before stair movement.");
            fight("Galactic HQ second entrance grunt",false);
        }
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 580ms, 0ms);
        pbf_wait(context, 220ms);
        segment03_doorway(stream,context,true,1077ms,"Galactic HQ stairs 1F to 2F",-1.0);
        pbf_wait(context, 1145ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 758ms, 0ms);
        pbf_wait(context, 487ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 1633ms, 0ms);
        pbf_wait(context, 254ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 925ms, 0ms);
        pbf_wait(context, 188ms);
        segment03_doorway(stream,context,true,1145ms,"Galactic HQ stairs 2F to 3F",-1.0);
        pbf_wait(context, 1455ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 410ms, 0ms);
        pbf_wait(context, 679ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 744ms, 0ms);
        pbf_wait(context, 2084ms);
        pbf_press_button(context,BUTTON_A,150ms,250ms);
        fight("Galactic HQ upper-floor grunt",false);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 2119ms, 0ms);
        pbf_wait(context, 903ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 263ms, 0ms);
        pbf_wait(context, 984ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 342ms, 0ms);
        pbf_wait(context, 1806ms);
        pbf_press_button(context,BUTTON_A,150ms,250ms);
        fight("Galactic HQ Scientist",false);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 425ms, 0ms);
        pbf_wait(context, 161ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 723ms, 0ms);
        pbf_wait(context, 1639ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 494ms, 0ms);
        pbf_wait(context, 678ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 409ms, 0ms);
        pbf_wait(context, 858ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 389ms, 0ms);
        pbf_wait(context, 1189ms);
        // Align directly above Jupiter, then turn south to interact without
        // trying to enter the narrow gap between her and the rescued NPC.
        align_to_overworld_landmarks(stream,context,match_galactic_hq_jupiter,
            "Galactic HQ Jupiter",false,true);
        // Use an explicit Down press: the 16ms stick pulse can leave the
        // previous right-facing direction active and interact with Clefairy.
        pbf_press_dpad(context,DPAD_DOWN,80ms,250ms);
        context.wait_for_all_requests();
        // Advance the introduction before entering the battle handler's
        // B-based waiting loop. A single press may only open the first text.
        BattleMenuWatcher jupiter_menu(BattleType::TRAINER);
        if (run_until<ProControllerContext>(stream,context,
            [](ProControllerContext& ctx) {
                for (size_t press=0;press<30;press++)
                    pbf_press_button(ctx,BUTTON_A,150ms,500ms);
                pbf_wait(ctx,15000ms);
            },{{jupiter_menu}}) != 0)
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Galactic HQ: Jupiter interaction did not reach the battle menu.",stream);
        fight("Commander Jupiter",true);
        // Jupiter's departure and the rescued family have chained dialogue.
        clear_post_battle_dialogue(stream,context,BUTTON_A,6000ms);
        stream.log("[Galactic HQ] Rescue dialogue cleared; following recorded exit.",COLOR_GREEN);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 691ms, 0ms);
        pbf_wait(context, 575ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 330ms, 0ms);
        pbf_wait(context, 2575ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 523ms, 0ms);
        pbf_wait(context, 239ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 717ms, 0ms);
        pbf_wait(context, 1721ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 306ms, 0ms);
        pbf_wait(context, 294ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1041ms, 0ms);
        pbf_wait(context, 307ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 431ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, 1.000000}, 11ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 999ms, 0ms);
        pbf_wait(context, 573ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 343ms, 0ms);
        pbf_wait(context, 175ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 999ms, 0ms);
        pbf_wait(context, 456ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 277ms, 0ms);
        pbf_wait(context, 122ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 654ms, 0ms);
        pbf_wait(context, 1092ms);
        context.wait_for_all_requests();
        BlackScreenOverWatcher hq_leaving(COLOR_RED,{0.1,0.1,0.8,0.8});
        if (run_until<ProControllerContext>(stream,context,
            [](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx,{0,-1},2350ms,0ms);
                pbf_move_left_joystick(ctx,{-1,-1},2744ms,0ms);
                pbf_wait(ctx,3000ms);
            },{{hq_leaving}}) != 0)
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Galactic HQ: Exit transition not confirmed.",stream);
        // Clamp against the uncut front bush; checkpoint 03.02 starts here.
        pbf_move_left_joystick(context,{0,-1},350ms,500ms);
        context.wait_for_all_requests();
        stream.log("[Galactic HQ] Outside, facing the front Cut bush.",COLOR_GREEN);
    }
    static void eterna_mart_escape_ropes(VideoStream& stream, ProControllerContext& context) {
        stream.log("[Eterna Mart] Buying exactly 3 Escape Ropes after the Jupiter heal.");
        // Recorded Center-exit to Mart-door path (eternamart1.txt).
        pbf_move_left_joystick(context,{0,-1},2713ms,0ms);
        pbf_wait(context,444ms);
        pbf_move_left_joystick(context,{1,0},587ms,0ms);
        pbf_wait(context,705ms);
        segment03_doorway(stream,context,true,896ms,"Eterna Mart entrance");
        // Standard Mart layout: walk north from the entrance to the clerk.
        pbf_move_left_joystick(context,{0,1},885ms,300ms);
        if (!buy_items(stream,context,{{ShopItem::EscapeRope,3}}))
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Eterna Mart: Could not buy 3 Escape Ropes.",stream);
        segment03_doorway(stream,context,false,5000ms,"Eterna Mart exit");
        clear_new_item_flags(stream,context);

        // Retrace to the Center column, then use its doorway to normalize the
        // position. Do not reverse the time spent loading the Mart interior.
        pbf_move_left_joystick(context,{0,-1},896ms,100ms);
        pbf_move_left_joystick(context,{-1,0},587ms,100ms);
        segment03_doorway(stream,context,true,5000ms,"Eterna Center return after Mart");
        segment03_doorway(stream,context,false,5000ms,"Eterna Center exit after Mart");
    }

    void jupiter_to_coronet(VideoStream& stream, ProControllerContext& context) {
        // Continue from 03.01 outside HQ, directly in front of the Cut bush.
        segment03_route_press(stream,context, BUTTON_A, 232ms, 0ms);
        pbf_wait(context, 314ms);
        segment03_route_press(stream,context, BUTTON_A, 199ms, 0ms);
        pbf_wait(context, 129ms);
        segment03_route_press(stream,context, BUTTON_A, 163ms, 0ms);
        pbf_wait(context, 116ms);
        segment03_route_press(stream,context, BUTTON_A, 174ms, 0ms);
        pbf_wait(context, 173ms);
        segment03_route_press(stream,context, BUTTON_A, 165ms, 0ms);
        pbf_wait(context, 4174ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 1413ms, 0ms);
        pbf_wait(context, 182ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 712ms, 0ms);
        pbf_wait(context, 91ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 1126ms, 0ms);
        pbf_wait(context, 27ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 745ms, 0ms);
        pbf_wait(context, 868ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 866ms, 0ms);
        if (!heal_pokemon(stream,context,"Eterna after Jupiter")) OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,"Eterna Center exit not confirmed.",stream);
        eterna_mart_escape_ropes(stream,context);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 1514ms, 0ms);
        pbf_wait(context, 177ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 587ms, 0ms);
        pbf_wait(context, 522ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 144ms, 0ms);
        pbf_wait(context, 439ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 1078ms, 0ms);
        pbf_wait(context, 3295ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 505ms, 0ms);
        pbf_wait(context, 405ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 568ms, 0ms);
        pbf_wait(context, 403ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 642ms, 0ms);
        pbf_wait(context, 14ms);
        segment03_route_press(stream,context, BUTTON_A, 186ms, 0ms);
        pbf_wait(context, 612ms);
        segment03_route_press(stream,context, BUTTON_A, 211ms, 0ms);
        pbf_wait(context, 190ms);
        segment03_route_press(stream,context, BUTTON_A, 168ms, 0ms);
        pbf_wait(context, 376ms);
        segment03_route_press(stream,context, BUTTON_A, 198ms, 0ms);
        pbf_wait(context, 122ms);
        segment03_route_press(stream,context, BUTTON_A, 158ms, 0ms);
        pbf_wait(context, 242ms);
        segment03_route_press(stream,context, BUTTON_A, 174ms, 0ms);
        pbf_wait(context, 472ms);
        segment03_route_press(stream,context, BUTTON_A, 126ms, 0ms);
        pbf_wait(context, 8007ms);
        segment03_route_press(stream,context, BUTTON_A, 168ms, 0ms);
        pbf_wait(context, 83ms);
        segment03_route_press(stream,context, BUTTON_A, 143ms, 0ms);
        pbf_wait(context, 1005ms);
        segment03_route_press(stream,context, BUTTON_A, 101ms, 0ms);
        pbf_wait(context, 268ms);
        segment03_route_press(stream,context, BUTTON_A, 109ms, 0ms);
        pbf_wait(context, 123ms);
        segment03_route_press(stream,context, BUTTON_A, 112ms, 0ms);
        pbf_wait(context, 539ms);
        segment03_route_press(stream,context, BUTTON_A, 147ms, 0ms);
        pbf_wait(context, 518ms);
        segment03_route_press(stream,context, BUTTON_A, 160ms, 0ms);
        pbf_wait(context, 484ms);
        segment03_route_press(stream,context, BUTTON_A, 185ms, 0ms);
        pbf_wait(context, 613ms);
        segment03_route_press(stream,context, BUTTON_A, 168ms, 0ms);
        pbf_wait(context, 526ms);
        segment03_route_press(stream,context, BUTTON_A, 192ms, 0ms);
        pbf_wait(context, 755ms);
        segment03_route_press(stream,context, BUTTON_A, 196ms, 0ms);
        pbf_wait(context, 962ms);
        segment03_route_press(stream,context, BUTTON_A, 253ms, 0ms);
        pbf_wait(context, 323ms);
        segment03_route_press(stream,context, BUTTON_A, 277ms, 0ms);
        pbf_wait(context, 506ms);
        segment03_route_press(stream,context, BUTTON_A, 253ms, 0ms);
        pbf_wait(context, 313ms);
        segment03_route_press(stream,context, BUTTON_A, 170ms, 0ms);
        pbf_wait(context, 481ms);
        segment03_route_press(stream,context, BUTTON_A, 196ms, 0ms);
        pbf_wait(context, 737ms);
        segment03_route_press(stream,context, BUTTON_A, 211ms, 0ms);
        pbf_wait(context, 589ms);
        segment03_route_press(stream,context, BUTTON_A, 211ms, 0ms);
        pbf_wait(context, 296ms);
        segment03_route_press(stream,context, BUTTON_A, 144ms, 0ms);
        pbf_wait(context, 588ms);
        segment03_route_press(stream,context, BUTTON_A, 189ms, 0ms);
        pbf_wait(context, 425ms);
        segment03_route_press(stream,context, BUTTON_A, 210ms, 0ms);
        pbf_wait(context, 422ms);
        segment03_route_press(stream,context, BUTTON_A, 193ms, 0ms);
        pbf_wait(context, 1007ms);
        segment03_route_press(stream,context, BUTTON_A, 235ms, 0ms);
        pbf_wait(context, 333ms);
        segment03_route_press(stream,context, BUTTON_A, 198ms, 0ms);
        pbf_wait(context, 5044ms);
        segment03_route_press(stream,context, BUTTON_A, 208ms, 0ms);
        pbf_wait(context, 2822ms);
        segment03_route_press(stream,context, BUTTON_A, 210ms, 0ms);
        pbf_wait(context, 4173ms);
        segment03_route_press(stream,context, BUTTON_A, 119ms, 0ms);
        pbf_wait(context, 482ms);
        segment03_route_press(stream,context, BUTTON_B, 204ms, 0ms);
        pbf_wait(context, 1087ms);
        segment03_route_press(stream,context, BUTTON_X, 180ms, 0ms);
        pbf_wait(context, 1306ms);
        segment03_route_press(stream,context, BUTTON_A, 141ms, 0ms);
        pbf_wait(context, 1456ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 285ms, 0ms);
        pbf_wait(context, 741ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 206ms, 0ms);
        pbf_wait(context, 209ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 194ms, 0ms);
        pbf_wait(context, 869ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 181ms, 0ms);
        pbf_wait(context, 306ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 188ms, 0ms);
        pbf_wait(context, 361ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 154ms, 0ms);
        pbf_wait(context, 1911ms);
        segment03_route_press(stream,context, BUTTON_Y, 156ms, 0ms);
        pbf_wait(context, 2929ms);
        pbf_move_right_joystick(context, {1.000000, -0.000000}, 205ms, 0ms);
        pbf_wait(context, 898ms);
        pbf_move_right_joystick(context, {-1.000000, -0.000000}, 255ms, 0ms);
        pbf_wait(context, 469ms);
        segment03_route_press(stream,context, BUTTON_A, 196ms, 0ms);
        pbf_wait(context, 508ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 177ms, 0ms);
        pbf_wait(context, 274ms);
        segment03_route_press(stream,context, BUTTON_A, 182ms, 0ms);
        pbf_wait(context, 1400ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 207ms, 0ms);
        pbf_wait(context, 663ms);
        segment03_route_press(stream,context, BUTTON_A, 163ms, 0ms);
        pbf_wait(context, 1097ms);
        segment03_route_press(stream,context, BUTTON_B, 178ms, 0ms);
        pbf_wait(context, 742ms);
        segment03_route_press(stream,context, BUTTON_B, 182ms, 0ms);
        pbf_wait(context, 872ms);
        segment03_route_press(stream,context, BUTTON_B, 171ms, 0ms);
        pbf_wait(context, 774ms);
        if (!clear_new_item_flags(stream,context)) OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,"New reward flags could not be cleared before continuing.",stream);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 733ms, 0ms);
        pbf_wait(context, 316ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 3280ms, 0ms);
        pbf_wait(context, 1944ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 1789ms, 0ms);
        pbf_wait(context, 1848ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 3291ms, 0ms);
        pbf_wait(context, 2531ms);
        segment03_route_battle(stream,context,"Cycling Road first trainer");
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 676ms, 0ms);
        pbf_wait(context, 2016ms);
        segment03_route_battle(stream,context,"Cycling Road second trainer");
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 145ms, 0ms);
        pbf_wait(context, 1136ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 424ms, 0ms);
        pbf_wait(context, 374ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 160ms, 0ms);
        pbf_wait(context, 895ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 224ms, 0ms);
        pbf_wait(context, 643ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 838ms, 0ms);
        pbf_wait(context, 1761ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 1839ms, 0ms);
        pbf_wait(context, 1407ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 3343ms, 0ms);
        pbf_wait(context, 3457ms);
        segment03_route_press(stream,context, BUTTON_A, 233ms, 0ms);
        pbf_wait(context, 135ms);
        segment03_route_press(stream,context, BUTTON_A, 175ms, 0ms);
        pbf_wait(context, 159ms);
        segment03_route_press(stream,context, BUTTON_A, 91ms, 0ms);
        pbf_wait(context, 340ms);
        segment03_route_press(stream,context, BUTTON_A, 180ms, 0ms);
        pbf_wait(context, 243ms);
        segment03_route_press(stream,context, BUTTON_A, 223ms, 0ms);
        pbf_wait(context, 375ms);
        segment03_route_press(stream,context, BUTTON_A, 346ms, 0ms);
        pbf_wait(context, 1168ms);
        segment03_route_press(stream,context, BUTTON_A, 232ms, 0ms);
        pbf_wait(context, 589ms);
        segment03_route_press(stream,context, BUTTON_A, 211ms, 0ms);
        pbf_wait(context, 550ms);
        segment03_route_press(stream,context, BUTTON_A, 195ms, 0ms);
        pbf_wait(context, 871ms);
        segment03_route_press(stream,context, BUTTON_A, 195ms, 0ms);
        pbf_wait(context, 956ms);
        segment03_route_press(stream,context, BUTTON_A, 193ms, 0ms);
        pbf_wait(context, 295ms);
        segment03_route_press(stream,context, BUTTON_B, 172ms, 0ms);
        pbf_wait(context, 206ms);
        segment03_route_press(stream,context, BUTTON_B, 193ms, 0ms);
        pbf_wait(context, 175ms);
        segment03_route_press(stream,context, BUTTON_B, 175ms, 0ms);
        pbf_wait(context, 307ms);
        segment03_route_press(stream,context, BUTTON_B, 181ms, 0ms);
        pbf_wait(context, 325ms);
        segment03_route_press(stream,context, BUTTON_B, 144ms, 0ms);
        pbf_wait(context, 417ms);
        segment03_route_press(stream,context, BUTTON_B, 196ms, 0ms);
        pbf_wait(context, 309ms);
        segment03_route_press(stream,context, BUTTON_B, 173ms, 0ms);
        pbf_wait(context, 827ms);
        segment03_route_press(stream,context, BUTTON_B, 239ms, 0ms);
        pbf_wait(context, 473ms);
        segment03_route_press(stream,context, BUTTON_B, 212ms, 0ms);
        pbf_wait(context, 321ms);
        segment03_route_press(stream,context, BUTTON_B, 128ms, 0ms);
        pbf_wait(context, 426ms);
        segment03_route_press(stream,context, BUTTON_B, 158ms, 0ms);
        pbf_wait(context, 488ms);
        segment03_route_press(stream,context, BUTTON_B, 231ms, 0ms);
        pbf_wait(context, 724ms);
        segment03_route_press(stream,context, BUTTON_B, 210ms, 0ms);
        pbf_wait(context, 404ms);
        segment03_route_press(stream,context, BUTTON_B, 109ms, 0ms);
        pbf_wait(context, 357ms);
        segment03_route_press(stream,context, BUTTON_B, 199ms, 0ms);
        pbf_wait(context, 416ms);
        segment03_route_press(stream,context, BUTTON_B, 196ms, 0ms);
        pbf_wait(context, 175ms);
        segment03_route_press(stream,context, BUTTON_B, 189ms, 0ms);
        pbf_wait(context, 345ms);
        segment03_route_press(stream,context, BUTTON_B, 189ms, 0ms);
        pbf_wait(context, 277ms);
        segment03_route_press(stream,context, BUTTON_B, 212ms, 0ms);
        pbf_wait(context, 368ms);
        segment03_route_press(stream,context, BUTTON_B, 232ms, 0ms);
        pbf_wait(context, 189ms);
        segment03_route_press(stream,context, BUTTON_B, 172ms, 0ms);
        pbf_wait(context, 474ms);
        segment03_route_press(stream,context, BUTTON_B, 211ms, 0ms);
        pbf_wait(context, 504ms);
        segment03_route_press(stream,context, BUTTON_B, 228ms, 0ms);
        pbf_wait(context, 372ms);
        segment03_route_press(stream,context, BUTTON_B, 162ms, 0ms);
        pbf_wait(context, 4324ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 360ms, 0ms);
        pbf_wait(context, 188ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 944ms, 0ms);
        pbf_wait(context, 161ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 388ms, 0ms);
        pbf_wait(context, 39ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 2236ms, 0ms);
        pbf_wait(context, 1820ms);
        segment03_route_battle(stream,context,"Route 207 Camper");
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 2758ms, 0ms);
        pbf_wait(context, 94ms);
        segment03_route_battle(stream,context,"Route 207 Hiker");
        context.wait_for_all_requests();
    }
    void coronet1(VideoStream& stream, ProControllerContext& context) {
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 1093ms, 0ms);
        pbf_wait(context, 832ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 178ms, 0ms);
        pbf_wait(context, 305ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 745ms, 0ms);
        pbf_wait(context, 2805ms);
        segment03_route_press(stream,context, BUTTON_X, 163ms, 0ms);
        pbf_wait(context, 837ms);
        segment03_route_press(stream,context, BUTTON_A, 166ms, 0ms);
        pbf_wait(context, 2708ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 171ms, 0ms);
        pbf_wait(context, 308ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 185ms, 0ms);
        pbf_wait(context, 967ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 196ms, 0ms);
        pbf_wait(context, 52ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 182ms, 0ms);
        pbf_wait(context, 507ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 138ms, 0ms);
        pbf_wait(context, 1004ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 179ms, 0ms);
        pbf_wait(context, 844ms);
        segment03_route_press(stream,context, BUTTON_A, 192ms, 0ms);
        pbf_wait(context, 487ms);
        segment03_route_press(stream,context, BUTTON_A, 193ms, 0ms);
        pbf_wait(context, 677ms);
        segment03_route_press(stream,context, BUTTON_A, 176ms, 0ms);
        pbf_wait(context, 989ms);
        segment03_route_press(stream,context, BUTTON_B, 179ms, 0ms);
        pbf_wait(context, 736ms);
        segment03_route_press(stream,context, BUTTON_B, 186ms, 0ms);
        pbf_wait(context, 551ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 2190ms, 0ms);
        pbf_wait(context, 104ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 852ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, 1.000000}, 37ms, 0ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 102ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, 1.000000}, 78ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 983ms, 0ms);
        pbf_wait(context, 1754ms);
        segment03_route_press(stream,context, BUTTON_A, 173ms, 0ms);
        pbf_wait(context, 1072ms);
        segment03_route_press(stream,context, BUTTON_A, 168ms, 0ms);
        pbf_wait(context, 155ms);
        segment03_route_press(stream,context, BUTTON_A, 142ms, 0ms);
        pbf_wait(context, 136ms);
        segment03_route_press(stream,context, BUTTON_A, 159ms, 0ms);
        pbf_wait(context, 1598ms);
        segment03_route_press(stream,context, BUTTON_A, 244ms, 0ms);
        pbf_wait(context, 148ms);
        segment03_route_press(stream,context, BUTTON_A, 170ms, 0ms);
        pbf_wait(context, 587ms);
        segment03_route_press(stream,context, BUTTON_A, 211ms, 0ms);
        pbf_wait(context, 135ms);
        segment03_route_press(stream,context, BUTTON_A, 160ms, 0ms);
        pbf_wait(context, 321ms);
        segment03_route_press(stream,context, BUTTON_A, 211ms, 0ms);
        pbf_wait(context, 205ms);
        segment03_route_press(stream,context, BUTTON_A, 180ms, 0ms);
        pbf_wait(context, 672ms);
        segment03_route_press(stream,context, BUTTON_A, 182ms, 0ms);
        pbf_wait(context, 122ms);
        segment03_route_press(stream,context, BUTTON_A, 172ms, 0ms);
        pbf_wait(context, 362ms);
        segment03_route_press(stream,context, BUTTON_A, 172ms, 0ms);
        pbf_wait(context, 373ms);
        segment03_route_press(stream,context, BUTTON_A, 199ms, 0ms);
        pbf_wait(context, 150ms);
        segment03_route_press(stream,context, BUTTON_A, 183ms, 0ms);
        pbf_wait(context, 190ms);
        segment03_route_press(stream,context, BUTTON_A, 188ms, 0ms);
        pbf_wait(context, 213ms);
        segment03_route_press(stream,context, BUTTON_A, 143ms, 0ms);
        pbf_wait(context, 7001ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 999ms, 0ms);
        pbf_wait(context, 268ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 375ms, 0ms);
        pbf_wait(context, 227ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 741ms, 0ms);
        pbf_wait(context, 46ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 743ms, 0ms);
        pbf_wait(context, 68ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 265ms, 0ms);
        pbf_wait(context, 255ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 890ms, 0ms);
        pbf_wait(context, 88ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 462ms, 0ms);
        pbf_wait(context, 271ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 647ms, 0ms);
        pbf_wait(context, 20ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 1047ms, 0ms);
        pbf_wait(context, 56ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 776ms, 0ms);
        pbf_wait(context, 2038ms);
        context.wait_for_all_requests();
    }
    void coronet_to_heartholme(VideoStream& stream, ProControllerContext& context) {
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 4828ms, 0ms);
        pbf_wait(context, 104ms);
        segment03_route_battle(stream,context,"Route 208 Hiker");
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 421ms, 0ms);
        pbf_wait(context, 178ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 1436ms, 0ms);
        pbf_wait(context, 81ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 478ms, 0ms);
        pbf_wait(context, 298ms);
        segment03_route_press(stream,context, BUTTON_A, 218ms, 0ms);
        pbf_wait(context, 274ms);
        segment03_route_press(stream,context, BUTTON_A, 182ms, 0ms);
        pbf_wait(context, 611ms);
        segment03_route_press(stream,context, BUTTON_A, 180ms, 0ms);
        pbf_wait(context, 359ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 189ms, 0ms);
        pbf_wait(context, 98ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 719ms, 0ms);
        pbf_wait(context, 55ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 1300ms, 0ms);
        pbf_wait(context, 432ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 1453ms, 0ms);
        pbf_wait(context, 522ms);
        segment03_route_battle(stream,context,"Aroma Lady Hannah");
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 1252ms, 0ms);
        pbf_wait(context, 491ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 2576ms, 0ms);
        pbf_wait(context, 211ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 282ms, 0ms);
        pbf_wait(context, 198ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 602ms, 0ms);
        pbf_wait(context, 1732ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 1878ms, 0ms);
        pbf_wait(context, 1588ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 2201ms, 0ms);
        pbf_wait(context, 6189ms);
        segment03_route_press(stream,context, BUTTON_A, 276ms, 0ms);
        pbf_wait(context, 201ms);
        segment03_route_press(stream,context, BUTTON_A, 111ms, 0ms);
        pbf_wait(context, 1090ms);
        segment03_route_press(stream,context, BUTTON_A, 198ms, 0ms);
        pbf_wait(context, 218ms);
        segment03_route_press(stream,context, BUTTON_A, 182ms, 0ms);
        pbf_wait(context, 255ms);
        segment03_route_press(stream,context, BUTTON_A, 238ms, 0ms);
        pbf_wait(context, 272ms);
        segment03_route_press(stream,context, BUTTON_A, 199ms, 0ms);
        pbf_wait(context, 171ms);
        segment03_route_press(stream,context, BUTTON_A, 202ms, 0ms);
        pbf_wait(context, 451ms);
        segment03_route_press(stream,context, BUTTON_A, 213ms, 0ms);
        pbf_wait(context, 267ms);
        segment03_route_press(stream,context, BUTTON_A, 199ms, 0ms);
        pbf_wait(context, 482ms);
        segment03_route_press(stream,context, BUTTON_A, 227ms, 0ms);
        pbf_wait(context, 813ms);
        segment03_route_press(stream,context, BUTTON_A, 186ms, 0ms);
        pbf_wait(context, 427ms);
        segment03_route_press(stream,context, BUTTON_A, 210ms, 0ms);
        pbf_wait(context, 243ms);
        segment03_route_press(stream,context, BUTTON_A, 187ms, 0ms);
        pbf_wait(context, 191ms);
        segment03_route_press(stream,context, BUTTON_A, 194ms, 0ms);
        pbf_wait(context, 190ms);
        segment03_route_press(stream,context, BUTTON_A, 212ms, 0ms);
        pbf_wait(context, 3054ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 3448ms, 0ms);
        pbf_wait(context, 437ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 2292ms, 0ms);
        pbf_wait(context, 1582ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1125ms, 0ms);
        pbf_wait(context, 334ms);
        segment03_route_press(stream,context, BUTTON_B, 177ms, 0ms);
        pbf_wait(context, 357ms);
        segment03_route_press(stream,context, BUTTON_B, 216ms, 0ms);
        pbf_wait(context, 400ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 2228ms, 0ms);
        pbf_wait(context, 304ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 279ms, 0ms);
        pbf_wait(context, 174ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 498ms, 0ms);
        pbf_move_left_joystick(context, {-1.000000, 1.000000}, 76ms, 0ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 1452ms, 0ms);
        pbf_wait(context, 21ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 648ms, 0ms);
        pbf_wait(context, 221ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1161ms, 0ms);
        pbf_wait(context, 59ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 533ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, 1.000000}, 96ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 81ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, 1.000000}, 37ms, 0ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 941ms, 0ms);
        pbf_wait(context, 2095ms);
        if (!heal_pokemon(stream,context,"Hearthome")) OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,"Hearthome Center exit not confirmed.",stream);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 2092ms, 0ms);
        pbf_wait(context, 185ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 1004ms, 0ms);
        pbf_wait(context, 62ms);
        segment03_route_press(stream,context, BUTTON_A, 194ms, 0ms);
        pbf_wait(context, 277ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 131ms, 0ms);
        pbf_wait(context, 70ms);
        segment03_route_press(stream,context, BUTTON_A, 190ms, 0ms);
        pbf_wait(context, 512ms);
        segment03_route_press(stream,context, BUTTON_A, 180ms, 0ms);
        pbf_wait(context, 169ms);
        segment03_route_press(stream,context, BUTTON_A, 155ms, 0ms);
        pbf_wait(context, 150ms);
        segment03_route_press(stream,context, BUTTON_A, 167ms, 0ms);
        pbf_wait(context, 129ms);
        segment03_route_press(stream,context, BUTTON_A, 170ms, 0ms);
        pbf_wait(context, 179ms);
        segment03_route_press(stream,context, BUTTON_A, 162ms, 0ms);
        pbf_wait(context, 323ms);
        segment03_route_press(stream,context, BUTTON_A, 207ms, 0ms);
        pbf_wait(context, 160ms);
        segment03_route_press(stream,context, BUTTON_A, 195ms, 0ms);
        pbf_wait(context, 287ms);
        segment03_route_press(stream,context, BUTTON_A, 206ms, 0ms);
        pbf_wait(context, 188ms);
        segment03_route_press(stream,context, BUTTON_A, 176ms, 0ms);
        pbf_wait(context, 233ms);
        segment03_route_press(stream,context, BUTTON_A, 243ms, 0ms);
        pbf_wait(context, 381ms);
        segment03_route_press(stream,context, BUTTON_A, 220ms, 0ms);
        pbf_wait(context, 287ms);
        segment03_route_press(stream,context, BUTTON_A, 197ms, 0ms);
        pbf_wait(context, 221ms);
        segment03_route_press(stream,context, BUTTON_A, 205ms, 0ms);
        pbf_wait(context, 191ms);
        segment03_route_press(stream,context, BUTTON_A, 165ms, 0ms);
        pbf_wait(context, 4201ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 1179ms, 0ms);
        pbf_wait(context, 4268ms);
        segment03_route_press(stream,context, BUTTON_A, 181ms, 0ms);
        pbf_wait(context, 219ms);
        segment03_route_press(stream,context, BUTTON_A, 195ms, 0ms);
        pbf_wait(context, 123ms);
        segment03_route_press(stream,context, BUTTON_A, 185ms, 0ms);
        pbf_wait(context, 121ms);
        segment03_route_press(stream,context, BUTTON_A, 170ms, 0ms);
        pbf_wait(context, 805ms);
        segment03_route_press(stream,context, BUTTON_A, 195ms, 0ms);
        pbf_wait(context, 140ms);
        segment03_route_press(stream,context, BUTTON_A, 102ms, 0ms);
        pbf_wait(context, 218ms);
        segment03_route_press(stream,context, BUTTON_B, 181ms, 0ms);
        pbf_wait(context, 127ms);
        segment03_route_press(stream,context, BUTTON_B, 167ms, 0ms);
        pbf_wait(context, 539ms);
        segment03_route_press(stream,context, BUTTON_B, 179ms, 0ms);
        pbf_wait(context, 246ms);
        segment03_route_press(stream,context, BUTTON_B, 211ms, 0ms);
        pbf_wait(context, 363ms);
        segment03_route_press(stream,context, BUTTON_B, 182ms, 0ms);
        pbf_wait(context, 176ms);
        segment03_route_press(stream,context, BUTTON_B, 237ms, 0ms);
        pbf_wait(context, 163ms);
        segment03_route_press(stream,context, BUTTON_B, 211ms, 0ms);
        pbf_wait(context, 123ms);
        segment03_route_press(stream,context, BUTTON_B, 186ms, 0ms);
        pbf_wait(context, 173ms);
        segment03_route_press(stream,context, BUTTON_B, 212ms, 0ms);
        pbf_wait(context, 121ms);
        segment03_route_press(stream,context, BUTTON_B, 196ms, 0ms);
        pbf_wait(context, 337ms);
        segment03_route_press(stream,context, BUTTON_B, 228ms, 0ms);
        pbf_wait(context, 149ms);
        segment03_route_press(stream,context, BUTTON_B, 236ms, 0ms);
        pbf_wait(context, 138ms);
        segment03_route_press(stream,context, BUTTON_B, 194ms, 0ms);
        pbf_wait(context, 139ms);
        segment03_route_press(stream,context, BUTTON_B, 195ms, 0ms);
        pbf_wait(context, 162ms);
        segment03_route_press(stream,context, BUTTON_B, 188ms, 0ms);
        pbf_wait(context, 571ms);
        segment03_route_press(stream,context, BUTTON_B, 229ms, 0ms);
        pbf_wait(context, 122ms);
        segment03_route_press(stream,context, BUTTON_B, 211ms, 0ms);
        pbf_wait(context, 122ms);
        segment03_route_press(stream,context, BUTTON_B, 181ms, 0ms);
        pbf_wait(context, 176ms);
        segment03_route_press(stream,context, BUTTON_B, 170ms, 0ms);
        pbf_wait(context, 175ms);
        segment03_route_press(stream,context, BUTTON_B, 203ms, 0ms);
        pbf_wait(context, 212ms);
        segment03_route_press(stream,context, BUTTON_B, 209ms, 0ms);
        pbf_wait(context, 274ms);
        segment03_route_press(stream,context, BUTTON_B, 210ms, 0ms);
        pbf_wait(context, 214ms);
        segment03_route_press(stream,context, BUTTON_B, 186ms, 0ms);
        pbf_wait(context, 547ms);
        segment03_route_press(stream,context, BUTTON_B, 212ms, 0ms);
        pbf_wait(context, 1964ms);
        segment03_route_press(stream,context, BUTTON_B, 195ms, 0ms);
        pbf_wait(context, 139ms);
        segment03_route_press(stream,context, BUTTON_B, 211ms, 0ms);
        pbf_wait(context, 279ms);
        segment03_route_press(stream,context, BUTTON_B, 210ms, 0ms);
        pbf_wait(context, 350ms);
        segment03_route_press(stream,context, BUTTON_B, 250ms, 0ms);
        pbf_wait(context, 150ms);
        segment03_route_press(stream,context, BUTTON_B, 196ms, 0ms);
        pbf_wait(context, 488ms);
        segment03_route_press(stream,context, BUTTON_B, 209ms, 0ms);
        pbf_wait(context, 225ms);
        segment03_route_press(stream,context, BUTTON_B, 242ms, 0ms);
        pbf_wait(context, 347ms);
        segment03_route_press(stream,context, BUTTON_B, 237ms, 0ms);
        pbf_wait(context, 232ms);
        segment03_route_press(stream,context, BUTTON_B, 210ms, 0ms);
        pbf_wait(context, 400ms);
        segment03_route_press(stream,context, BUTTON_B, 226ms, 0ms);
        pbf_wait(context, 162ms);
        segment03_route_press(stream,context, BUTTON_B, 145ms, 0ms);
        pbf_wait(context, 365ms);
        segment03_route_press(stream,context, BUTTON_B, 209ms, 0ms);
        pbf_wait(context, 560ms);
        segment03_route_press(stream,context, BUTTON_B, 224ms, 0ms);
        pbf_wait(context, 540ms);
        segment03_route_press(stream,context, BUTTON_B, 210ms, 0ms);
        pbf_wait(context, 625ms);
        segment03_route_press(stream,context, BUTTON_B, 198ms, 0ms);
        pbf_wait(context, 218ms);
        segment03_route_press(stream,context, BUTTON_B, 183ms, 0ms);
        pbf_wait(context, 432ms);
        segment03_route_press(stream,context, BUTTON_B, 235ms, 0ms);
        pbf_wait(context, 323ms);
        segment03_route_press(stream,context, BUTTON_B, 210ms, 0ms);
        pbf_wait(context, 189ms);
        segment03_route_press(stream,context, BUTTON_B, 210ms, 0ms);
        pbf_wait(context, 365ms);
        segment03_route_press(stream,context, BUTTON_B, 236ms, 0ms);
        pbf_wait(context, 189ms);
        segment03_route_press(stream,context, BUTTON_B, 170ms, 0ms);
        pbf_wait(context, 3365ms);
        context.wait_for_all_requests();
    }
    void heartholme_to_lost_tower(VideoStream& stream, ProControllerContext& context) {
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 903ms, 0ms);
        pbf_wait(context, 1455ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 1830ms, 0ms);
        pbf_wait(context, 144ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 1880ms, 0ms);
        pbf_wait(context, 81ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 609ms, 0ms);
        pbf_wait(context, 73ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 2014ms, 0ms);
        pbf_wait(context, 186ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 1744ms, 0ms);
        pbf_wait(context, 14ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 411ms, 0ms);
        pbf_wait(context, 164ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1944ms, 0ms);
        pbf_wait(context, 226ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 1142ms, 0ms);
        pbf_wait(context, 539ms);
        segment03_route_battle(stream,context,"Barry in Hearthome");
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 877ms, 0ms);
        pbf_wait(context, 55ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 2561ms, 0ms);
        pbf_wait(context, 25ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 344ms, 0ms);
        pbf_wait(context, 203ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 518ms, 0ms);
        pbf_wait(context, 1655ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 1906ms, 0ms);
        pbf_wait(context, 1761ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 878ms, 0ms);
        pbf_wait(context, 1967ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 1366ms, 0ms);
        pbf_wait(context, 167ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 450ms, 0ms);
        pbf_move_left_joystick(context, {-1.000000, 1.000000}, 250ms, 0ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 244ms, 0ms);
        pbf_wait(context, 77ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 143ms, 0ms);
        pbf_wait(context, 71ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 232ms, 0ms);
        pbf_move_left_joystick(context, {-1.000000, 1.000000}, 261ms, 0ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 206ms, 0ms);
        pbf_move_left_joystick(context, {-1.000000, 1.000000}, 142ms, 0ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 278ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, 1.000000}, 225ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 3015ms, 0ms);
        pbf_wait(context, 95ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 1122ms, 0ms);
        pbf_wait(context, 9ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 1523ms, 0ms);
        pbf_wait(context, 14ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 626ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, 1.000000}, 28ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 300ms, 0ms);
        pbf_wait(context, 110ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 1079ms, 0ms);
        pbf_wait(context, 189ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 322ms, 0ms);
        pbf_wait(context, 344ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 90ms, 0ms);
        pbf_wait(context, 777ms);
        context.wait_for_all_requests();
    }
    static void lost_tower(VideoStream& stream, ProControllerContext& context) {
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 745ms, 0ms);
        pbf_wait(context, 100ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 994ms, 0ms);
        pbf_wait(context, 94ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 1179ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, 1.000000}, 193ms, 0ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 884ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 1382ms, 0ms);
        pbf_wait(context, 524ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 1750ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, 1.000000}, 145ms, 0ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 116ms, 0ms);
        pbf_wait(context, 373ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 321ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, 1.000000}, 212ms, 0ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 202ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, 1.000000}, 132ms, 0ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 212ms, 0ms);
        pbf_wait(context, 1507ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 403ms, 0ms);
        pbf_move_left_joystick(context, {-1.000000, 1.000000}, 210ms, 0ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 244ms, 0ms);
        pbf_wait(context, 56ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 567ms, 0ms);
        pbf_wait(context, 855ms);
        segment03_route_press(stream,context, BUTTON_X, 212ms, 0ms);
        pbf_wait(context, 683ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 184ms, 0ms);
        pbf_wait(context, 415ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 129ms, 0ms);
        pbf_wait(context, 190ms);
        segment03_route_press(stream,context, BUTTON_A, 196ms, 0ms);
        pbf_wait(context, 1326ms);
        segment03_route_press(stream,context, BUTTON_A, 144ms, 0ms);
        pbf_wait(context, 591ms);
        segment03_route_press(stream,context, BUTTON_A, 142ms, 0ms);
        pbf_wait(context, 1017ms);
        segment03_route_press(stream,context, BUTTON_A, 160ms, 0ms);
        pbf_wait(context, 411ms);
        segment03_route_press(stream,context, BUTTON_B, 190ms, 0ms);
        pbf_wait(context, 594ms);
        segment03_route_press(stream,context, BUTTON_B, 172ms, 0ms);
        pbf_wait(context, 190ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 317ms, 0ms);
        pbf_move_left_joystick(context, {-1.000000, 1.000000}, 266ms, 0ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 76ms, 0ms);
        pbf_wait(context, 424ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 265ms, 0ms);
        pbf_move_left_joystick(context, {-1.000000, 1.000000}, 61ms, 0ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 280ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, 1.000000}, 144ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 814ms, 0ms);
        pbf_wait(context, 1386ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 944ms, 0ms);
        pbf_wait(context, 390ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 347ms, 0ms);
        pbf_wait(context, 236ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 450ms, 0ms);
        pbf_wait(context, 400ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 581ms, 0ms);
        pbf_wait(context, 8ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 964ms, 0ms);
        pbf_wait(context, 129ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 215ms, 0ms);
        pbf_wait(context, 390ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1092ms, 0ms);
        pbf_wait(context, 129ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 1353ms, 0ms);
        pbf_wait(context, 276ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 193ms, 0ms);
        pbf_wait(context, 535ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 711ms, 0ms);
        pbf_wait(context, 123ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 746ms, 0ms);
        pbf_wait(context, 1670ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 1114ms, 0ms);
        pbf_wait(context, 474ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 175ms, 0ms);
        pbf_wait(context, 377ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 828ms, 0ms);
        pbf_wait(context, 366ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 165ms, 0ms);
        pbf_wait(context, 274ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 1012ms, 0ms);
        pbf_wait(context, 1860ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 900ms, 0ms);
        pbf_wait(context, 1078ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 1089ms, 0ms);
        pbf_wait(context, 711ms);
        segment03_route_battle(stream,context,"Lost Tower trainers");
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 245ms, 0ms);
        pbf_move_left_joystick(context, {-1.000000, 1.000000}, 167ms, 0ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 228ms, 0ms);
        pbf_wait(context, 455ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 817ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, 1.000000}, 47ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 853ms, 0ms);
        pbf_wait(context, 13884ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 1010ms, 0ms);
        pbf_wait(context, 2229ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 261ms, 0ms);
        pbf_wait(context, 887ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 60ms, 0ms);
        pbf_wait(context, 420ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1567ms, 0ms);
        pbf_wait(context, 173ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 578ms, 0ms);
        pbf_wait(context, 53ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 308ms, 0ms);
        pbf_wait(context, 54ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 451ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, 1.000000}, 126ms, 0ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 33ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, 1.000000}, 85ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 883ms, 0ms);
        pbf_wait(context, 1309ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 419ms, 0ms);
        pbf_move_left_joystick(context, {-1.000000, -1.000000}, 44ms, 0ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 300ms, 0ms);
        pbf_wait(context, 371ms);
        segment03_route_press(stream,context, BUTTON_A, 181ms, 0ms);
        pbf_wait(context, 136ms);
        segment03_route_press(stream,context, BUTTON_A, 177ms, 0ms);
        pbf_wait(context, 137ms);
        segment03_route_press(stream,context, BUTTON_A, 146ms, 0ms);
        pbf_wait(context, 286ms);
        segment03_route_press(stream,context, BUTTON_A, 150ms, 0ms);
        pbf_wait(context, 175ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 310ms, 0ms);
        pbf_move_left_joystick(context, {-1.000000, -1.000000}, 124ms, 0ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 377ms, 0ms);
        pbf_wait(context, 320ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 327ms, 0ms);
        pbf_wait(context, 118ms);
        segment03_route_press(stream,context, BUTTON_A, 213ms, 0ms);
        pbf_wait(context, 173ms);
        segment03_route_press(stream,context, BUTTON_A, 205ms, 0ms);
        pbf_wait(context, 106ms);
        segment03_route_press(stream,context, BUTTON_A, 170ms, 0ms);
        pbf_wait(context, 115ms);
        segment03_route_press(stream,context, BUTTON_A, 177ms, 0ms);
        pbf_wait(context, 145ms);
        segment03_route_press(stream,context, BUTTON_A, 194ms, 0ms);
        pbf_wait(context, 334ms);
        segment03_route_press(stream,context, BUTTON_A, 217ms, 0ms);
        pbf_wait(context, 174ms);
        segment03_route_press(stream,context, BUTTON_A, 190ms, 0ms);
        pbf_wait(context, 234ms);
        segment03_route_press(stream,context, BUTTON_A, 233ms, 0ms);
        pbf_wait(context, 119ms);
        segment03_route_press(stream,context, BUTTON_A, 201ms, 0ms);
        pbf_wait(context, 133ms);
        segment03_route_press(stream,context, BUTTON_A, 195ms, 0ms);
        pbf_wait(context, 123ms);
        segment03_route_press(stream,context, BUTTON_A, 214ms, 0ms);
        pbf_wait(context, 109ms);
        segment03_route_press(stream,context, BUTTON_B, 217ms, 0ms);
        pbf_wait(context, 157ms);
        segment03_route_press(stream,context, BUTTON_B, 162ms, 0ms);
        pbf_wait(context, 523ms);
        segment03_route_press(stream,context, BUTTON_B, 192ms, 0ms);
        pbf_wait(context, 79ms);
        segment03_route_press(stream,context, BUTTON_B, 132ms, 0ms);
        pbf_wait(context, 115ms);
        segment03_route_press(stream,context, BUTTON_B, 126ms, 0ms);
        pbf_wait(context, 211ms);
        segment03_route_press(stream,context, BUTTON_B, 156ms, 0ms);
        pbf_wait(context, 528ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 156ms, 0ms);
        pbf_wait(context, 559ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 171ms, 0ms);
        pbf_wait(context, 65ms);
        segment03_route_press(stream,context, BUTTON_A, 204ms, 0ms);
        pbf_wait(context, 172ms);
        segment03_route_press(stream,context, BUTTON_A, 177ms, 0ms);
        pbf_wait(context, 113ms);
        segment03_route_press(stream,context, BUTTON_A, 173ms, 0ms);
        pbf_wait(context, 92ms);
        segment03_route_press(stream,context, BUTTON_A, 158ms, 0ms);
        pbf_wait(context, 102ms);
        segment03_route_press(stream,context, BUTTON_A, 168ms, 0ms);
        pbf_wait(context, 104ms);
        segment03_route_press(stream,context, BUTTON_A, 179ms, 0ms);
        pbf_wait(context, 106ms);
        segment03_route_press(stream,context, BUTTON_A, 194ms, 0ms);
        pbf_wait(context, 402ms);
        segment03_route_press(stream,context, BUTTON_A, 182ms, 0ms);
        pbf_wait(context, 390ms);
        segment03_route_press(stream,context, BUTTON_A, 193ms, 0ms);
        pbf_wait(context, 571ms);
        segment03_route_press(stream,context, BUTTON_A, 213ms, 0ms);
        pbf_wait(context, 2265ms);
        segment03_route_press(stream,context, BUTTON_A, 218ms, 0ms);
        pbf_wait(context, 934ms);
        segment03_route_press(stream,context, BUTTON_A, 232ms, 0ms);
        pbf_wait(context, 171ms);
        segment03_route_press(stream,context, BUTTON_A, 180ms, 0ms);
        pbf_wait(context, 488ms);
        segment03_route_press(stream,context, BUTTON_A, 229ms, 0ms);
        pbf_wait(context, 154ms);
        segment03_route_press(stream,context, BUTTON_A, 193ms, 0ms);
        pbf_wait(context, 287ms);
        segment03_route_press(stream,context, BUTTON_A, 199ms, 0ms);
        pbf_wait(context, 855ms);
        segment03_route_press(stream,context, BUTTON_A, 249ms, 0ms);
        pbf_wait(context, 594ms);
        segment03_route_press(stream,context, BUTTON_A, 218ms, 0ms);
        pbf_wait(context, 172ms);
        segment03_route_press(stream,context, BUTTON_A, 172ms, 0ms);
        pbf_wait(context, 363ms);
        segment03_route_press(stream,context, BUTTON_A, 207ms, 0ms);
        pbf_wait(context, 1485ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 400ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, 1.000000}, 49ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 652ms, 0ms);
        pbf_wait(context, 69ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 89ms, 0ms);
        pbf_move_left_joystick(context, {-1.000000, 1.000000}, 126ms, 0ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 149ms, 0ms);
        pbf_move_left_joystick(context, {-1.000000, 1.000000}, 101ms, 0ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 104ms, 0ms);
        pbf_move_left_joystick(context, {-1.000000, 1.000000}, 24ms, 0ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 152ms, 0ms);
        context.wait_for_all_requests();
    }
    void lost_tower_to_veilstone_gym(VideoStream& stream, ProControllerContext& context) {
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 501ms, 0ms);
        pbf_wait(context, 303ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 527ms, 0ms);
        pbf_wait(context, 243ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 221ms, 0ms);
        pbf_wait(context, 546ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 1208ms, 0ms);
        pbf_wait(context, 177ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 171ms, 0ms);
        pbf_wait(context, 177ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 2245ms, 0ms);
        pbf_wait(context, 487ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 523ms, 0ms);
        pbf_wait(context, 196ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 854ms, 0ms);
        pbf_wait(context, 378ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 205ms, 0ms);
        pbf_wait(context, 273ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 792ms, 0ms);
        pbf_wait(context, 2437ms);
        if (!heal_pokemon(stream,context,"Solaceon")) OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,"Solaceon Center exit not confirmed.",stream);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 181ms, 0ms);
        pbf_move_left_joystick(context, {-1.000000, -1.000000}, 192ms, 0ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 448ms, 0ms);
        pbf_wait(context, 13ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 811ms, 0ms);
        pbf_wait(context, 49ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 549ms, 0ms);
        pbf_wait(context, 391ms);
        segment03_route_press(stream,context, BUTTON_B, 175ms, 0ms);
        pbf_wait(context, 222ms);
        segment03_route_press(stream,context, BUTTON_B, 141ms, 0ms);
        pbf_wait(context, 781ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 180ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, -1.000000}, 38ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 1070ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, -1.000000}, 17ms, 0ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 186ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, -1.000000}, 35ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 191ms, 0ms);
        pbf_wait(context, 31ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 323ms, 0ms);
        pbf_wait(context, 3187ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 795ms, 0ms);
        pbf_wait(context, 72ms);
        segment03_route_press(stream,context, BUTTON_A, 178ms, 0ms);
        pbf_wait(context, 540ms);
        segment03_route_press(stream,context, BUTTON_A, 159ms, 0ms);
        pbf_wait(context, 655ms);
        segment03_route_press(stream,context, BUTTON_A, 168ms, 0ms);
        pbf_wait(context, 1683ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 163ms, 0ms);
        pbf_wait(context, 2372ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 160ms, 0ms);
        pbf_wait(context, 454ms);
        segment03_route_press(stream,context, BUTTON_A, 165ms, 0ms);
        pbf_wait(context, 1591ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 147ms, 0ms);
        pbf_wait(context, 853ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 91ms, 0ms);
        pbf_wait(context, 389ms);
        segment03_route_press(stream,context, BUTTON_A, 162ms, 0ms);
        pbf_wait(context, 438ms);
        segment03_route_press(stream,context, BUTTON_A, 160ms, 0ms);
        pbf_wait(context, 106ms);
        segment03_route_press(stream,context, BUTTON_A, 144ms, 0ms);
        pbf_wait(context, 287ms);
        segment03_route_press(stream,context, BUTTON_A, 169ms, 0ms);
        pbf_wait(context, 534ms);
        segment03_route_press(stream,context, BUTTON_A, 160ms, 0ms);
        pbf_wait(context, 655ms);
        segment03_route_press(stream,context, BUTTON_B, 211ms, 0ms);
        pbf_wait(context, 473ms);
        segment03_route_press(stream,context, BUTTON_B, 168ms, 0ms);
        pbf_wait(context, 401ms);
        segment03_route_press(stream,context, BUTTON_B, 175ms, 0ms);
        pbf_wait(context, 439ms);
        segment03_route_press(stream,context, BUTTON_B, 78ms, 0ms);
        pbf_wait(context, 723ms);
        segment03_route_press(stream,context, BUTTON_B, 159ms, 0ms);
        pbf_wait(context, 213ms);
        segment03_route_press(stream,context, BUTTON_B, 122ms, 0ms);
        pbf_wait(context, 544ms);
        segment03_route_press(stream,context, BUTTON_B, 144ms, 0ms);
        pbf_wait(context, 567ms);
        segment03_route_press(stream,context, BUTTON_B, 167ms, 0ms);
        pbf_wait(context, 216ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 1117ms, 0ms);
        pbf_wait(context, 1924ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1610ms, 0ms);
        pbf_wait(context, 189ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 5877ms, 0ms);
        pbf_wait(context, 2351ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 383ms, 0ms);
        pbf_wait(context, 321ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 745ms, 0ms);
        pbf_wait(context, 3127ms);
        segment03_route_press(stream,context, BUTTON_A, 213ms, 0ms);
        pbf_wait(context, 144ms);
        segment03_route_press(stream,context, BUTTON_A, 184ms, 0ms);
        pbf_wait(context, 550ms);
        segment03_route_press(stream,context, BUTTON_A, 211ms, 0ms);
        pbf_wait(context, 122ms);
        segment03_route_press(stream,context, BUTTON_A, 188ms, 0ms);
        pbf_wait(context, 130ms);
        segment03_route_press(stream,context, BUTTON_A, 160ms, 0ms);
        pbf_wait(context, 272ms);
        segment03_route_press(stream,context, BUTTON_A, 168ms, 0ms);
        pbf_wait(context, 5334ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 159ms, 0ms);
        pbf_wait(context, 204ms);
        segment03_route_press(stream,context, BUTTON_A, 170ms, 0ms);
        pbf_wait(context, 698ms);
        segment03_route_press(stream,context, BUTTON_A, 195ms, 0ms);
        pbf_wait(context, 230ms);
        segment03_route_press(stream,context, BUTTON_A, 169ms, 0ms);
        pbf_wait(context, 773ms);
        segment03_route_press(stream,context, BUTTON_X, 212ms, 0ms);
        pbf_wait(context, 1555ms);
        segment03_route_press(stream,context, BUTTON_X, 233ms, 0ms);
        pbf_wait(context, 1255ms);
        segment03_route_press(stream,context, BUTTON_A, 173ms, 0ms);
        pbf_wait(context, 1640ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 177ms, 0ms);
        pbf_wait(context, 278ms);
        segment03_route_press(stream,context, BUTTON_A, 189ms, 0ms);
        pbf_wait(context, 506ms);
        segment03_route_press(stream,context, BUTTON_A, 182ms, 0ms);
        pbf_wait(context, 700ms);
        segment03_route_press(stream,context, BUTTON_A, 167ms, 0ms);
        pbf_wait(context, 869ms);
        segment03_route_press(stream,context, BUTTON_B, 176ms, 0ms);
        pbf_wait(context, 356ms);
        segment03_route_press(stream,context, BUTTON_B, 211ms, 0ms);
        pbf_wait(context, 257ms);
        segment03_route_press(stream,context, BUTTON_B, 158ms, 0ms);
        pbf_wait(context, 496ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1677ms, 0ms);
        pbf_move_left_joystick(context, {-1.000000, 1.000000}, 14ms, 0ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 2254ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, 1.000000}, 11ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 2190ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, 1.000000}, 1ms, 0ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 3839ms, 0ms);
        pbf_wait(context, 98ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 1963ms, 0ms);
        pbf_wait(context, 13ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 1099ms, 0ms);
        pbf_wait(context, 235ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 2766ms, 0ms);
        pbf_wait(context, 40ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 1652ms, 0ms);
        pbf_wait(context, 30ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 839ms, 0ms);
        pbf_wait(context, 98ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 725ms, 0ms);
        pbf_wait(context, 77ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 824ms, 0ms);
        pbf_wait(context, 356ms);
        segment03_route_battle(stream,context,"Route 215 first trainer");
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 2350ms, 0ms);
        pbf_wait(context, 56ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 2095ms, 0ms);
        pbf_wait(context, 69ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 747ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, 1.000000}, 10ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 1434ms, 0ms);
        pbf_wait(context, 41ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 1993ms, 0ms);
        pbf_wait(context, 55ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 1542ms, 0ms);
        pbf_wait(context, 192ms);
        segment03_route_press(stream,context, BUTTON_A, 174ms, 0ms);
        pbf_wait(context, 841ms);
        segment03_route_press(stream,context, BUTTON_A, 184ms, 0ms);
        pbf_wait(context, 549ms);
        segment03_route_press(stream,context, BUTTON_A, 177ms, 0ms);
        pbf_wait(context, 261ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 541ms, 0ms);
        pbf_wait(context, 6ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 562ms, 0ms);
        pbf_wait(context, 42ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 520ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, -1.000000}, 77ms, 0ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 166ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, -1.000000}, 37ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 647ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, 1.000000}, 15ms, 0ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 1062ms, 0ms);
        pbf_wait(context, 197ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 1426ms, 0ms);
        pbf_wait(context, 5ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 676ms, 0ms);
        pbf_wait(context, 9ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 2523ms, 0ms);
        pbf_wait(context, 471ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 378ms, 0ms);
        pbf_wait(context, 488ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 943ms, 0ms);
        pbf_wait(context, 272ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 127ms, 0ms);
        pbf_wait(context, 380ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 920ms, 0ms);
        pbf_wait(context, 92ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 1115ms, 0ms);
        pbf_wait(context, 16ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 144ms, 0ms);
        pbf_wait(context, 480ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 613ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, -1.000000}, 10ms, 0ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 320ms, 0ms);
        pbf_wait(context, 56ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 778ms, 0ms);
        pbf_wait(context, 476ms);
        segment03_route_battle(stream,context,"Route 215 Ace Trainers");
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 4051ms, 0ms);
        pbf_wait(context, 1256ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 1047ms, 0ms);
        pbf_wait(context, 503ms);
        segment03_route_press(stream,context, BUTTON_B, 189ms, 0ms);
        pbf_wait(context, 222ms);
        segment03_route_press(stream,context, BUTTON_B, 183ms, 0ms);
        pbf_wait(context, 637ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 1824ms, 0ms);
        pbf_wait(context, 1256ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 2226ms, 0ms);
        pbf_wait(context, 891ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 1678ms, 0ms);
        pbf_wait(context, 70ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 1496ms, 0ms);
        pbf_wait(context, 54ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 1179ms, 0ms);
        pbf_wait(context, 21ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 2448ms, 0ms);
        pbf_wait(context, 54ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 1574ms, 0ms);
        pbf_wait(context, 58ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 3675ms, 0ms);
        pbf_wait(context, 93ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 666ms, 0ms);
        pbf_wait(context, 69ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1757ms, 0ms);
        pbf_wait(context, 57ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 3227ms, 0ms);
        pbf_wait(context, 4788ms);
        segment03_route_press(stream,context, BUTTON_A, 182ms, 0ms);
        pbf_wait(context, 388ms);
        segment03_route_press(stream,context, BUTTON_A, 211ms, 0ms);
        pbf_wait(context, 176ms);
        segment03_route_press(stream,context, BUTTON_A, 172ms, 0ms);
        pbf_wait(context, 509ms);
        segment03_route_press(stream,context, BUTTON_A, 198ms, 0ms);
        pbf_wait(context, 255ms);
        segment03_route_press(stream,context, BUTTON_A, 170ms, 0ms);
        pbf_wait(context, 408ms);
        segment03_route_press(stream,context, BUTTON_A, 212ms, 0ms);
        pbf_wait(context, 189ms);
        segment03_route_press(stream,context, BUTTON_A, 166ms, 0ms);
        pbf_wait(context, 458ms);
        segment03_route_press(stream,context, BUTTON_A, 168ms, 0ms);
        pbf_wait(context, 3630ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 744ms, 0ms);
        // The recording ends outside the gym door, facing north.
        segment03_doorway(stream,context,true,1200ms,"Veilstone Gym entrance");
        context.wait_for_all_requests();
    }

static void gym_Puzzle_03(VideoStream& stream, ProControllerContext& context){

    //Upon entering the Gym.

    struct Move {
        DpadPosition dir;
        int count;
        std::chrono::milliseconds delay;
    };

    const Move sequence1[] = {
        {DPAD_RIGHT, 8, 0ms},
        {DPAD_UP, 6, 0ms},
        {DPAD_LEFT, 2, 0ms},
        {DPAD_UP, 2, 0ms},
        {DPAD_RIGHT, 1, 0ms},
        {DPAD_RIGHT, 1, 1000ms},
        {DPAD_UP, 2, 0ms},
    };

    const Move sequence2[] = {
        {DPAD_UP, 5, 0ms},
        {DPAD_RIGHT, 1, 0ms},
        {DPAD_RIGHT, 1, 1000ms},
        {DPAD_DOWN, 8, 0ms},
        {DPAD_RIGHT, 2, 0ms},
        {DPAD_DOWN, 3, 0ms},
        {DPAD_LEFT, 5, 0ms},
        {DPAD_UP, 7, 0ms},
        {DPAD_RIGHT, 1, 0ms},
        {DPAD_RIGHT, 1, 1000ms},
        {DPAD_RIGHT, 1, 0ms},
        {DPAD_UP, 3, 0ms},
        {DPAD_LEFT, 2, 0ms},
        {DPAD_UP, 2, 0ms},
        {DPAD_RIGHT, 1, 0ms},
        {DPAD_RIGHT, 1, 2000ms},
        {DPAD_RIGHT, 1, 0ms},
        {DPAD_DOWN, 10, 0ms},
        {DPAD_RIGHT, 6, 0ms},
        {DPAD_UP, 7, 0ms},
        {DPAD_LEFT, 1, 0ms},
        {DPAD_LEFT, 1, 1000ms},
        {DPAD_UP, 6, 0ms},
        {DPAD_RIGHT, 2, 0ms},
        {DPAD_UP, 2, 0ms},
        {DPAD_LEFT, 1, 0ms},
        {DPAD_LEFT, 1, 1000ms},
        {DPAD_LEFT, 1, 0ms},
        {DPAD_DOWN, 13, 0ms},
        {DPAD_LEFT, 3, 0ms},
        {DPAD_UP, 4, 0ms},
        {DPAD_LEFT, 2, 0ms},
        {DPAD_UP, 8, 0ms},
        {DPAD_RIGHT, 2, 0ms},
        {DPAD_UP, 4, 0ms},
        {DPAD_LEFT, 4, 0ms},
        {DPAD_DOWN, 2, 0ms},
        {DPAD_LEFT, 1, 0ms},
        {DPAD_LEFT, 1, 1000ms},
        {DPAD_UP, 2, 0ms},
        {DPAD_RIGHT, 4, 0ms},
        {DPAD_DOWN, 6, 0ms},
        {DPAD_LEFT, 2, 0ms},
        {DPAD_DOWN, 7, 0ms},
        {DPAD_RIGHT, 2, 0ms},
        {DPAD_DOWN, 3, 0ms},
        {DPAD_LEFT, 5, 0ms},
        {DPAD_UP, 7, 0ms},
        {DPAD_LEFT, 4, 0ms},
        {DPAD_LEFT, 1, 2000ms},
        {DPAD_LEFT, 2, 0ms},
        {DPAD_UP, 5, 0ms},
        {DPAD_LEFT, 1, 0ms},
        {DPAD_LEFT, 1, 2000ms},
        {DPAD_DOWN, 5, 0ms},
        {DPAD_RIGHT, 4, 0ms},
        {DPAD_DOWN, 3, 0ms},
    };

    const Move sequence3[] = {
        {DPAD_RIGHT, 2, 0ms},
        {DPAD_DOWN, 3, 0ms},
        {DPAD_LEFT, 1, 0ms},
        {DPAD_LEFT, 1, 3000ms},
        {DPAD_DOWN, 3, 0ms},
        {DPAD_LEFT, 4, 0ms},
        {DPAD_UP, 5, 0ms},
        {DPAD_LEFT, 1, 0ms},
        {DPAD_LEFT, 1, 2000ms},
        {DPAD_DOWN, 5, 0ms},
        {DPAD_LEFT, 8, 0ms},
        {DPAD_UP, 3, 0ms},
        {DPAD_RIGHT, 1, 0ms},
        {DPAD_RIGHT, 1, 3000ms},
        {DPAD_UP, 11, 0ms},
        {DPAD_RIGHT, 1, 0ms},
        {DPAD_RIGHT, 1, 1000ms},
        {DPAD_DOWN, 13, 0ms},
        {DPAD_RIGHT, 4, 0ms},
        {DPAD_UP, 3, 0ms},
    };

    const Move sequence4[] = {
        {DPAD_RIGHT, 2, 0ms},
        {DPAD_UP, 9, 0ms},
        {DPAD_LEFT, 2, 0ms},
        {DPAD_UP, 4, 0ms},
        {DPAD_RIGHT, 4, 0ms},
        {DPAD_RIGHT, 1, 1000ms},
        {DPAD_RIGHT, 1, 0ms},
        {DPAD_DOWN, 8, 0ms},
        {DPAD_RIGHT, 4, 0ms},
        {DPAD_UP, 3, 0ms},
    };

    const Move sequence5[] = {
        {DPAD_RIGHT, 2, 0ms},
        {DPAD_UP, 3, 0ms},
        {DPAD_LEFT, 2, 0ms},
        {DPAD_UP, 8, 0ms},
    };

    const Move sequence6[] = {
        {DPAD_DOWN, 8, 0ms},
        {DPAD_LEFT, 2, 0ms},
        {DPAD_DOWN, 3, 0ms},
        {DPAD_RIGHT, 2, 0ms},
        {DPAD_DOWN, 5, 0ms},
        {DPAD_RIGHT, 2, 0ms},
        {DPAD_DOWN, 3, 0ms},
        {DPAD_LEFT, 1, 0ms},
        {DPAD_LEFT, 1, 3000ms},
        {DPAD_DOWN, 6, 0ms},
    };

    for (const auto& move : sequence1){
        for (int i = 0; i < move.count; i++){
            pbf_press_dpad(context, move.dir, 80ms, 300ms);
            if (move.delay.count() > 0){
                pbf_wait(context, move.delay);
            }
        }
    }

    //Button presses to initiate Trainer battle 01 (Black Belt Rafael).

    pbf_press_button(context,BUTTON_A,150ms,300ms);
    segment03_route_battle(stream,context,"Black Belt Rafael (Flamethrower)");

    pbf_mash_button(context, BUTTON_B, 2500ms); //Battle exit dialogue

    for (const auto& move : sequence2){
        for (int i = 0; i < move.count; i++){
            pbf_press_dpad(context, move.dir, 80ms, 300ms);
            if (move.delay.count() > 0){
                pbf_wait(context, move.delay);
            }
        }
    }

    //Button presses to initiate Trainer battle 02 (Black Belt Jeffrey).

    pbf_press_button(context,BUTTON_A,150ms,300ms);
    segment03_route_battle(stream,context,"Black Belt Jeffrey (Flamethrower)");

    pbf_mash_button(context, BUTTON_B, 2500ms); //Battle exit dialogue

    for (const auto& move : sequence3){
        for (int i = 0; i < move.count; i++){
            pbf_press_dpad(context, move.dir, 80ms, 300ms);
            if (move.delay.count() > 0){
                pbf_wait(context, move.delay);
            }
        }
    }

    //Button presses to initiate Trainer battle 03 (Black Belt Darren).

    pbf_press_button(context,BUTTON_A,150ms,300ms);
    segment03_route_battle(stream,context,"Black Belt Darren (Flamethrower)");

    pbf_mash_button(context, BUTTON_B, 2500ms); //Battle exit dialogue

    for (const auto& move : sequence4){
        for (int i = 0; i < move.count; i++){
            pbf_press_dpad(context, move.dir, 80ms, 300ms);
            if (move.delay.count() > 0){
                pbf_wait(context, move.delay);
            }
        }
    }

    //Button presses to initiate Trainer battle 04 (Black Belt Colby).

    pbf_press_button(context,BUTTON_A,150ms,300ms);
    segment03_route_battle(stream,context,"Black Belt Colby (Flamethrower)");

    pbf_mash_button(context, BUTTON_B, 2500ms); //Battle exit dialogue

    for (const auto& move : sequence5){
        for (int i = 0; i < move.count; i++){
            pbf_press_dpad(context, move.dir, 80ms, 300ms);
            if (move.delay.count() > 0){
                pbf_wait(context, move.delay);
            }
        }
    }

    //Button presses to initiate the Gym battle.

    pbf_press_button(context,BUTTON_A,150ms,300ms);
    segment03_route_battle(stream,context,"Maylene (Flamethrower)",true);

    pbf_mash_button(context, BUTTON_B, 2500ms); //Battle exit dialogue

    for (const auto& move : sequence6){
        for (int i = 0; i < move.count; i++){
            pbf_press_dpad(context, move.dir, 80ms, 300ms);
            if (move.delay.count() > 0){
                pbf_wait(context, move.delay);
            }
        }
    }

    //Exits Buiilding
    pbf_wait(context, 1000ms);
}

std::string AutoStory_Segment_03::name()       const{ return "03: Second Badge to Third Badge"; }
std::string AutoStory_Segment_03::start_text() const{ return "Start: Second Gym Badge acquired (Eterna City)."; }
std::string AutoStory_Segment_03::end_text()   const{ return "End: Third Gym Badge acquired (Veilstone City)."; }

void AutoStory_Segment_03::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_0301().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0302().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0303().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0304().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0305().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0306().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0307().run_checkpoint(env, context, options, stats);
    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}


std::string AutoStory_Checkpoint_0301::name()       const{ return "03.01 - Eterna Gym to Jupiter"; }
std::string AutoStory_Checkpoint_0301::start_text() const{ return "Outside Eterna Gym after defeating Gardenia and acquiring the Forest Badge."; }
std::string AutoStory_Checkpoint_0301::end_text()   const{ return "Outside Eterna Galactic HQ after defeating Jupiter and completing the rescue dialogue, facing south against the uncut front bush."; }

void AutoStory_Checkpoint_0301::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    Checkpoint_0301(env, context, options, stats);
}


void Checkpoint_0301(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options, stats,
        [&](size_t /*attempt*/){
            // Recorded route with feedback battle handling.
            eterna_gym_to_jupiter(env.console, context);

        },

        true
    );
}
std::string AutoStory_Checkpoint_0302::name()       const { return "03.02 - Jupiter to Mount Coronet"; }
std::string AutoStory_Checkpoint_0302::start_text() const { return "Outside Eterna Galactic HQ after defeating Jupiter and completing the rescue dialogue, facing south against the uncut front bush."; }
std::string AutoStory_Checkpoint_0302::end_text()   const { return "On Route 207 after the Hiker battle, before the west entrance of Mount Coronet."; }

void AutoStory_Checkpoint_0302::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const {
    Checkpoint_0302(env, context, options, stats);
}


void Checkpoint_0302(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) {
    checkpoint_reattempt_loop(env, context, options, stats,
        [&](size_t /*attempt*/) {
            // Recorded route with feedback battle handling.
            jupiter_to_coronet(env.console, context);

        },

        true
    );
}
std::string AutoStory_Checkpoint_0303::name()       const { return "03.03 - Mount Coronet"; }
std::string AutoStory_Checkpoint_0303::start_text() const { return "On Route 207 after the Hiker battle, before the west entrance of Mount Coronet."; }
std::string AutoStory_Checkpoint_0303::end_text()   const { return "Inside Mount Coronet near the east exit, after the first Cyrus meeting."; }

void AutoStory_Checkpoint_0303::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const {
    Checkpoint_0303(env, context, options, stats);
}


void Checkpoint_0303(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) {
    checkpoint_reattempt_loop(env, context, options, stats,
        [&](size_t /*attempt*/) {
            // Recorded route with feedback battle handling.
            coronet1(env.console, context);

        },

        true
    );
}
std::string AutoStory_Checkpoint_0304::name()       const { return "03.04 - Coronet to Hearthome"; }
std::string AutoStory_Checkpoint_0304::start_text() const { return "Inside Mount Coronet near the east exit, after the first Cyrus meeting."; }
std::string AutoStory_Checkpoint_0304::end_text()   const { return "Outside the Hearthome Contest Hall after the Keira/Johanna introduction."; }

void AutoStory_Checkpoint_0304::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const {
    Checkpoint_0304(env, context, options, stats);
}


void Checkpoint_0304(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) {
    checkpoint_reattempt_loop(env, context, options, stats,
        [&](size_t /*attempt*/) {
            // Recorded route with feedback battle handling.
            coronet_to_heartholme(env.console, context);

        },

        true
    );
}
std::string AutoStory_Checkpoint_0305::name()       const { return "03.05 - Hearthome and Lost Tower"; }
std::string AutoStory_Checkpoint_0305::start_text() const { return "Outside the Hearthome Contest Hall after the Keira/Johanna introduction."; }
std::string AutoStory_Checkpoint_0305::end_text()   const { return "At the Lost Tower departure position after the tower gifts and recorded return path."; }

void AutoStory_Checkpoint_0305::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const {
    Checkpoint_0305(env, context, options, stats);
}


void Checkpoint_0305(
	SingleSwitchProgramEnvironment& env,
	ProControllerContext& context,
	AutoStoryOptions options,
	AutoStoryStats& stats
) {
	checkpoint_reattempt_loop(env, context, options, stats,
		[&](size_t /*attempt*/) {
			// Recorded route with feedback battle handling.
			heartholme_to_lost_tower(env.console, context);
			lost_tower(env.console, context);

        },

        true
    );
}

std::string AutoStory_Checkpoint_0306::name()       const { return "03.06 - Lost Tower to Veilstone"; }
std::string AutoStory_Checkpoint_0306::start_text() const { return "At the Lost Tower departure position after the tower gifts and recorded return path."; }
std::string AutoStory_Checkpoint_0306::end_text()   const { return "Inside Veilstone Gym at the entrance, facing north."; }

void AutoStory_Checkpoint_0306::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const {
    Checkpoint_0306(env, context, options, stats);
}


void Checkpoint_0306(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) {
    checkpoint_reattempt_loop(env, context, options, stats,
        [&](size_t /*attempt*/) {
            // Recorded route with feedback battle handling.
            lost_tower_to_veilstone_gym(env.console, context);
        },
        true
    );
}

std::string AutoStory_Checkpoint_0307::name()       const { return "03.07 - Inside Gym 3"; }
std::string AutoStory_Checkpoint_0307::start_text() const { return "Inside Veilstone Gym at the entrance, facing north."; }
std::string AutoStory_Checkpoint_0307::end_text()   const { return "Third Gym Badge acquired (Veilstone City)."; }

void AutoStory_Checkpoint_0307::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const {
    Checkpoint_0307(env, context, options, stats);
}


void Checkpoint_0307(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) {
    checkpoint_reattempt_loop(env, context, options, stats,
        [&](size_t /*attempt*/) {
            //#. Veilstone Gym navigation and battle
            gym_Puzzle_03(env.console, context);
        },
        true
    );
}

}
}
}
