/*  BDSP AutoStory - Segment 06
 *  From after acquiring the fifth badge until after acquiring the sixth badge.
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
#include "PokemonBDSP_AutoStory_Segment_06.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


std::string AutoStory_Segment_06::name()       const{ return "06: Fifth Badge to Sixth Badge"; }
std::string AutoStory_Segment_06::start_text() const{ return "Start: Fifth Gym Badge acquired (Veilstone City)."; }
std::string AutoStory_Segment_06::end_text()   const{ return "End: Sixth Gym Badge acquired (Canalave City)."; }

void AutoStory_Segment_06::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_019().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}


std::string AutoStory_Checkpoint_019::name()       const{ return "019 - " + AutoStory_Segment_06().name(); }
std::string AutoStory_Checkpoint_019::start_text() const{ return "Fifth Gym Badge acquired (Veilstone City)."; }
std::string AutoStory_Checkpoint_019::end_text()   const{ return "Sixth Gym Badge acquired (Canalave City)."; }

void AutoStory_Checkpoint_019::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_019(env, context, options, stats);
}

static void move_back_after_unexpected_battle(
    VideoStream& stream,
    ProControllerContext& context,
    const std::string& trainerid
){
    stream.log("Starting battle with " + trainerid, COLOR_GREEN);
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    if (trainerid == "route_218_sailor_skyler") {
        pbf_move_left_joystick(context, {0, +1}, 200ms, 100ms); // 1+
        pbf_move_left_joystick(context, {-1, 0}, 200ms, 100ms); // 1+
    }
}

static bool handle_battle(
    VideoStream& stream,
    ProControllerContext& context,
    const std::string& trainerid
){
    context.wait_for_all_requests();
    BattleMenuWatcher battle_menu(BattleType::TRAINER);
    int ret_menu = wait_until(stream, context, std::chrono::seconds(30), {{battle_menu}});
    if (ret_menu < 0){
        stream.log("handle_battle_" + trainerid + ": battle menu not detected!", COLOR_RED);
        return false;
    }
    pbf_wait(context, 500ms);
    pbf_press_button(context, BUTTON_A, 80ms, 300ms);
    pbf_wait(context, 500ms);

    if (
        trainerid == "route_218_sailor_skyler"
    ) {
        // Select Razor leaf
        pbf_press_dpad(context, DPAD_UP, 280ms, 200ms);
    }
    context.wait_for_all_requests();

    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_A, 200000ms);
        },
        {{black_screen}}
    );
    if (ret < 0){
        stream.log("handle_battle_" + trainerid + ": black screen not detected!", COLOR_RED);
        return false;
    }
    context.wait_for_all_requests();
    pbf_mash_button(context, BUTTON_B, 2000ms);
    stream.log("handle_battle_" + trainerid + ": transition confirmed.");
    return true;
}

static bool cynthia_and_fly_to_jubilife(
    VideoStream& stream,
    ProControllerContext& context
){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("cynthia_and_fly_to_jubilife: talking to Cynthia and then flying to Jubilife.");

    /*
    {
        ShortDialogWatcher talk_to_cynthia(COLOR_BLUE);
        int ret = wait_until(stream, context, std::chrono::seconds(30), {{talk_to_cynthia}});
        if (ret < 0){
            stream.log("cynthia_and_fly_to_jubilife: dialog box not detected!", COLOR_RED);
            return false;
        } else if (ret == 0) {
            stream.log("cynthia_and_fly_to_jubilife: dialog box detected.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 12000ms);
            context.wait_for_all_requests();
            open_menu(stream, context, MenuCursorPosition::MAP, 8);
            fly_to(stream, context, FlyPoint::JubilifeCity);
        }
    }*/
    // Temporary, will be removed when the gym exit is done

    context.wait_for_all_requests();
    open_menu(stream, context, MenuCursorPosition::MAP, 8);
    fly_to(stream, context, FlyPoint::JubilifeCity);

    // End of temporary addition

    return true;
}

static bool enter_jubilife_pokemon_center(
    VideoStream& stream,
    ProControllerContext& context
){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("enter_jubilife_pokemon_center: entering...");

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
            stream.log("enter_jubilife_pokemon_center: black screen not detected!", COLOR_RED);
            return false;
        }
    };
    return true;
}

static bool heal_and_exit_jubilife_pokemon_center(
    VideoStream& stream,
    ProControllerContext& context
){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("heal_and_exit_jubilife_pokemon_center: healing and exiting...");
    heal_pokemon(stream, context, "Jubilife city");
    return true;
}

