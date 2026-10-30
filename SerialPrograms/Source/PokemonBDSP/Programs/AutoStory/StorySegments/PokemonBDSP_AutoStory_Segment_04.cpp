/*  BDSP AutoStory - Segment 04
 *  From after acquiring the third badge until after acquiring the fourth badge.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <vector>

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "CommonFramework/Globals.h"

#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/VisualDetectors/ImageMatchDetector.h"

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"

#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_MarkFinder.h"
#include "PokemonBDSP/Programs/AutoStory/PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP/Programs/AutoStory/Utils/PokemonBDSP_AutoStory_Battle.h"
#include "PokemonBDSP/Programs/AutoStory/Utils/PokemonBDSP_AutoStory_Segment_00_tools.h"

#include "PokemonBDSP_AutoStory_Segment_04.h"

using namespace std::chrono_literals;

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonBDSP {

// ---------------------------------------------------------------------------
//  Helpers
// ---------------------------------------------------------------------------

static void
move_back_after_unexpected_battle(VideoStream& stream, ProControllerContext& context, const std::string& trainerid){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    if (trainerid == "route_213_tuber_chelsea"){

        pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms);  // 4+
        pbf_move_left_joystick(context, {0, +1}, 1000ms, 100ms); // 4+
        pbf_move_left_joystick(context, {0, -1}, 800ms, 100ms);  // 5
    }else if (trainerid == "route_213_tuber_jared"){
        // Just need to wait, nothing else
    }else if (trainerid == "route_213_beauty_cindy"){
        pbf_move_left_joystick(context, {0, +1}, 1400ms, 100ms); // 7+
    }
}

static bool handle_battle(VideoStream& stream,
    ProControllerContext& context,
    const std::string& trainerid,
    AutoStoryStats& stats){
    fight_trainer_using_moves(stream, context, stats, trainerid, true);
    if (trainerid == "route_213_tuber_chelsea" || trainerid == "route_213_tuber_jared" ||
        trainerid == "route_213_beauty_cindy"){
        move_back_after_unexpected_battle(stream, context, trainerid);
    }
    return true;
}

// ---------------------------------------------------------------------------
//  Named route navigation blocks
// ---------------------------------------------------------------------------

static bool leave_gym(VideoStream& stream, ProControllerContext& context){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("leave_gym: walking out of gym");

    pbf_move_left_joystick(context, {0, -1}, 1200ms, 100ms); // 2
    pbf_move_left_joystick(context, {+1, 0}, 300ms, 100ms);  // 2

    pbf_press_dpad(context, DPAD_RIGHT, 280ms, 200ms);
    pbf_move_left_joystick(context, {0, -1}, 1000ms, 100ms); // 2
    pbf_press_dpad(context, DPAD_LEFT, 280ms, 200ms);
    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {0, -1}, 800ms, 100ms); // 4
    pbf_press_dpad(context, DPAD_RIGHT, 280ms, 200ms);
    pbf_move_left_joystick(context, {0, -1}, 1000ms, 100ms); // 2
    pbf_press_dpad(context, DPAD_LEFT, 280ms, 200ms);
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, -1}, 10000ms, 0ms);
        },
        {{black_screen}});
    if (ret < 0){
        stream.log("leave_gym: black screen not detected!", COLOR_RED);
        return false;
    }
    stream.log("leave_gym: transition confirmed.");
    return true;
}

static bool go_to_veilstone_pokemon_center(VideoStream& stream, ProControllerContext& context){
    DpadState dpad;
    stream.log("Walking towards the pokemon center");
    pbf_wait(context, 4000ms);
    wait_for_dialogue(stream, context, "Dawn");
    mash_until_dialogue_ends(stream, context, BUTTON_B);
    context.wait_for_all_requests();
    pbf_wait(context, 4000ms);

    pbf_move_left_joystick(context, {0, -1}, 3000ms, 100ms); // 21
    pbf_move_left_joystick(context, {+1, 0}, 2000ms, 100ms); // 14
    pbf_move_left_joystick(context, {0, +1}, 600ms, 100ms);  // 4
    pbf_move_left_joystick(context, {+1, 0}, 3400ms, 100ms); // 25
    pbf_move_left_joystick(context, {0, +1}, 1600ms, 100ms); // 11
    pbf_move_left_joystick(context, {-1, 0}, 1600ms, 100ms); // 12
    pbf_move_left_joystick(context, {0, +1}, 800ms, 100ms);  // 5
    pbf_move_left_joystick(context, {+1, 0}, 1400ms, 100ms); // 8+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 2, true);
    context.wait_for_all_requests();

    /* Entering the Pokemon Center */
    BlackScreenOverWatcher black_screen1(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
        },
        {{black_screen1}});
    if (ret < 0){
        stream.log("Didn't enter pokemon center!", COLOR_RED);
        return false;
    }

    stream.log("Entered pokemon center successfully!");

    stream.log("go_to_veilstone_pokemon_center: transition confirmed.");
    return true;
}

