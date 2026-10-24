#include "PokemonBDSP/Inference/PokemonBDSP_TransitionWatchers.h"
/*  BDSP AutoStory - Segment 03
 *  From after acquiring the second badge until after acquiring the third badge.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include <chrono>
#include <functional>
#include "CommonFramework/Globals.h"
#include "CommonTools/VisualDetectors/ImageMatchDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_StartBattleDetector.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_EndBattleDetector.h"
#include "../Detect/PokemonBDSP_AutoStory_OverworldDetector.h"
#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "../Utils/PokemonBDSP_AutoStory_Segment_00_tools.h"
#include "../Utils/PokemonBDSP_Autostory_Segment_01_tools.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "../PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP_AutoStory_Segment_02.h"
#include "PokemonBDSP_AutoStory_Segment_03.h"

using namespace std::chrono_literals;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
            static void segment03_route_battle(VideoStream& stream, ProControllerContext& context,
                const std::string& label, bool post_battle_dialogue_cleanup = false) {
                fight_trainer_using_moves(stream,context,2,label,post_battle_dialogue_cleanup);
            }

    static void segment03_finish_transition(VideoStream& stream, ProControllerContext& context,
        const std::string& label) {
        BlackScreenWatcher visible(COLOR_RED,{0.1,0.1,0.8,0.8},
            100,10,BlackScreenWatcher::FinderType::GONE,500ms);
        if (wait_until(stream,context,30s,{{visible}}) != 0)
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                label+": Transition did not finish.",stream);
        pbf_wait(context,500ms);
        context.wait_for_all_requests();
    }

    // Stationary destination confirmation: do not steer during an NPC reward
    // scene or declare arrival from a fade that has only just started.
    static void segment03_confirm_location(VideoStream& stream, ProControllerContext& context,
        OverworldMatchResult (*matcher)(const ImageRGB32&), const std::string& label,
        size_t observations = 20, bool advance_dialogue = false, bool require_target = false,
        OverworldMatchResult (*instruction_card)(const ImageRGB32&) = nullptr) {
        context.wait_for_all_requests();
        size_t confirmations=0;
        for (size_t observation=0;observation<observations;observation++) {
            const auto frame=stream.video().snapshot();
            // Instruction cards require B; A cannot dismiss the Bike Riding card.
            // Require both the blue Bike illustration and its B Close prompt.
            if (frame && instruction_card && instruction_card(*frame.frame).located) {
                confirmations=0;
                stream.log("["+label+"] Blue Bike and B Close prompt detected; dismissing instruction card.");
                pbf_press_button(context,BUTTON_B,80ms,250ms);
                context.wait_for_all_requests();
                continue;
            }
            const auto match=frame ? matcher(*frame.frame) : OverworldMatchResult{};
            if (require_target ? match.at_target : match.located) {
                if (++confirmations>=3) { stream.log("["+label+"] Scene confirmed.",COLOR_GREEN); return; }
                pbf_wait(context,250ms);
            } else {
                confirmations=0;
                if (advance_dialogue) pbf_press_button(context,BUTTON_A,150ms,500ms);
                else pbf_wait(context,500ms);
            }
            context.wait_for_all_requests();
        }
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            label+": Expected scene not confirmed.",stream);
    }

    // Watch the approach itself, cancelling queued movement as soon as the
    // trainer speaks or a battle starts. The existing battle engine owns combat.
    static void segment03_expected_encounter(VideoStream& stream, ProControllerContext& context,
        const std::string& label, const std::function<void(ProControllerContext&)>& approach,
        Milliseconds encounter_wait=45000ms, bool require_encounter=true) {
        context.wait_for_all_requests();
        ShortDialogWatcher dialogue;
        StartBattleDetector starting(stream.overlay());
        BattleMenuWatcher menu(BattleType::TRAINER);
        const int encountered=run_until<ProControllerContext>(stream,context,
            [&](ProControllerContext& ctx) { approach(ctx); pbf_wait(ctx,encounter_wait); },
            {{dialogue},{starting},{menu}});
        if (encountered<0) {
            if (!require_encounter) {
                stream.log("["+label+"] No encounter within "+std::to_string(encounter_wait.count())+
                    "ms after approach; trainer dodged, continuing.",COLOR_GREEN);
                return;
            }
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                label+": Expected trainer encounter did not start.",stream);
        }
        stream.log("["+label+"] Encounter detected; route movement cancelled.");
        segment03_route_battle(stream,context,label,false);
    }

    static void segment03_wait_image(VideoStream& stream, ProControllerContext& context,
        const std::string& resource, const ImageFloatBox& box, const std::string& label) {
        auto reference=std::make_shared<ImageRGB32>(RESOURCE_PATH()+resource);
        ImageMatchWatcher ready(reference,box,25.0,false,250ms);
        if (wait_until(stream,context,5s,{{ready}}) != 0)
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                label+": Visual confirmation missing.",stream);
    }

    static void segment03_register_bike(VideoStream& stream, ProControllerContext& context) {
        // Receiving Bike adds new flags after the earlier pre-shop Bag pass.
        if (!clear_new_item_flags(stream,context))
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Eterna Bike: Could not clear reward flags.",stream);
        BagItemTemplate bike;
        bike.reference_path="PokemonBDSP/Autostory/EternaBikeShop/BikeBag.png";
        bike.label="Bike";
        bike.reference_row=4;
        if (!find_bag_item(stream,context,bike,8))
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Eterna Bike: Bike selection not confirmed.",stream);
        // Locate the Key Items tab and Bike first, then use the fixed Register
        // sequence without depending on popup, wheel, or confirmation images.
        pbf_press_button(context,BUTTON_A,150ms,500ms);
        pbf_press_dpad(context,DPAD_DOWN,80ms,250ms);
        pbf_press_button(context,BUTTON_A,150ms,500ms);
        pbf_press_dpad(context,DPAD_RIGHT,80ms,250ms);
        pbf_press_button(context,BUTTON_A,150ms,500ms);
        pbf_mash_button(context,BUTTON_B,2000ms);
        context.wait_for_all_requests();
        stream.log("[Eterna Bike] Registration inputs sent; preserving the reward's mounted state.",COLOR_GREEN);
    }

    // Stop at the completed doorway transition, rather than spending the
    // remaining movement budget inside the next map.
    // Recorded Cycling Road movement may cross a gate before its budget ends.
    // Cancel at either fade color and resume only after that fade has cleared.
    static bool segment03_cycling_move(VideoStream& stream, ProControllerContext& context,
        double x, double y, Milliseconds duration, Milliseconds rest,
        const std::string& label, bool require_transition = false) {
        context.wait_for_all_requests();
        BlackScreenWatcher black;
        WhiteScreenWatcher white;
        BattleMenuWatcher battle(BattleType::TRAINER);
        const int result=run_until<ProControllerContext>(stream,context,
            [&](ProControllerContext& ctx) {
                pbf_move_left_joystick(ctx,{x,y},duration,0ms);
                pbf_wait(ctx,rest);
            },{{black},{white},{battle}});
        if (result==2) {
            segment03_route_battle(stream,context,label+" encounter");
            return false;
        }
        if (result<0) {
            if (require_transition)
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    label+": Transition did not start.",stream);
            return false;
        }
        BlackScreenWatcher black_gone(COLOR_RED,{0.1,0.1,0.8,0.8},
            100,10,BlackScreenWatcher::FinderType::GONE,500ms);
        WhiteScreenWatcher white_gone(COLOR_RED,{0.1,0.1,0.8,0.8},
            500,10,WhiteScreenWatcher::FinderType::GONE,500ms);
        const int finished=result==1
            ? wait_until(stream,context,30s,{{white_gone}})
            : wait_until(stream,context,30s,{{black_gone}});
        if (finished!=0)
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                label+": Transition did not finish.",stream);
        // A cleared fade can precede restored movement control.
        pbf_wait(context,2000ms);
        context.wait_for_all_requests();
        return true;
    }

    // Returns true if an encounter began during alignment. The battle strip
    // then owns it and counts completion without replaying entry movement.
    static bool segment03_align_cycle_start(VideoStream& stream, ProControllerContext& context) {
        context.wait_for_all_requests();
        size_t confirmed=0,missing=0;
        bool have_pose=false;
        double previous_dx=0,previous_dy=0;
        size_t observations_without_strong_alignment=0;
        for (size_t observation=0;observations_without_strong_alignment<240;observation++) {
            const auto frame=stream.video().snapshot();
            const auto match=frame ? match_cycling_road_bridge_approach(*frame.frame) : OverworldMatchResult{};
            const bool stable=match.located && have_pose &&
                std::abs(match.dx-previous_dx)<=1.0/192 &&
                std::abs(match.dy-previous_dy)<=1.0/108;
            // Reliable rail/deck geometry and a settled pose renew the budget.
            // Keep correcting while the visual alignment remains strong.
            if (stable) observations_without_strong_alignment=0;
            else ++observations_without_strong_alignment;
            std::string diagnostic="[Cycling Road bridge] observation="+std::to_string(observation+1)+
                " located="+std::to_string(match.located)+" target="+std::to_string(match.at_target)+
                " stable="+std::to_string(stable)+
                " observations_without_strong_alignment="+std::to_string(observations_without_strong_alignment)+
                " offset="+std::to_string(match.dx)+","+std::to_string(match.dy);
            for (const auto& anchor:match.landmarks)
                diagnostic+=" ["+anchor.name+" coverage="+std::to_string(anchor.score)+
                    " accepted="+std::to_string(anchor.accepted)+"]";
            stream.log(diagnostic);
            double x=0,y=0;
            Milliseconds hold=0ms,rest=250ms;
            if (!match.located) {
                confirmed=0;have_pose=false;
                if (++missing>=6)
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Cycling Road: Bridge deck and right rail not acquired.",stream);
                rest=500ms;
            } else {
                missing=0;
                if (std::abs(match.dx)>.32 || std::abs(match.dy)>.60)
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Cycling Road: Bridge alignment offset exceeds bounds.",stream);
                if (!stable) {
                    // The camera keeps easing after the bicycle stops. Require
                    // two agreeing observations before every new correction.
                    confirmed=0;
                } else if (match.at_target) {
                    if (++confirmed>=3) {
                        stream.log("[Cycling Road] Upper white platform/right rail pose confirmed; starting watched downhill entry.",COLOR_GREEN);
                        return false;
                    }
                } else {
                    confirmed=0;
                    // Align against the rail first. The demonstrated target is
                    // on the stationary upper white platform, before gravity.
                    if (std::abs(match.dx)>.012) {
                        x=match.dx>0 ? 1 : -1;
                        hold=std::abs(match.dx)>.04 ? 320ms : 240ms;
                    } else if (std::abs(match.dy)>1.0/54) {
                        y=match.dy>0 ? -1 : 1;
                        hold=std::abs(match.dy)>.06 ? 320ms : 200ms;
                    }
                }
                previous_dx=match.dx;previous_dy=match.dy;
                // A movement invalidates the old pose. Reacquire two fresh
                // settled measurements before another movement is allowed.
                have_pose=hold==0ms;
            }
            BlackScreenWatcher black;
            WhiteScreenWatcher white;
            ShortDialogWatcher dialogue;
            StartBattleDetector starting(stream.overlay());
            BattleMenuWatcher menu(BattleType::TRAINER);
            const int interrupted=run_until<ProControllerContext>(stream,context,
                [&](ProControllerContext& ctx) {
                    if (hold>0ms) pbf_move_left_joystick(ctx,{x,y},hold,rest);
                    else pbf_wait(ctx,rest);
                    ctx.wait_for_all_requests();
                },{{black},{white},{dialogue},{starting},{menu}});
            if (interrupted>=2) {
                stream.log("[Cycling Road] Encounter during bridge approach; alignment movement cancelled.");
                return true;
            }
            if (interrupted>=0)
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Cycling Road: Unexpected transition during bridge alignment.",stream);
        }
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Cycling Road: Bridge lane alignment did not converge.",stream);
    }


    static void segment03_cycle_road_battles(VideoStream& stream, ProControllerContext& context,
        bool encounter_pending) {
        size_t completed=0;
        auto watch=[&](bool initial) {
            context.wait_for_all_requests();
            ShortDialogWatcher dialogue;
            StartBattleDetector starting(stream.overlay());
            BattleMenuWatcher menu(BattleType::TRAINER);
            return run_until<ProControllerContext>(stream,context,
                [&](ProControllerContext& ctx) {
                    if (initial) pbf_move_left_joystick(ctx,{0,-1},3300ms,0ms);
                    // From the first encounter onward the controller stays
                    // neutral. The bicycle rolls naturally into the next trainer.
                    pbf_wait(ctx,completed>=3 ? 7000ms : 45000ms);
                    ctx.wait_for_all_requests();
                },{{dialogue},{starting},{menu}});
        };
        while (completed<4) {
            const int result=encounter_pending ? 0 : watch(completed==0);
            encounter_pending=false;
            if (result<0) {
                if (completed>=3) {
                    stream.log("[Cycling Road] Three battles complete; 7s without another encounter. Continuing south.");
                    return;
                }
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Cycling Road: Expected at least three battles before the south gate.",stream);
            }
            segment03_route_battle(stream,context,
                "Cycling Road encounter "+std::to_string(completed+1),false);
            ++completed;
            stream.log("[Cycling Road] Completed battles: "+std::to_string(completed)+"/4.");
        }
        stream.log("[Cycling Road] Four battles complete; skipping the quiet wait.");
    }

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
        segment03_finish_transition(stream,context,label);
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
        segment03_finish_transition(stream,context,"Galactic HQ exit");
        segment03_confirm_location(stream,context,match_galactic_hq_exterior,"Galactic HQ exterior");
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

        // Retrace to the Center column, then use its doorway to normalize the
        // position. Do not reverse the time spent loading the Mart interior.
        pbf_move_left_joystick(context,{0,-1},896ms,100ms);
        pbf_move_left_joystick(context,{-1,0},587ms,100ms);
        segment03_doorway(stream,context,true,5000ms,"Eterna Center return after Mart");
        segment03_doorway(stream,context,false,5000ms,"Eterna Center exit after Mart");
        align_to_overworld_landmarks(stream,context,match_eterna_center_departure,
            "Eterna Center departure after Mart",false,true);
    }

    static void segment03_enter_cycle_shop(VideoStream& stream, ProControllerContext& context) {
        context.wait_for_all_requests();
        auto frame=stream.video().snapshot();
        if (frame && match_eterna_cycle_shop_interior(*frame.frame).located) {
            segment03_confirm_location(stream,context,match_eterna_cycle_shop_interior,
                "Eterna bicycle shop interior");
            return;
        }
        bool entered=false;
        // Watch the entire approach AND the locator. A fade cancels all queued
        // input, even if the doorway is crossed before alignment has finished.
        BlackScreenWatcher entering;
        const int result=run_until<ProControllerContext>(stream,context,
            [&](ProControllerContext& ctx) {
                pbf_move_left_joystick(ctx,{0,-1},1514ms,0ms);
                pbf_wait(ctx,177ms);
                pbf_move_left_joystick(ctx,{1,0},587ms,0ms);
                pbf_wait(ctx,522ms);
                pbf_move_left_joystick(ctx,{1,0},144ms,0ms);
                pbf_wait(ctx,439ms);
                align_to_overworld_landmarks(stream,ctx,match_eterna_cycle_shop_entrance,
                    "Eterna bicycle shop entrance",false,true,false,
                    match_eterna_cycle_shop_interior,&entered);
                if (!entered) {
                    pbf_move_left_joystick(ctx,{0,1},4000ms,0ms);
                    pbf_wait(ctx,3000ms);
                }
            },{{entering}});
        if (result!=0 && !entered)
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Eterna bicycle shop: Entry transition not detected.",stream);
        segment03_finish_transition(stream,context,"Eterna bicycle shop entrance");
        segment03_confirm_location(stream,context,match_eterna_cycle_shop_interior,
            "Eterna bicycle shop interior");
        stream.log("[Eterna bicycle shop] Interior confirmed; outdoor alignment stopped.",COLOR_GREEN);
    }

    void jupiter_to_coronet(VideoStream& stream, ProControllerContext& context) {
        // Continue from 03.01 outside HQ, directly in front of the Cut bush.
        use_HM(stream,context,"Galactic HQ exit Cut");
        clear_post_battle_dialogue(stream,context,BUTTON_A,6000ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 1413ms, 0ms);
        pbf_wait(context, 182ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 712ms, 0ms);
        pbf_wait(context, 91ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 1126ms, 0ms);
        pbf_wait(context, 27ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 745ms, 0ms);
        pbf_wait(context, 868ms);
        align_to_overworld_landmarks(stream,context,match_eterna_center_entrance,
            "Eterna Center after Jupiter",false,true);
        segment03_doorway(stream,context,true,4000ms,"Eterna Center entrance after Jupiter");
        if (!heal_pokemon(stream,context,"Eterna after Jupiter")) OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,"Eterna Center exit not confirmed.",stream);
        segment03_finish_transition(stream,context,"Eterna Center heal exit");
        align_to_overworld_landmarks(stream,context,match_eterna_center_departure,
            "Eterna Center departure to Mart",false,true);
        eterna_mart_escape_ropes(stream,context);
        segment03_enter_cycle_shop(stream,context);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 505ms, 0ms);
        pbf_wait(context, 405ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 568ms, 0ms);
        pbf_wait(context, 403ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 642ms, 0ms);
        pbf_wait(context, 14ms);
        // The recorded approach reaches the keeper even when the exact
        // counter reference pose differs. Start talking before any reward wait.
        // Cancel Up immediately on dialogue; all subsequent inputs are A only.
        context.wait_for_all_requests();
        ShortDialogWatcher keeper_dialogue;
        if (run_until<ProControllerContext>(stream,context,
            [](ProControllerContext& ctx) {
                for (size_t press=0;press<20;press++) {
                    pbf_move_left_joystick(ctx,{0,1},80ms,100ms);
                    pbf_press_button(ctx,BUTTON_A,150ms,500ms);
                }
            },{{keeper_dialogue}}) != 0)
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Eterna bicycle shop: Keeper interaction did not produce dialogue.",stream);
        // The reward scene returns us outside already mounted. Wait for that
        // positive destination instead of replaying a fixed dialogue burst.
        segment03_confirm_location(stream,context,match_eterna_cycle_shop_reward,
            "Eterna bicycle reward complete",180,true,true,
            match_eterna_bike_riding_card);
        segment03_register_bike(stream,context);
        segment03_cycling_move(stream,context,-1,0,733ms,316ms,"Cycling Road approach");
        segment03_cycling_move(stream,context,0,-1,3280ms,3000ms,"Cycling Road north gate entrance",true);
        segment03_cycling_move(stream,context,0,-1,1789ms,3000ms,"Cycling Road north gate exit",true);
        // Fade completion can precede restored player control. Settle and
        // clear the steps before acquiring the bridge, not the cliff wall.
        pbf_wait(context,2000ms);
        context.wait_for_all_requests();
        OverworldWatcher north_gate_overworld;
        if (wait_until(stream,context,10s,{{north_gate_overworld}})!=0)
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Cycling Road: Overworld not visible after north gate exit settle.",stream);
        BlackScreenWatcher north_gate_black;
        WhiteScreenWatcher north_gate_white;
        ShortDialogWatcher north_gate_dialogue;
        StartBattleDetector north_gate_starting(stream.overlay());
        BattleMenuWatcher north_gate_menu(BattleType::TRAINER);
        const int clearance=run_until<ProControllerContext>(stream,context,
            [](ProControllerContext& ctx) {
                pbf_move_left_joystick(ctx,{0,-1},750ms,250ms);
                ctx.wait_for_all_requests();
            },{{north_gate_black},{north_gate_white},{north_gate_dialogue},{north_gate_starting},{north_gate_menu}});
        if (clearance==0 || clearance==1)
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Cycling Road: Unexpected transition while clearing north gate steps.",stream);
        const bool cycling_encounter=clearance>=2 || segment03_align_cycle_start(stream,context);
        segment03_cycle_road_battles(stream,context,cycling_encounter);
        // Any approach movement may already enter the south gate. Preserve
        // that transition instead of requesting two more doorway crossings.
        bool south_gate_entered=segment03_cycling_move(stream,context,-1,0,129ms,1499ms,"Cycling Road south gate approach");
        if (!south_gate_entered)
            south_gate_entered=segment03_cycling_move(stream,context,-1,0,361ms,3345ms,"Cycling Road south gate approach");
        if (!south_gate_entered)
            south_gate_entered=segment03_cycling_move(stream,context,0,-1,472ms,2574ms,"Cycling Road south gate approach");
        if (!south_gate_entered)
            segment03_cycling_move(stream,context,0,-1,1977ms,1544ms,"Cycling Road south gate entrance",true);
        segment03_confirm_location(stream,context,match_cycling_road_south_gate_interior,
            "Cycling Road south gate interior");
        segment03_cycling_move(stream,context,0,-1,1977ms,1544ms,"Cycling Road south gate exit",true);
        OverworldWatcher south_gate_overworld;
        if (wait_until(stream,context,10s,{{south_gate_overworld}})!=0)
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Cycling Road: Overworld not visible after south gate exit.",stream);
        // Stay southbound until the scripted Dawn/Lucas dialogue actually
        // appears. Cancel movement at the dialogue instead of spending inputs
        // against the cutscene, and never begin the eastward trainer route early.
        bool dawn_dialogue_seen=false;
        for (size_t attempt=0;attempt<4 && !dawn_dialogue_seen;attempt++) {
            ShortDialogWatcher dawn_dialogue;
            // A dialogue already open after the transition needs no movement.
            if (wait_until(stream,context,500ms,{{dawn_dialogue}})==0) {
                dawn_dialogue_seen=true;
                break;
            }
            BlackScreenWatcher dawn_black;
            WhiteScreenWatcher dawn_white;
            StartBattleDetector dawn_starting(stream.overlay());
            BattleMenuWatcher dawn_menu(BattleType::TRAINER);
            const Milliseconds south=attempt==0 ? 1546ms : 3000ms;
            stream.log("[Route 207 Dawn] South approach attempt="+std::to_string(attempt+1));
            const int result=run_until<ProControllerContext>(stream,context,
                [south](ProControllerContext& ctx) {
                    pbf_move_left_joystick(ctx,{0,-1},south,500ms);
                    ctx.wait_for_all_requests();
                },{{dawn_dialogue},{dawn_black},{dawn_white},{dawn_starting},{dawn_menu}});
            if (result==0) dawn_dialogue_seen=true;
            else if (result>=0)
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Route 207 Dawn: Unexpected transition or battle before dialogue.",stream);
        }
        if (!dawn_dialogue_seen)
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Route 207 Dawn: Dialogue not encountered after southward retries.",stream);
        stream.log("[Route 207 Dawn] Dialogue detected; south movement cancelled.",COLOR_GREEN);
        bool dawn_dialogue_cleared=false;
        for (size_t pass=0;pass<6 && !dawn_dialogue_cleared;pass++) {
            // A advances the conversation and accepts its default choice.
            // Reuse the existing dialogue helper, then guard against a brief
            // gap between text boxes being mistaken for the end of the scene.
            mash_until_dialogue_ends(stream,context,BUTTON_A,60s);
            context.wait_for_all_requests();
            ShortDialogWatcher resumed_dialogue;
            if (wait_until(stream,context,2000ms,{{resumed_dialogue}})==0) continue;
            const auto frame=stream.video().snapshot();
            OverworldDetector overworld;
            dawn_dialogue_cleared=frame && overworld.detect(*frame.frame);
        }
        if (!dawn_dialogue_cleared)
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Route 207 Dawn: Dialogue did not remain cleared.",stream);
        pbf_wait(context, 6000ms);
        stream.log("[Route 207 Dawn] Dialogue cleared; continuing to Camper Anthony.",COLOR_GREEN);
        // restof206.txt begins after Dawn. Recorded battle/menu inputs are
        // replaced by the encounter watchers and existing move-2 battle engine.
        segment03_expected_encounter(stream,context,"Route 207 Camper Anthony",
            [](ProControllerContext& ctx) {
                pbf_move_left_joystick(ctx,{0,-1},1013ms,1667ms);
                pbf_move_left_joystick(ctx,{1,0},3309ms,1011ms);
            });
        // The recorded approach triggers Lauren next without further steering.
        segment03_expected_encounter(stream,context,"Route 207 Picnicker Lauren",
            [](ProControllerContext&) {});
        // Justin can be dodged. After the recorded approach, wait three seconds
        // for an encounter; otherwise keep the remaining Coronet route.
        segment03_expected_encounter(stream,context,"Route 207 Hiker Justin",
            [](ProControllerContext& ctx) {
                pbf_move_left_joystick(ctx,{1,0},994ms,508ms);
                pbf_move_left_joystick(ctx,{0,1},534ms,457ms);
                pbf_move_left_joystick(ctx,{1,0},2076ms,149ms);
            },3000ms,false);
        // Keep the recorded route after Justin; replace its final blind doorway
        // excerpt with the same watched down/right search used at Oreburgh Gate.
        bool coronet_entered=segment03_cycling_move(stream,context,1,0,942ms,443ms,"Mount Coronet approach");
        if (!coronet_entered)
            coronet_entered=segment03_cycling_move(stream,context,0,-1,776ms,119ms,"Mount Coronet approach");
        if (!coronet_entered)
            coronet_entered=segment03_cycling_move(stream,context,1,0,627ms,147ms,"Mount Coronet approach");
        if (!coronet_entered)
            coronet_entered=segment03_cycling_move(stream,context,0,1,725ms,1008ms,"Mount Coronet approach");
        if (!coronet_entered) {
            enter_cave_with_down_right_search(stream,context,"Mount Coronet west entrance");
            segment03_finish_transition(stream,context,"Mount Coronet west entrance");
            // finish_transition includes 500ms; allow two seconds in total.
            pbf_wait(context,1500ms);
            context.wait_for_all_requests();
        }
        OverworldWatcher coronet_overworld;
        if (wait_until(stream,context,10s,{{coronet_overworld}})!=0)
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Mount Coronet: Interior not ready after west entrance.",stream);
        context.wait_for_all_requests();
    }
    void coronet1(VideoStream& stream, ProControllerContext& context) {
        activate_repel(stream, context, 5);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 2190ms, 0ms);
        pbf_wait(context, 104ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 852ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, 1.000000}, 37ms, 0ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 102ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, 1.000000}, 78ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 983ms, 0ms);
        pbf_wait(context, 1754ms);
        pbf_press_button(context, BUTTON_A, 173ms, 0ms);
        pbf_wait(context, 1072ms);
        pbf_press_button(context, BUTTON_A, 168ms, 0ms);
        pbf_wait(context, 155ms);
        pbf_press_button(context, BUTTON_A, 142ms, 0ms);
        pbf_wait(context, 136ms);
        pbf_press_button(context, BUTTON_A, 159ms, 0ms);
        pbf_wait(context, 1598ms);
        pbf_press_button(context, BUTTON_A, 244ms, 0ms);
        pbf_wait(context, 148ms);
        pbf_press_button(context, BUTTON_A, 170ms, 0ms);
        pbf_wait(context, 587ms);
        pbf_press_button(context, BUTTON_A, 211ms, 0ms);
        pbf_wait(context, 135ms);
        pbf_press_button(context, BUTTON_A, 160ms, 0ms);
        pbf_wait(context, 321ms);
        pbf_press_button(context, BUTTON_A, 211ms, 0ms);
        pbf_wait(context, 205ms);
        pbf_press_button(context, BUTTON_A, 180ms, 0ms);
        pbf_wait(context, 672ms);
        pbf_press_button(context, BUTTON_A, 182ms, 0ms);
        pbf_wait(context, 122ms);
        pbf_press_button(context, BUTTON_A, 172ms, 0ms);
        pbf_wait(context, 362ms);
        pbf_press_button(context, BUTTON_A, 172ms, 0ms);
        pbf_wait(context, 373ms);
        pbf_press_button(context, BUTTON_A, 199ms, 0ms);
        pbf_wait(context, 150ms);
        pbf_press_button(context, BUTTON_A, 183ms, 0ms);
        pbf_wait(context, 190ms);
        pbf_press_button(context, BUTTON_A, 188ms, 0ms);
        pbf_wait(context, 213ms);
        pbf_press_button(context, BUTTON_A, 143ms, 0ms);
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
        // The expiry routine starts west. Step east first to leave room for
        // its west/east pairs, then clear the old Repel before route movement.
        context.wait_for_all_requests();
        pbf_wait(context, 250ms);
        pbf_move_left_joystick(context,{1,0},1000ms,0ms);
        pbf_wait(context, 250ms);
        context.wait_for_all_requests();
        // Leave Repel off during the potentially long Hiker sight-line sweeps.
        force_repel_expiry(stream,context,RepelRenewalChoice::NO,false,true);

        // Either Hiker may interrupt the crossing. Resume toward the east
        // wall after each fight. The wiggles are one encounter-triggering
        // attempt: once started, do not restart or finish them after an
        // interruption. Keep the resumed east approach watched for a later Hiker.
        bool bridge_approach_clear=false,bridge_wiggle_started=false;
        size_t bridge_battles=0,approach_dialogues=0;
        for (size_t pass=0;pass<6 && !bridge_approach_clear;pass++) {
            ShortDialogWatcher dialogue;
            StartBattleDetector starting(stream.overlay());
            BattleMenuWatcher menu(BattleType::TRAINER);
            const int engaged=run_until<ProControllerContext>(stream,context,
                [&](ProControllerContext& ctx) {
                    pbf_move_left_joystick(ctx,{1,0},4828ms,104ms);
                    ctx.wait_for_all_requests();
                    pbf_wait(ctx,3000ms);
                    ctx.wait_for_all_requests();
                    if (!bridge_wiggle_started) {
                        // Mark before the first input: an encounter during
                        // this set fulfills its purpose and consumes the set.
                        bridge_wiggle_started=true;
                        for (size_t pair=0;pair<8;pair++) {
                            pbf_move_left_joystick(ctx,{0,1},500ms,150ms);
                            pbf_move_left_joystick(ctx,{0,-1},500ms,150ms);
                            ctx.wait_for_all_requests();
                        }
                    } else {
                        stream.log("[Route 208 bridge] Wiggle set already used; continuing watched east approach without repeating it.");
                    }
                    pbf_wait(ctx,2500ms);
                },{{dialogue},{starting},{menu}});
            if (engaged<0) {
                bridge_approach_clear=true;
                break;
            }
            if (engaged==0) {
                // A short box can be a Repel prompt. Advance it with A (the
                // default renewal choice) while watching for actual combat.
                // Never send a nonbattle choice arrow to the battle engine.
                StartBattleDetector dialogue_starting(stream.overlay());
                BattleMenuWatcher dialogue_menu(BattleType::TRAINER);
                bool dialogue_cleared=false;
                const int battle_started=run_until<ProControllerContext>(stream,context,
                    [&](ProControllerContext& ctx) {
                        mash_until_dialogue_ends(stream,ctx,BUTTON_A,20s);
                        ctx.wait_for_all_requests();
                        pbf_wait(ctx,3000ms);
                        ctx.wait_for_all_requests();
                        const auto frame=stream.video().snapshot();
                        OverworldDetector overworld;
                        ShortDialogDetector short_dialogue;
                        dialogue_cleared=frame && overworld.detect(*frame.frame) &&
                            !short_dialogue.detect(*frame.frame);
                    },{{dialogue_starting},{dialogue_menu}});
                if (battle_started<0) {
                    if (!dialogue_cleared || ++approach_dialogues>2)
                        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                            "Route 208 bridge: Approach dialogue did not clear within bounded recovery.",stream);
                    stream.log("[Route 208 bridge] Nonbattle dialogue cleared; resuming watched crossing.");
                    continue;
                }
            }
            segment03_route_battle(stream,context,"Route 208 Hiker");
            if (++bridge_battles>2)
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Route 208 bridge: Unexpected additional Hiker battle.",stream);
            pbf_wait(context,500ms);
            context.wait_for_all_requests();
        }
        if (!bridge_approach_clear)
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Route 208 bridge: Watched east approach did not clear.",stream);
        stream.log("[Route 208 bridge] Full east approach clear after " +
            std::to_string(bridge_battles) + " battle(s); continuing toward the stairs.",COLOR_GREEN);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 421ms, 0ms);
        pbf_wait(context, 178ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 1436ms, 0ms);
        pbf_wait(context, 81ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 1197ms, 0ms);
        pbf_wait(context, 298ms);
        // Fixed eastward travel can stop beyond the stair column. Center above
        // the stairs using the native left-rock pose before moving south.
        bool rock_acquisition_timed_out=false;
        align_to_overworld_landmarks(stream,context,match_route208_stair_top,
            "Route 208 stair-top rocks",false,true,false,nullptr,nullptr,0,0,0,
            &rock_acquisition_timed_out);
        if (!activate_repel(stream,context,5))
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Route 208 stair top: Repel activation failed.",stream);
        context.wait_for_all_requests();
        if (rock_acquisition_timed_out) {
            stream.log("[Route 208 stair fallback] Using east 2000ms, southwest 1500ms, then south trial 1650ms.");
            // Keep the recorded inputs explicit for review and calibration.
            // Watch this whole recovery phase; cancel its remaining inputs if
            // a transition or battle starts before the gate approach.
            BlackScreenWatcher fallback_black;
            WhiteScreenWatcher fallback_white;
            BattleMenuWatcher fallback_battle(BattleType::TRAINER);
            const int fallback_interrupted=run_until<ProControllerContext>(stream,context,
                [&](ProControllerContext& ctx) {
                    // East: clamp against the cliff edge.
                    pbf_move_left_joystick(ctx,{1,0},2000ms,0ms);
                    ctx.wait_for_all_requests();
                    // Southwest: drag toward the stair column.
                    pbf_move_left_joystick(ctx,{-1,-1},1500ms,0ms);
                    ctx.wait_for_all_requests();
                    // South: user-requested 1650ms trial; recovery may start farther down the stairs.
                    pbf_move_left_joystick(ctx,{0,-1},1650ms,0ms);
                    pbf_wait(ctx,643ms);
                    ctx.wait_for_all_requests();
                },{{fallback_black},{fallback_white},{fallback_battle}});
            if (fallback_interrupted>=0)
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Route 208 stair fallback: Unexpected transition or battle before west gate approach.",stream);
        } else {
            // Pass just above the honey tree, between the southern trainer
            // and Hannah to the north. Calibrate this 1825ms trial from video.
            pbf_move_left_joystick(context, {0.000000, -1.000000}, 1825ms, 0ms);
            pbf_wait(context, 643ms);
        }
        // Entry may occur during an earlier approach leg. Once a fade is
        // caught, skip the remaining outdoor approach rather than replaying it
        // inside the gatehouse. The existing watcher cancels at black/white,
        // waits for GONE500ms, then settles for2s before the next input.
        bool hearthome_gate_entered=segment03_cycling_move(stream,context,
            1,0,rock_acquisition_timed_out ?3422ms:3500ms,443ms,"Hearthome west gate approach");
        if (!hearthome_gate_entered)
            hearthome_gate_entered=segment03_cycling_move(stream,context,
                0,1,rock_acquisition_timed_out ?241ms:250ms,405ms,"Hearthome west gate doorway alignment");
        if (!hearthome_gate_entered)
            hearthome_gate_entered=segment03_cycling_move(stream,context,
                1,0,rock_acquisition_timed_out ?633ms:600ms,1617ms,"Hearthome west gate entrance",true);
        if (!hearthome_gate_entered)
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Hearthome west gate: Interior transition not confirmed.",stream);
        if (!segment03_cycling_move(stream,context,
            1,0,rock_acquisition_timed_out ?1743ms:1800ms,1588ms,"Hearthome west gate exit",true))
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Hearthome west gate: Exterior transition not confirmed.",stream);
        OverworldWatcher hearthome_outside;
        if (wait_until(stream,context,10s,{{hearthome_outside}})!=0)
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Hearthome west gate: Overworld not ready after exit.",stream);
        if (segment03_cycling_move(stream,context,
            1,0,2200ms,6189ms,"Hearthome Buneary approach"))
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Hearthome Buneary approach: Unexpected extra transition.",stream);
        // Begin the scene deadline after the approach requests finish, not
        // while the recorded movement and neutral waits remain queued.
        context.wait_for_all_requests();
        ShortDialogWatcher buneary_dialogue;
        if (wait_until(stream,context,15s,{{buneary_dialogue}})!=0)
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Hearthome Buneary: Dialogue did not start.",stream);
        bool buneary_cleared=false;
        for (size_t pass=0;pass<6 && !buneary_cleared;pass++) {
            mash_until_dialogue_ends(stream,context,BUTTON_A,60s);
            context.wait_for_all_requests();
            ShortDialogWatcher resumed_dialogue;
            if (wait_until(stream,context,2000ms,{{resumed_dialogue}})==0) continue;
            // Let the NPC finish walking away before any expiration movement.
            pbf_wait(context,6000ms);
            context.wait_for_all_requests();
            const auto frame=stream.video().snapshot();
            OverworldDetector overworld;
            ShortDialogDetector dialogue;
            buneary_cleared=frame && overworld.detect(*frame.frame) && !dialogue.detect(*frame.frame);
        }
        if (!buneary_cleared)
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Hearthome Buneary: Scene did not clear.",stream);
        stream.log("[Hearthome Buneary] Scene cleared; expiring Repel without renewal before Center travel.",COLOR_GREEN);
        pbf_move_left_joystick(context,{1,0},500ms,0ms);
        context.wait_for_all_requests();
        force_repel_expiry(stream,context,RepelRenewalChoice::NO,false,true);
        // Use the same whole-approach watcher as the Eterna bicycle shop.
        // A doorway reached during recorded travel or alignment cancels all
        // remaining outdoor inputs; the room must then be confirmed visually.
        bool center_entered=false;
        BlackScreenWatcher center_transition;
        const int center_entry=run_until<ProControllerContext>(stream,context,
            [&](ProControllerContext& ctx) {
                pbf_move_left_joystick(ctx, {1.000000, -0.000000}, 3500ms, 0ms);
                pbf_wait(ctx, 437ms);
                // Clear the northbound lamp column before crossing the plaza.
                pbf_move_left_joystick(ctx, {-1.000000, -0.000000}, 250ms, 0ms);
                pbf_move_left_joystick(ctx, {0.000000, 1.000000}, 3000ms, 0ms);
                pbf_wait(ctx, 1582ms);
                pbf_move_left_joystick(ctx, {-1.000000, -0.000000}, 1150ms, 0ms);
                pbf_wait(ctx, 334ms);
                // Repel expired before this travel; no blind response inputs remain.
                pbf_move_left_joystick(ctx, {-1.000000, -0.000000}, 2250ms, 0ms);
                pbf_wait(ctx, 304ms);
                pbf_move_left_joystick(ctx, {0.000000, -1.000000}, 300ms, 0ms);
                pbf_wait(ctx, 174ms);
                pbf_move_left_joystick(ctx, {-1.000000, -0.000000}, 500ms, 0ms);
                pbf_move_left_joystick(ctx, {-1.000000, 1.000000}, 100ms, 0ms);
                pbf_move_left_joystick(ctx, {0.000000, 1.000000}, 1800ms, 0ms);
                pbf_wait(ctx, 21ms);
                pbf_wait(ctx, 221ms);
                pbf_move_left_joystick(ctx, {-1.000000, -0.000000}, 500ms, 0ms);
                pbf_move_left_joystick(ctx, {0.000000, 1.000000}, 250ms, 0ms);
                pbf_wait(ctx, 59ms);
                align_to_overworld_landmarks(stream,ctx,match_hearthome_center_entrance,
                    "Hearthome Center exterior alignment",false,true,false,
                    match_hearthome_center_interior,&center_entered);
                if (!center_entered) {
                    pbf_move_left_joystick(ctx,{0,1},4000ms,0ms);
                    pbf_wait(ctx,3000ms);
                }
            },{{center_transition}});
        if (center_entry!=0 && !center_entered)
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Hearthome Center: Entry transition not detected.",stream);
        segment03_finish_transition(stream,context,"Hearthome Center entrance");
        pbf_wait(context,1500ms);
        context.wait_for_all_requests();
        segment03_confirm_location(stream,context,match_hearthome_center_interior,
            "Hearthome Center interior");
        if (!heal_pokemon(stream,context,"Hearthome")) OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,"Hearthome Center exit not confirmed.",stream);
        // Let the exit scene and camera settle before spending the Hall travel
        // budget. The old east hold began while the exit was still fading in.
        segment03_finish_transition(stream,context,"Hearthome Center heal exit");
        pbf_wait(context,1500ms);
        context.wait_for_all_requests();
        OverworldWatcher center_exit_ready;
        if (wait_until(stream,context,10s,{{center_exit_ready}})!=0)
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Hearthome Center: Exterior not ready after healing.",stream);
        pbf_move_left_joystick(context,{1,0},2092ms,185ms);
        // Bring the Hall into view without the old 1004ms north overshoot.
        pbf_move_left_joystick(context,{0,1},600ms,500ms);
        context.wait_for_all_requests();
        align_to_overworld_landmarks(stream,context,match_hearthome_contest_fantina,
            "Hearthome Contest Hall Fantina",false,true);
        // Confirm the actual first interaction before clearing its text. Do not
        // spend the old blind north/west holds against the side of the Hall.
        pbf_move_left_joystick(context,{0,1},16ms,100ms);
        pbf_press_button(context,BUTTON_A,194ms,277ms);
        context.wait_for_all_requests();
        ShortDialogWatcher fantina_dialogue;
        if (wait_until(stream,context,15s,{{fantina_dialogue}})!=0)
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Hearthome Contest Hall: Fantina dialogue did not start.",stream);
        clear_post_battle_dialogue(stream,context,BUTTON_B);
        pbf_wait(context,2000ms);
        clear_post_battle_dialogue(stream,context,BUTTON_B);
        // Preserve the separate north1179 Hall-entry phase and initial choice
        // inputs. A failed doorway or absent conversation must not finish18.
        segment03_cycling_move(stream,context,0,1,1179ms,4268ms,
            "Hearthome Contest Hall entrance",true);
        pbf_press_button(context,BUTTON_A,181ms,219ms);
        context.wait_for_all_requests();
        ShortDialogWatcher contest_dialogue;
        if (wait_until(stream,context,15s,{{contest_dialogue}})!=0)
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Hearthome Contest Hall: Interior dialogue did not start.",stream);
        pbf_press_button(context,BUTTON_A,195ms,123ms);
        pbf_press_button(context,BUTTON_A,185ms,121ms);
        pbf_press_button(context,BUTTON_A,170ms,805ms);
        pbf_press_button(context,BUTTON_A,195ms,140ms);
        pbf_press_button(context,BUTTON_A,102ms,218ms);
        clear_post_battle_dialogue(stream,context,BUTTON_B);
        pbf_wait(context,2000ms);
        clear_post_battle_dialogue(stream,context,BUTTON_B);
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
        pbf_press_button(context, BUTTON_X, 212ms, 0ms);
        pbf_wait(context, 683ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 184ms, 0ms);
        pbf_wait(context, 415ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 129ms, 0ms);
        pbf_wait(context, 190ms);
        pbf_press_button(context, BUTTON_A, 196ms, 0ms);
        pbf_wait(context, 1326ms);
        pbf_press_button(context, BUTTON_A, 144ms, 0ms);
        pbf_wait(context, 591ms);
        pbf_press_button(context, BUTTON_A, 142ms, 0ms);
        pbf_wait(context, 1017ms);
        pbf_press_button(context, BUTTON_A, 160ms, 0ms);
        pbf_wait(context, 411ms);
        pbf_press_button(context, BUTTON_B, 190ms, 0ms);
        pbf_wait(context, 594ms);
        pbf_press_button(context, BUTTON_B, 172ms, 0ms);
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
        pbf_press_button(context, BUTTON_A, 181ms, 0ms);
        pbf_wait(context, 136ms);
        pbf_press_button(context, BUTTON_A, 177ms, 0ms);
        pbf_wait(context, 137ms);
        pbf_press_button(context, BUTTON_A, 146ms, 0ms);
        pbf_wait(context, 286ms);
        pbf_press_button(context, BUTTON_A, 150ms, 0ms);
        pbf_wait(context, 175ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 310ms, 0ms);
        pbf_move_left_joystick(context, {-1.000000, -1.000000}, 124ms, 0ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 377ms, 0ms);
        pbf_wait(context, 320ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 327ms, 0ms);
        pbf_wait(context, 118ms);
        pbf_press_button(context, BUTTON_A, 213ms, 0ms);
        pbf_wait(context, 173ms);
        pbf_press_button(context, BUTTON_A, 205ms, 0ms);
        pbf_wait(context, 106ms);
        pbf_press_button(context, BUTTON_A, 170ms, 0ms);
        pbf_wait(context, 115ms);
        pbf_press_button(context, BUTTON_A, 177ms, 0ms);
        pbf_wait(context, 145ms);
        pbf_press_button(context, BUTTON_A, 194ms, 0ms);
        pbf_wait(context, 334ms);
        pbf_press_button(context, BUTTON_A, 217ms, 0ms);
        pbf_wait(context, 174ms);
        pbf_press_button(context, BUTTON_A, 190ms, 0ms);
        pbf_wait(context, 234ms);
        pbf_press_button(context, BUTTON_A, 233ms, 0ms);
        pbf_wait(context, 119ms);
        pbf_press_button(context, BUTTON_A, 201ms, 0ms);
        pbf_wait(context, 133ms);
        pbf_press_button(context, BUTTON_A, 195ms, 0ms);
        pbf_wait(context, 123ms);
        pbf_press_button(context, BUTTON_A, 214ms, 0ms);
        pbf_wait(context, 109ms);
        pbf_press_button(context, BUTTON_B, 217ms, 0ms);
        pbf_wait(context, 157ms);
        pbf_press_button(context, BUTTON_B, 162ms, 0ms);
        pbf_wait(context, 523ms);
        pbf_press_button(context, BUTTON_B, 192ms, 0ms);
        pbf_wait(context, 79ms);
        pbf_press_button(context, BUTTON_B, 132ms, 0ms);
        pbf_wait(context, 115ms);
        pbf_press_button(context, BUTTON_B, 126ms, 0ms);
        pbf_wait(context, 211ms);
        pbf_press_button(context, BUTTON_B, 156ms, 0ms);
        pbf_wait(context, 528ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 156ms, 0ms);
        pbf_wait(context, 559ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 171ms, 0ms);
        pbf_wait(context, 65ms);
        pbf_press_button(context, BUTTON_A, 204ms, 0ms);
        pbf_wait(context, 172ms);
        pbf_press_button(context, BUTTON_A, 177ms, 0ms);
        pbf_wait(context, 113ms);
        pbf_press_button(context, BUTTON_A, 173ms, 0ms);
        pbf_wait(context, 92ms);
        pbf_press_button(context, BUTTON_A, 158ms, 0ms);
        pbf_wait(context, 102ms);
        pbf_press_button(context, BUTTON_A, 168ms, 0ms);
        pbf_wait(context, 104ms);
        pbf_press_button(context, BUTTON_A, 179ms, 0ms);
        pbf_wait(context, 106ms);
        pbf_press_button(context, BUTTON_A, 194ms, 0ms);
        pbf_wait(context, 402ms);
        pbf_press_button(context, BUTTON_A, 182ms, 0ms);
        pbf_wait(context, 390ms);
        pbf_press_button(context, BUTTON_A, 193ms, 0ms);
        pbf_wait(context, 571ms);
        pbf_press_button(context, BUTTON_A, 213ms, 0ms);
        pbf_wait(context, 2265ms);
        pbf_press_button(context, BUTTON_A, 218ms, 0ms);
        pbf_wait(context, 934ms);
        pbf_press_button(context, BUTTON_A, 232ms, 0ms);
        pbf_wait(context, 171ms);
        pbf_press_button(context, BUTTON_A, 180ms, 0ms);
        pbf_wait(context, 488ms);
        pbf_press_button(context, BUTTON_A, 229ms, 0ms);
        pbf_wait(context, 154ms);
        pbf_press_button(context, BUTTON_A, 193ms, 0ms);
        pbf_wait(context, 287ms);
        pbf_press_button(context, BUTTON_A, 199ms, 0ms);
        pbf_wait(context, 855ms);
        pbf_press_button(context, BUTTON_A, 249ms, 0ms);
        pbf_wait(context, 594ms);
        pbf_press_button(context, BUTTON_A, 218ms, 0ms);
        pbf_wait(context, 172ms);
        pbf_press_button(context, BUTTON_A, 172ms, 0ms);
        pbf_wait(context, 363ms);
        pbf_press_button(context, BUTTON_A, 207ms, 0ms);
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
        pbf_press_button(context, BUTTON_B, 175ms, 0ms);
        pbf_wait(context, 222ms);
        pbf_press_button(context, BUTTON_B, 141ms, 0ms);
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
        pbf_press_button(context, BUTTON_A, 178ms, 0ms);
        pbf_wait(context, 540ms);
        pbf_press_button(context, BUTTON_A, 159ms, 0ms);
        pbf_wait(context, 655ms);
        pbf_press_button(context, BUTTON_A, 168ms, 0ms);
        pbf_wait(context, 1683ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 163ms, 0ms);
        pbf_wait(context, 2372ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 160ms, 0ms);
        pbf_wait(context, 454ms);
        pbf_press_button(context, BUTTON_A, 165ms, 0ms);
        pbf_wait(context, 1591ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 147ms, 0ms);
        pbf_wait(context, 853ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 91ms, 0ms);
        pbf_wait(context, 389ms);
        pbf_press_button(context, BUTTON_A, 162ms, 0ms);
        pbf_wait(context, 438ms);
        pbf_press_button(context, BUTTON_A, 160ms, 0ms);
        pbf_wait(context, 106ms);
        pbf_press_button(context, BUTTON_A, 144ms, 0ms);
        pbf_wait(context, 287ms);
        pbf_press_button(context, BUTTON_A, 169ms, 0ms);
        pbf_wait(context, 534ms);
        pbf_press_button(context, BUTTON_A, 160ms, 0ms);
        pbf_wait(context, 655ms);
        pbf_press_button(context, BUTTON_B, 211ms, 0ms);
        pbf_wait(context, 473ms);
        pbf_press_button(context, BUTTON_B, 168ms, 0ms);
        pbf_wait(context, 401ms);
        pbf_press_button(context, BUTTON_B, 175ms, 0ms);
        pbf_wait(context, 439ms);
        pbf_press_button(context, BUTTON_B, 78ms, 0ms);
        pbf_wait(context, 723ms);
        pbf_press_button(context, BUTTON_B, 159ms, 0ms);
        pbf_wait(context, 213ms);
        pbf_press_button(context, BUTTON_B, 122ms, 0ms);
        pbf_wait(context, 544ms);
        pbf_press_button(context, BUTTON_B, 144ms, 0ms);
        pbf_wait(context, 567ms);
        pbf_press_button(context, BUTTON_B, 167ms, 0ms);
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
        pbf_press_button(context, BUTTON_A, 213ms, 0ms);
        pbf_wait(context, 144ms);
        pbf_press_button(context, BUTTON_A, 184ms, 0ms);
        pbf_wait(context, 550ms);
        pbf_press_button(context, BUTTON_A, 211ms, 0ms);
        pbf_wait(context, 122ms);
        pbf_press_button(context, BUTTON_A, 188ms, 0ms);
        pbf_wait(context, 130ms);
        pbf_press_button(context, BUTTON_A, 160ms, 0ms);
        pbf_wait(context, 272ms);
        pbf_press_button(context, BUTTON_A, 168ms, 0ms);
        pbf_wait(context, 5334ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 159ms, 0ms);
        pbf_wait(context, 204ms);
        pbf_press_button(context, BUTTON_A, 170ms, 0ms);
        pbf_wait(context, 698ms);
        pbf_press_button(context, BUTTON_A, 195ms, 0ms);
        pbf_wait(context, 230ms);
        pbf_press_button(context, BUTTON_A, 169ms, 0ms);
        pbf_wait(context, 773ms);
        pbf_press_button(context, BUTTON_X, 212ms, 0ms);
        pbf_wait(context, 1555ms);
        pbf_press_button(context, BUTTON_X, 233ms, 0ms);
        pbf_wait(context, 1255ms);
        pbf_press_button(context, BUTTON_A, 173ms, 0ms);
        pbf_wait(context, 1640ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 177ms, 0ms);
        pbf_wait(context, 278ms);
        pbf_press_button(context, BUTTON_A, 189ms, 0ms);
        pbf_wait(context, 506ms);
        pbf_press_button(context, BUTTON_A, 182ms, 0ms);
        pbf_wait(context, 700ms);
        pbf_press_button(context, BUTTON_A, 167ms, 0ms);
        pbf_wait(context, 869ms);
        pbf_press_button(context, BUTTON_B, 176ms, 0ms);
        pbf_wait(context, 356ms);
        pbf_press_button(context, BUTTON_B, 211ms, 0ms);
        pbf_wait(context, 257ms);
        pbf_press_button(context, BUTTON_B, 158ms, 0ms);
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
        pbf_press_button(context, BUTTON_A, 174ms, 0ms);
        pbf_wait(context, 841ms);
        pbf_press_button(context, BUTTON_A, 184ms, 0ms);
        pbf_wait(context, 549ms);
        pbf_press_button(context, BUTTON_A, 177ms, 0ms);
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
        pbf_press_button(context, BUTTON_B, 189ms, 0ms);
        pbf_wait(context, 222ms);
        pbf_press_button(context, BUTTON_B, 183ms, 0ms);
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
        pbf_press_button(context, BUTTON_A, 182ms, 0ms);
        pbf_wait(context, 388ms);
        pbf_press_button(context, BUTTON_A, 211ms, 0ms);
        pbf_wait(context, 176ms);
        pbf_press_button(context, BUTTON_A, 172ms, 0ms);
        pbf_wait(context, 509ms);
        pbf_press_button(context, BUTTON_A, 198ms, 0ms);
        pbf_wait(context, 255ms);
        pbf_press_button(context, BUTTON_A, 170ms, 0ms);
        pbf_wait(context, 408ms);
        pbf_press_button(context, BUTTON_A, 212ms, 0ms);
        pbf_wait(context, 189ms);
        pbf_press_button(context, BUTTON_A, 166ms, 0ms);
        pbf_wait(context, 458ms);
        pbf_press_button(context, BUTTON_A, 168ms, 0ms);
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
std::string AutoStory_Checkpoint_0302::end_text()   const { return "Inside Mount Coronet at the west entrance after defeating Hiker Justin, facing east."; }

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
            // Set daylight once for this outdoor locator strip, including the
            // Center, Mart return, bicycle shop, and north Route 206 gate.
            context.wait_for_all_requests();
            env.console.log("[Eterna locator setup] Setting 14:00; preserving the current date.");
            DateTime locator_time;
            locator_time.hour = 14;
            locator_time.minute = 0;
            locator_time.second = 0;
            set_time_fast(env, context, locator_time, true);
            pbf_wait(context, 5000ms);
            context.wait_for_all_requests();
            // Recorded route with feedback battle handling.
            jupiter_to_coronet(env.console, context);

        },

        true
    );
}
std::string AutoStory_Checkpoint_0303::name()       const { return "03.03 - Mount Coronet"; }
std::string AutoStory_Checkpoint_0303::start_text() const { return "Inside Mount Coronet at the west entrance after defeating Hiker Justin, facing east."; }
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