static bool move_to_jubilife_west_exit(
    VideoStream& stream,
    ProControllerContext& context
){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("move_to_jubilife_west_exit: moving towards building...");
    pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms); // 6
    pbf_move_left_joystick(context, {0, +1}, 2600ms, 100ms); // 18

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 100000ms, 100ms); // 46
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("move_to_jubilife_west_exit: transition not detected!", COLOR_RED);
            return false;
        } else if (ret == 0) {
            stream.log("move_to_jubilife_west_exit: transition detected.", COLOR_GREEN);
        }
    }

    return true;
}

static bool go_through_jubilife_west_exit(
    VideoStream& stream,
    ProControllerContext& context
){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_through_jubilife_west_exit: going through the building...");

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 100000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("go_through_jubilife_west_exit: transition not detected!", COLOR_RED);
            return false;
        } else if (ret == 0) {
            stream.log("go_through_jubilife_west_exit: transition detected.", COLOR_GREEN);
        }
    }

    return true;
}

static bool go_through_route_218(
    VideoStream& stream,
    ProControllerContext& context
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_through_route_218: going through the road...");

    pbf_move_left_joystick(context, {-1, 0}, 1800ms, 100ms); // 7+
    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 2);
    pbf_move_left_joystick(context, {-1, 0}, 1800ms, 100ms); // 9+
    pbf_move_left_joystick(context, {0, +1}, 1800ms, 100ms); // 9+
    context.wait_for_all_requests();
    activate_repel(stream, context);
    pbf_mash_button(context, BUTTON_A, 2000ms);
    context.wait_for_all_requests();
    pbf_wait(context, 6000ms);
    context.wait_for_all_requests();

    {
        MarkDetector sailor_skyler_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms); // 2+
                pbf_move_left_joystick(context, {-1, 0}, 6000ms, 100ms); // 28++ (getting out of the water)
                pbf_move_left_joystick(context, {0, -1}, 2800ms, 100ms); // 14+
            },
            {{sailor_skyler_fight}}
        );
        if (ret == 0){
            context.wait_for_all_requests();
            pbf_wait(context, 2000ms);

            BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            ret = run_until<ProControllerContext>(
                stream, context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 200000ms);
                },
                {{black_screen}}
            );
            if (ret < 0){
                stream.log("go_through_route_218_sailor_skyler: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "route_218_sailor_skyler");
                context.wait_for_all_requests();
                pbf_wait(context, 3000ms);
                move_back_after_unexpected_battle(stream, context, "route_218_sailor_skyler");
            }
        }
    }
    
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 2);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 100000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("go_through_route_218: transition not detected!", COLOR_RED);
            return false;
        } else if (ret == 0) {
            stream.log("go_through_route_218: transition detected.", COLOR_GREEN);
        }
    }
    return true;
}

static bool go_through_building_route_218(
    VideoStream& stream,
    ProControllerContext& context
){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_through_building_route_218: going through the building...");

    {
        ShortDialogWatcher talk_to_scientist(COLOR_BLUE);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 100000ms, 100ms);
            },
            {{talk_to_scientist}}
        );
        if (ret < 0){
            stream.log("go_through_building_route_218_scientist: dialog box not detected!", COLOR_RED);
            return false;
        } else if (ret == 0) {
            stream.log("go_through_building_route_218_scientist: dialog box detected.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 12000ms);
            context.wait_for_all_requests();
        }
    }

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 100000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("go_through_building_route_218: transition not detected!", COLOR_RED);
            return false;
        } else if (ret == 0) {
            stream.log("go_through_building_route_218: transition detected.", COLOR_GREEN);
        }
    }
    return true;
}


void checkpoint_019(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!cynthia_and_fly_to_jubilife(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "cynthia_and_fly_to_jubilife: transition not detected.", env.console);
            }
            if (!enter_jubilife_pokemon_center(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "enter_jubilife_pokemon_center: transition not detected.", env.console);
            }
            if (!heal_and_exit_jubilife_pokemon_center(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "heal_and_exit_jubilife_pokemon_center: transition not detected.", env.console);
            }
            if (!move_to_jubilife_west_exit(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "move_to_jubilife_west_exit: transition not detected.", env.console);
            }
            if (!go_through_jubilife_west_exit(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_through_jubilife_west_exit: transition not detected.", env.console);
            }
            if (!go_through_route_218(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_through_route_218: transition not detected.", env.console);
            }
            if (!go_through_building_route_218(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_through_building_route_218: transition not detected.", env.console);
            }
        }
    );
}


}
}
}