static bool heal_at_pokemon_center(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats){
    return heal_pokemon(stream, context, "Veilstone", &stats);
}

static bool go_to_galactic_warehouse(VideoStream& stream, ProControllerContext& context){
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {0, -1}, 800ms, 100ms);  // 3+
    pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms);  // 6
    pbf_move_left_joystick(context, {0, -1}, 1400ms, 100ms); // 6+
    pbf_move_left_joystick(context, {-1, 0}, 1000ms, 100ms); // 7
    pbf_move_left_joystick(context, {0, +1}, 2400ms, 100ms); // 9+
    pbf_move_left_joystick(context, {-1, 0}, 2800ms, 100ms); // 12+
    pbf_move_left_joystick(context, {0, +1}, 4000ms, 100ms); // 15+
    pbf_move_left_joystick(context, {+1, 0}, 1400ms, 100ms); // 5+
    pbf_press_dpad(context, DPAD_DOWN, 400ms, 200ms);

    context.wait_for_all_requests();
    BlackScreenOverWatcher battle_black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_A, 12000ms);
        },
        {{battle_black_screen}});
    if (ret < 0){
        stream.log("go_to_galactic_warehouse: battle black screen not detected (2)", COLOR_RED);
        return false;
    }
    stream.log("go_to_galactic_warehouse: battle transition confirmed.");
    return true;
}

static bool enter_galactic_warehouse(VideoStream& stream, ProControllerContext& context){
    DpadState dpad;
    pbf_wait(context, 3000ms);
    context.wait_for_all_requests();

    pbf_mash_button(context, BUTTON_B, 16000ms);
    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {+1, 0}, 1200ms, 100ms); // 6+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1, true);

    context.wait_for_all_requests();

    BlackScreenOverWatcher black_screen_1(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
        },
        {{black_screen_1}});
    if (ret < 0){
        stream.log("Entering Galactic warehouse black screen not detected!", COLOR_RED);
        return false;
    }
    stream.log("enter_galactic_warehouse: transition confirmed.");
    return true;
}

static bool exit_galactic_warehouse(VideoStream& stream, ProControllerContext& context){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {+1, 0}, 1600ms, 100ms); // 7+
    pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms);  // 2+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 2, true);
    pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms); // 2+
    context.wait_for_all_requests();
    // Get fly

    ShortDialogWatcher getting_fly_TM(COLOR_BLUE);
    int ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_A, 2000ms);
        },
        {{getting_fly_TM}});
    if (ret < 0){
        stream.log("enter_galactic_warehouse_getting_fly_TM: dialog box not detected!", COLOR_RED);
        return false;
    }
    if (ret == 0){
        stream.log("enter_galactic_warehouse_getting_fly_TM: getting TM Fly not detected!");
        pbf_mash_button(context, BUTTON_A, 9000ms);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    pbf_move_left_joystick(context, {0, -1}, 1600ms, 100ms); // 5+
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 5, true);

    // Black screen detector here when exiting the warehouse
    BlackScreenOverWatcher black_screen_2(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, -1}, 10000ms, 100ms);
        },
        {{black_screen_2}});
    if (ret < 0){
        stream.log("enter_galactic_warehouse: black screen not detected!", COLOR_RED);
        return false;
    }
    stream.log("exit_galactic_warehouse: transition confirmed.");
    return true;
}

static bool go_to_route_214(VideoStream& stream, ProControllerContext& context){
    DpadState dpad;
    context.wait_for_all_requests();
    stream.log("Moving towards Route 214");
    pbf_wait(context, 3000ms);
    pbf_move_left_joystick(context, {0, -1}, 1600ms, 100ms); // 5+
    pbf_move_left_joystick(context, {-1, 0}, 1300ms, 100ms); // 8
    pbf_move_left_joystick(context, {0, -1}, 3000ms, 100ms); // 13+
    pbf_move_left_joystick(context, {+1, 0}, 3000ms, 100ms); // 15+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 4, true);

    pbf_move_left_joystick(context, {0, -1}, 2600ms, 100ms); // 14
    pbf_move_left_joystick(context, {-1, 0}, 1200ms, 100ms); // 6+
    pbf_move_left_joystick(context, {0, -1}, 2000ms, 100ms); // 10+
    pbf_move_left_joystick(context, {+1, 0}, 1400ms, 100ms); // 7+

    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 3, true);

    pbf_move_left_joystick(context, {+1, 0}, 2200ms, 100ms); // 15
    context.wait_for_all_requests();

    BlackScreenOverWatcher black_screen_1(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, -1}, 10000ms, 100ms);
        },
        {{black_screen_1}});
    if (ret < 0){
        stream.log("go_to_route_214_enter_house: black screen not detected!", COLOR_RED);
        return false;
    }
    stream.log("Entered house");
    pbf_wait(context, 1500ms);

    context.wait_for_all_requests();
    // Add a black screen detector here when entering the connecting house
    BlackScreenOverWatcher black_screen_2(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, -1}, 10000ms, 100ms);
        },
        {{black_screen_2}});
    if (ret < 0){
        stream.log("go_to_route_214_exit_house: black screen not detected!", COLOR_RED);
        return false;
    }
    stream.log("Exited house");
    pbf_wait(context, 1500ms);

    pbf_move_left_joystick(context, {0, -1}, 6000ms, 100ms);

    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_A, 200000ms);
        },
        {{black_screen}});
    if (ret < 0){
        stream.log("go_to_route_214: black screen not detected!", COLOR_RED);
        return false;
    }
    stream.log("go_to_route_214: transition confirmed.");
    return true;
}

static bool
walk_through_route_214(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats){
    context.wait_for_all_requests();
    stream.log("walk_through_route_214: Continuing on the first part of route 214");
    pbf_wait(context, 3000ms);
    DpadState dpad;

    pbf_move_left_joystick(context, {0, -1}, 600ms, 100ms);  // 4
    pbf_move_left_joystick(context, {+1, 0}, 3000ms, 100ms); // 13+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 3, true);

    if (!activate_repel(stream, context)){
        return false;
    }

    {
        MarkDetector battle_dialogue_1(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, -1}, 30000ms, 100ms); // 13+
            },
            {{battle_dialogue_1}});
        if (ret == 0){
            context.wait_for_all_requests();
            BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            ret = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 200000ms);
                },
                {{black_screen}});
            if (ret < 0){
                stream.log("walk_through_route_214_collector_brady: black screen not detected!", COLOR_RED);
                return false;
            };
            handle_battle(stream, context, "route_214_collector_brady", stats);
        }
    }
    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1, true);
    pbf_move_left_joystick(context, {0, -1}, 1200ms, 100ms); // 6+
    pbf_move_left_joystick(context, {-1, 0}, 400ms, 100ms);  // 2
    pbf_move_left_joystick(context, {0, -1}, 1000ms, 100ms); // 5+

    pbf_move_left_joystick(context, {+1, 0}, 1600ms, 100ms); // 8+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1, true);
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 6, true);

    {
        MarkDetector battle_dialogue_2(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_wait(context, 30000ms);
            },
            {{battle_dialogue_2}});
        if (ret == 0){
            context.wait_for_all_requests();
            BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            ret = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 200000ms);
                },
                {{black_screen}});
            if (ret < 0){
                stream.log("walk_through_route_214_psychic_mitchell: black screen not detected!", COLOR_RED);
                return false;
            };
            handle_battle(stream, context, "route_214_psychic_mitchell", stats);
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    pbf_move_left_joystick(context, {0, -1}, 1000ms, 100ms); // 6+

    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1, true);
    pbf_move_left_joystick(context, {0, -1}, 2200ms, 100ms); // 11+

    pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms); // 4+
    pbf_move_left_joystick(context, {0, -1}, 600ms, 100ms); // 3+
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 2, true);

    pbf_move_left_joystick(context, {0, -1}, 6200ms, 100ms); // 31+

    stream.log("walk_through_route_214: transition confirmed.");
    return true;
}

static bool pass_by_valor_lakefront(VideoStream& stream, ProControllerContext& context){
    context.wait_for_all_requests();
    stream.log("Finishing going through route 214");
    pbf_wait(context, 1000ms);
    DpadState dpad;

    {
        ShortDialogWatcher repel_ran_out(COLOR_BLUE);
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                DpadState dpad;
                dpad.last_dir = DPAD_DOWN;
                for (int c = 0; c < 20; c++){
                    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 1, true);
                    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 1, true);
                }
            },
            {{repel_ran_out}});
        if (ret == 0){
            pbf_mash_button(context, BUTTON_A, 2000ms);
        };
    }
    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, -1}, 1000ms, 100ms); // 20+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 6, true);

    pbf_move_left_joystick(context, {0, -1}, 8000ms, 100ms); // 41+
    pbf_move_left_joystick(context, {-1, 0}, 600ms, 100ms);  // 4
    pbf_move_left_joystick(context, {0, -1}, 800ms, 100ms);  // 5
    pbf_move_left_joystick(context, {-1, 0}, 2200ms, 100ms); // 10+
    pbf_move_left_joystick(context, {0, -1}, 800ms, 100ms);  // 4+
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 4, true);
    context.wait_for_all_requests();

    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, -1}, 30000ms, 100ms);
        },
        {{black_screen}});
    if (ret < 0){
        stream.log("walk_through_route_214: Didn't enter building!", COLOR_RED);
        return false;
    }

    return true;
}

static bool go_through_building_route_213(VideoStream& stream, ProControllerContext& context){
    stream.log("Going through building between routes 214 and 213");
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    ShortDialogWatcher repel_ran_out(COLOR_CYAN);
    int ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, -1}, 30000ms, 100ms);
        },
        {{repel_ran_out}});
    if (ret == 0){
        stream.log("go_through_building_route_214: Not reapplying repel.", COLOR_RED);
        pbf_mash_button(context, BUTTON_B, 2000ms);
    }

    context.wait_for_all_requests();

    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, -1}, 30000ms, 100ms);
        },
        {{black_screen}});
    if (ret < 0){
        stream.log("go_through_building_route_214: Didn't exit building!", COLOR_RED);
        return false;
    }
    stream.log("go_through_building_route_214: transition confirmed.");

    return true;
}

static bool
go_through_route_213(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats){
    context.wait_for_all_requests();
    stream.log("Going through route 213");
    pbf_wait(context, 2000ms);
    DpadState dpad;

    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 4, true);

    MarkDetector tuber_chelsea_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
    int ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {-1, 0}, 1000ms, 100ms); // 4+
            pbf_move_left_joystick(context, {0, -1}, 800ms, 100ms);  // 5
            context.wait_for_all_requests();
            pbf_wait(context, 2000ms);
        },
        {{tuber_chelsea_fight}});
    if (ret == 0){
        context.wait_for_all_requests();
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, 200000ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_route_213_tuber_chelsea: Didn't detect battle starting!", COLOR_RED);
            return false;
        }else if (ret == 0){
            stream.log("go_through_route_213_tuber_chelsea: Starting battle.");
            handle_battle(stream, context, "route_213_tuber_chelsea", stats);
            context.wait_for_all_requests();
            pbf_wait(context, 3000ms);
        }
    }
    context.wait_for_all_requests();

    MarkDetector tuber_jared_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
    ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, -1}, 1000ms, 100ms); // 4+
            pbf_move_left_joystick(context, {-1, 0}, 600ms, 100ms);  // 3
            context.wait_for_all_requests();
            pbf_wait(context, 1000ms);
        },
        {{tuber_jared_fight}});
    if (ret == 0){
        context.wait_for_all_requests();
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, 200000ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_route_213_tuber_jared: Didn't detect battle starting!", COLOR_RED);
            return false;
        }else if (ret == 0){
            stream.log("go_through_route_213_tuber_jared: Starting battle.");
            handle_battle(stream, context, "route_213_tuber_jared", stats);
            context.wait_for_all_requests();
            pbf_wait(context, 3000ms);
        }
    }

    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {-1, 0}, 3600ms, 100ms); // 20+
    pbf_move_left_joystick(context, {0, -1}, 400ms, 100ms);  // 3
    pbf_move_left_joystick(context, {-1, 0}, 2000ms, 100ms); // 13+

    MarkDetector beauty_cindy_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
    ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, +1}, 5000ms, 100ms); // 15+
        },
        {{beauty_cindy_fight}});
    if (ret == 0){
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, 200000ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_route_213_beauty_cindy: Didn't detect battle starting!", COLOR_RED);
            return false;
        }else if (ret == 0){
            context.wait_for_all_requests();
            stream.log("go_through_route_213_beauty_cindy: Starting battle.");
            handle_battle(stream, context, "route_213_beauty_cindy", stats);
            context.wait_for_all_requests();
            pbf_wait(context, 3000ms);
        }
    }

    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {+1, 0}, 800ms, 100ms);  // 5
    pbf_move_left_joystick(context, {0, +1}, 1000ms, 100ms); // 8

    if (!activate_repel(stream, context)){
        return false;
    }

    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {-1, 0}, 100000ms, 100ms);
        },
        {{black_screen}});
    if (ret < 0){
        stream.log("go_through_route_213: Didn't enter building!", COLOR_RED);
        return false;
    }
    stream.log("go_to_route_213: transition confirmed.");

    return true;
}

static bool go_through_house_east_pastoria(VideoStream& stream, ProControllerContext& context){
    stream.log("Going through the building at the entrance of Pastoria city");
    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {-1, 0}, 100000ms, 100ms);
        },
        {{black_screen}});
    if (ret < 0){
        stream.log("go_through_house_east_pastoria: Didn't exit building!", COLOR_RED);
        return false;
    }
    stream.log("go_through_house_east_pastoria: transition confirmed.");

    return true;
}

static bool go_through_pastoria(VideoStream& stream, ProControllerContext& context){
    stream.log("Going through Pastoria city and to the Pokemon center");
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms);  // 2+
    pbf_move_left_joystick(context, {-1, 0}, 4200ms, 100ms); // 23+
    pbf_move_left_joystick(context, {0, -1}, 500ms, 100ms);  // 3
    pbf_move_left_joystick(context, {-1, 0}, 600ms, 100ms);  // 4
    pbf_move_left_joystick(context, {0, -1}, 600ms, 100ms);  // 4

    ShortDialogWatcher repel_dialog(COLOR_RED);
    int ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {-1, 0}, 3400ms, 100ms); // 19
            pbf_move_left_joystick(context, {+1, 0}, 3400ms, 100ms); // 19
            pbf_move_left_joystick(context, {-1, 0}, 3400ms, 100ms); // 19
            pbf_move_left_joystick(context, {+1, 0}, 3400ms, 100ms); // 19
        },
        {{repel_dialog}});
    if (ret == 0){
        pbf_mash_button(context, BUTTON_B, 3000ms);
    }else if (ret < 0){
        stream.log("go_through_pastoria: Didn't detect repel dialog box!", COLOR_RED);
        return false;
    }

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {-1, 0}, 3400ms, 100ms); // 19

    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 8, true);

    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, +1}, 100000ms, 100ms);
        },
        {{black_screen}});
    if (ret < 0){
        stream.log("go_through_pastoria: Didn't enter Pokemon center!", COLOR_RED);
        return false;
    }
    stream.log("go_through_pastoria: transition confirmed.");

    return true;
}

static bool go_to_pastoria_gym(VideoStream& stream, ProControllerContext& context){
    stream.log("Going to Pastoria gym");
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {0, -1}, 800ms, 100ms);  // 4+
    pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms);  // 5
    pbf_move_left_joystick(context, {0, -1}, 1200ms, 100ms); // 8
    pbf_move_left_joystick(context, {-1, 0}, 2000ms, 100ms); // 10+
    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 3, true);

    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, +1}, 100000ms, 100ms);
        },
        {{black_screen}});
    if (ret < 0){
        stream.log("go_to_pastoria_gym: Didn't enter building!", COLOR_RED);
        return false;
    }
    stream.log("go_to_pastoria_gym: transition confirmed.");
    return true;
}

// Start inside Pastoria Gym at its entrance; finish outside after Wake rewards.
static void pastoria_gym_puzzle_and_wake(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryStats& stats){
    VideoStream& stream = env.console;
    struct Move {
        DpadPosition dir;
        int count;
        Milliseconds delay;
    };

    // Trial: use the supplied counts without an extra facing press.
    const std::vector<Move> sequences[] = {
        {
            // Trainer 1
            {DPAD_UP, 16, 0ms},
            {DPAD_LEFT, 11, 0ms},
            {DPAD_DOWN, 8, 0ms},
            {DPAD_DOWN, 1, 2000ms},
            {DPAD_RIGHT, 5, 0ms},
            {DPAD_UP, 7, 0ms},
            {DPAD_RIGHT, 10, 0ms},
            {DPAD_UP, 7, 0ms},
            {DPAD_LEFT, 4, 0ms},
            {DPAD_UP, 14, 0ms},
            {DPAD_RIGHT, 4, 0ms},
            {DPAD_RIGHT, 1, 2000ms},
            {DPAD_LEFT, 4, 0ms},
            {DPAD_DOWN, 6, 0ms},
            {DPAD_RIGHT, 3, 0ms},
            {DPAD_DOWN, 7, 0ms},
            {DPAD_LEFT, 12, 0ms},
            {DPAD_DOWN, 5, 0ms},
            {DPAD_RIGHT, 3, 0ms},
        },
        {
            // Trainer 2
            {DPAD_RIGHT, 2, 0ms},
            {DPAD_RIGHT, 1, 2000ms},
            {DPAD_LEFT, 4, 0ms},
            {DPAD_DOWN, 11, 0ms},
            {DPAD_RIGHT, 6, 0ms},
        },
        {
            // Trainer 3
            {DPAD_RIGHT, 10, 0ms},
            {DPAD_UP, 2, 0ms},
        },
        {
            // Trainer 4
            {DPAD_UP, 21, 0ms},
            {DPAD_LEFT, 1, 0ms},
            {DPAD_LEFT, 1, 2000ms},
            {DPAD_DOWN, 2, 0ms},
            {DPAD_RIGHT, 5, 0ms},
            {DPAD_DOWN, 17, 0ms},
            {DPAD_DOWN, 1, 2000ms},
            {DPAD_UP, 25, 0ms},
            {DPAD_LEFT, 18, 0ms},
            {DPAD_DOWN, 11, 0ms},
            {DPAD_LEFT, 4, 0ms},
            {DPAD_UP, 10, 0ms},
        },
        {
            // Crasher Wake approach
            {DPAD_UP, 2, 0ms},
            {DPAD_UP, 1, 4000ms},
            {DPAD_DOWN, 4, 0ms},
            {DPAD_RIGHT, 11, 0ms},
            {DPAD_UP, 5, 0ms},
        },
        {
            // Return from Crasher Wake
            {DPAD_UP, 3, 0ms},
            {DPAD_LEFT, 3, 0ms},
            {DPAD_UP, 3, 0ms},
            {DPAD_LEFT, 11, 0ms},
            {DPAD_DOWN, 35, 0ms},
            {DPAD_RIGHT, 10, 0ms},
            {DPAD_DOWN, 5, 0ms},
            {DPAD_RIGHT, 4, 0ms},
            {DPAD_DOWN, 2, 0ms},
        },
    };

    for (size_t stage = 0; stage < 6; stage++){
        stream.log("[Pastoria Gym puzzle] Movement stage " + std::to_string(stage + 1) + ".");
        if (stage == 5){
            // Observe the supplied return route and stop movement at the Gym exit fade.
            BlackScreenWatcher exit_black(COLOR_RED);
            if (run_until<ProControllerContext>(stream,
                context,
                [&](ProControllerContext& ctx){
                        for (const auto& move : sequences[stage]){
                            // repeat_dpad(ctx, dpad, move.dir, 80ms, 300ms, move.count, true);
                            for (int press = 0; press < move.count; press++){
                                pbf_press_dpad(ctx, move.dir, 80ms, 300ms);
                                if (move.delay > 0ms){
                                    pbf_wait(ctx, move.delay);
                                }
                            }
                        }
                        pbf_wait(ctx, 5000ms);
                    },
                {{exit_black}}) != 0){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Pastoria Gym return: Exit transition did not start.",
                    stream);
            }
            BlackScreenWatcher outside(COLOR_RED,
                {0.1, 0.1, 0.8, 0.8},
                100,
                10,
                BlackScreenWatcher::FinderType::GONE,
                500ms);
            if (wait_until(stream, context, 30s, {{outside}}) != 0){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Pastoria Gym return: Exit transition did not finish.",
                    stream);
            }
            pbf_wait(context, 500ms);
            context.wait_for_all_requests();
            // Let any automatic exit dialogue finish before 05.01 saves or opens the map.
            clear_post_battle_dialogue(stream, context, BUTTON_B, 4000ms);
            break;
        }
        for (const auto& move : sequences[stage]){
            // repeat_dpad(context, dpad, move.dir, 80ms, 300ms, move.count, true);
            for (int press = 0; press < move.count; press++){
                pbf_press_dpad(context, move.dir, 80ms, 300ms);
                if (move.delay > 0ms){
                    pbf_wait(context, move.delay);
                }
            }
        }
        context.wait_for_all_requests();

        const bool gym_leader = stage == 4;
        const std::string label =
            gym_leader ? "Crasher Wake" : "Pastoria Gym trainer " + std::to_string(stage + 1);
        pbf_press_button(context, BUTTON_A, 150ms, 300ms);
        fight_trainer_using_moves(stream, context, stats, label, gym_leader);
        clear_post_battle_dialogue(stream, context, BUTTON_B, gym_leader ? 6000ms : 3000ms);
    }
    stream.log("[Pastoria Gym puzzle] Reward dialogue cleared; outside the Gym doorway, ready for 05.01.",
        COLOR_GREEN);
}

// ---------------------------------------------------------------------------
//  Segment wrappers
// ---------------------------------------------------------------------------

std::string AutoStory_Segment_04::name() const {
    return "04: Veilstone to Pastoria";
}

std::string AutoStory_Segment_04::start_text() const {
    return "Start: Inside Veilstone Gym in front of Maylene, facing north, with third-badge rewards and dialogue finished.";
}

std::string AutoStory_Segment_04::end_text() const {
    return "Outside Pastoria Gym at the doorway, facing south, with fourth-badge rewards and dialogue finished.";
}

void AutoStory_Segment_04::run_segment(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);
    AutoStory_Checkpoint_0401().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0402().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0403().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0404().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0405().run_checkpoint(env, context, options, stats);
    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}

// ---------------------------------------------------------------------------
//  Checkpoint wrappers
// ---------------------------------------------------------------------------

std::string AutoStory_Checkpoint_0401::name() const {
    return "04.01 - Veilstone recovery";
}

std::string AutoStory_Checkpoint_0401::start_text() const {
    return "Inside Veilstone Gym in front of Maylene, facing north, with third-badge rewards and dialogue finished.";
}

std::string AutoStory_Checkpoint_0401::end_text() const {
    return "Healed and outside Veilstone Pokemon Center, with Dawn's post-Gym dialogue finished.";
}

void AutoStory_Checkpoint_0401::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0401(env, context, options, stats);
}

void checkpoint_0401(SingleSwitchProgramEnvironment& env,
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
                if (!leave_gym(env.console, context)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "leave_gym: transition not detected.",
                        env.console);
                }
            }
            {
                if (!go_to_veilstone_pokemon_center(env.console, context)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "go_to_veilstone_pokemon_center: transition not detected.",
                        env.console);
                }
            }
            {
                if (!heal_at_pokemon_center(env.console, context, stats)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "heal_at_pokemon_center: transition not detected.",
                        env.console);
                }
            }
            context.wait_for_all_requests();
        },
        true);
}

std::string AutoStory_Checkpoint_0402::name() const {
    return "04.02 - Galactic grunts and Fly";
}

std::string AutoStory_Checkpoint_0402::start_text() const {
    return "Healed and outside Veilstone Pokemon Center, with Dawn's post-Gym dialogue finished.";
}

std::string AutoStory_Checkpoint_0402::end_text() const {
    return "Outside Galactic Warehouse with Fly obtained and dialogue cleared.";
}

void AutoStory_Checkpoint_0402::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0402(env, context, options, stats);
}

void checkpoint_0402(SingleSwitchProgramEnvironment& env,
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
                if (!go_to_galactic_warehouse(env.console, context)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "go_to_galactic_warehouse: transition not detected.",
                        env.console);
                }
            }
            {
                if (!handle_battle(env.console, context, "veilstone_galactic_grunts_1", stats)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "handle_battle_veilstone_galactic_grunts_1: transition not detected.",
                        env.console);
                }
            }
            {
                if (!enter_galactic_warehouse(env.console, context)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "enter_galactic_warehouse: transition not detected.",
                        env.console);
                }
            }
            {
                if (!exit_galactic_warehouse(env.console, context)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "exit_galactic_warehouse: transition not detected.",
                        env.console);
                }
            }
            context.wait_for_all_requests();
        },
        true);
}

std::string AutoStory_Checkpoint_0403::name() const {
    return "04.03 - Route 214 and Valor Lakefront";
}

std::string AutoStory_Checkpoint_0403::start_text() const {
    return "Outside Galactic Warehouse with Fly obtained and dialogue cleared.";
}

std::string AutoStory_Checkpoint_0403::end_text() const {
    return "Outside the Valor Lakefront south gate on Route 213.";
}

void AutoStory_Checkpoint_0403::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0403(env, context, options, stats);
}

void checkpoint_0403(SingleSwitchProgramEnvironment& env,
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
                if (!go_to_route_214(env.console, context)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "go_to_route_214: transition not detected.",
                        env.console);
                }
            }
            {
                if (!handle_battle(env.console, context, "route_214_psychic_abigail", stats)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "handle_battle_route_214_psychic_abigail: transition not detected.",
                        env.console);
                }
            }
            {
                if (!walk_through_route_214(env.console, context, stats)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "walk_through_route_214: transition not detected.",
                        env.console);
                }
            }
            {
                if (!pass_by_valor_lakefront(env.console, context)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "pass_by_valor_lakefront: transition not detected.",
                        env.console);
                }
            }
            {
                if (!go_through_building_route_213(env.console, context)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "go_through_building_route_213: transition not detected.",
                        env.console);
                }
            }
            context.wait_for_all_requests();
        },
        true);
}

std::string AutoStory_Checkpoint_0404::name() const {
    return "04.04 - Route 213 to Pastoria";
}

std::string AutoStory_Checkpoint_0404::start_text() const {
    return "Outside the Valor Lakefront south gate on Route 213.";
}

std::string AutoStory_Checkpoint_0404::end_text() const {
    return "Healed and outside Pastoria Pokemon Center at the doorway.";
}

void AutoStory_Checkpoint_0404::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0404(env, context, options, stats);
}

void checkpoint_0404(SingleSwitchProgramEnvironment& env,
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
                if (!go_through_route_213(env.console, context, stats)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "go_through_route_213: transition not detected.",
                        env.console);
                }
            }
            {
                if (!go_through_house_east_pastoria(env.console, context)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "go_through_house_east_pastoria: transition not detected.",
                        env.console);
                }
            }
            {
                if (!go_through_pastoria(env.console, context)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "go_through_pastoria: transition not detected.",
                        env.console);
                }
            }
            {
                if (!heal_at_pokemon_center(env.console, context, stats)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "heal_at_pokemon_center: transition not detected.",
                        env.console);
                }
            }
            context.wait_for_all_requests();
        },
        true);
}

std::string AutoStory_Checkpoint_0405::name() const {
    return "04.05 - Pastoria Gym and Wake";
}

std::string AutoStory_Checkpoint_0405::start_text() const {
    return "Healed and outside Pastoria Pokemon Center at the doorway.";
}

std::string AutoStory_Checkpoint_0405::end_text() const {
    return "Outside Pastoria Gym at the doorway, facing south, with fourth-badge rewards and dialogue finished.";
}

void AutoStory_Checkpoint_0405::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0405(env, context, options, stats);
}

void checkpoint_0405(SingleSwitchProgramEnvironment& env,
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
                if (!go_to_pastoria_gym(env.console, context)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "go_to_pastoria_gym: transition not detected.",
                        env.console);
                }
            }
            pastoria_gym_puzzle_and_wake(env, context, stats);
            context.wait_for_all_requests();
        },
        true);
}

}
}
}