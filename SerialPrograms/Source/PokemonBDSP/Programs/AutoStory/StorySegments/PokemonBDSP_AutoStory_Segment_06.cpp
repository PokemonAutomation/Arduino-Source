/*  BDSP AutoStory - Segment 06
 *  From after acquiring the fifth badge until after acquiring the sixth badge.
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

#include "PokemonBDSP_AutoStory_Segment_06.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonBDSP {

// ---------------------------------------------------------------------------
//  Helpers
// ---------------------------------------------------------------------------

static void
move_back_after_unexpected_battle(VideoStream& stream, ProControllerContext& context, const std::string& trainerid){
    stream.log("Finishing battle with " + trainerid, COLOR_GREEN);
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    if (trainerid == "route_218_sailor_skyler"){
        pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms);  // 1+
        pbf_move_left_joystick(context, {-1, 0}, 400ms, 100ms);  // 1+
        pbf_move_left_joystick(context, {0, -1}, 2800ms, 100ms); // 14+
    }
}

// ---------------------------------------------------------------------------
//  Named route navigation blocks
// ---------------------------------------------------------------------------

static bool enter_jubilife_pokemon_center(VideoStream& stream, ProControllerContext& context){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("enter_jubilife_pokemon_center: entering...");

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("enter_jubilife_pokemon_center: black screen not detected!", COLOR_RED);
            return false;
        }
    };
    return true;
}

static bool heal_and_exit_jubilife_pokemon_center(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("heal_and_exit_jubilife_pokemon_center: healing and exiting...");
    if (!heal_pokemon(stream, context, "Jubilife city", &stats)){
        return false;
    }
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    return true;
}

static bool move_to_jubilife_west_exit(VideoStream& stream, ProControllerContext& context){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("move_to_jubilife_west_exit: moving towards building...");
    pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms);  // 6
    pbf_move_left_joystick(context, {0, +1}, 2600ms, 100ms); // 18

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 100000ms, 100ms); // 46
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("move_to_jubilife_west_exit: transition not detected!", COLOR_RED);
            return false;
        }else if (ret == 0){
            stream.log("move_to_jubilife_west_exit: transition detected.", COLOR_GREEN);
        }
    }

    return true;
}

static bool go_through_jubilife_west_exit(VideoStream& stream, ProControllerContext& context){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_through_jubilife_west_exit: going through the building...");

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 100000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_jubilife_west_exit: transition not detected!", COLOR_RED);
            return false;
        }else if (ret == 0){
            stream.log("go_through_jubilife_west_exit: transition detected.", COLOR_GREEN);
        }
    }

    return true;
}

static bool go_through_route_218(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats){
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

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms); // 2+
                pbf_move_left_joystick(context, {-1, 0}, 6000ms,
                    100ms);                                              // 28++ (getting out of the water)
                pbf_move_left_joystick(context, {0, -1}, 2800ms, 100ms); // 14+
            },
            {{sailor_skyler_fight}});
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
                stream.log("go_through_route_218_sailor_skyler: black screen not detected!", COLOR_RED);
                return false;
            }else if (ret == 0){
                fight_trainer_using_moves(stream, context, stats, "route_218_sailor_skyler", true);
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
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 100000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_route_218: transition not detected!", COLOR_RED);
            return false;
        }else if (ret == 0){
            stream.log("go_through_route_218: transition detected.", COLOR_GREEN);
        }
    }
    return true;
}

static bool go_through_building_route_218(VideoStream& stream, ProControllerContext& context){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_through_building_route_218: going through the building...");

    {
        ShortDialogWatcher talk_to_scientist(COLOR_BLUE);
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 100000ms, 100ms);
            },
            {{talk_to_scientist}});
        if (ret < 0){
            stream.log("go_through_building_route_218_scientist: dialog box not detected!", COLOR_RED);
            return false;
        }else if (ret == 0){
            stream.log("go_through_building_route_218_scientist: dialog box detected.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 12000ms);
            context.wait_for_all_requests();
        }
    }

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 100000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_building_route_218: transition not detected!", COLOR_RED);
            return false;
        }else if (ret == 0){
            stream.log("go_through_building_route_218: transition detected.", COLOR_GREEN);
        }
    }
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    return true;
}

static bool go_to_clint_and_pokemon_center(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_to_clint_and_pokemon_center: going to the Pokemon Center...");

    pbf_move_left_joystick(context, {-1, 0}, 1400ms, 100ms); // 7+
    pbf_move_left_joystick(context, {0, +1}, 1000ms, 100ms); // 5+
    pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms);  // 4+
    pbf_move_left_joystick(context, {0, +1}, 4200ms, 100ms); // 21+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 4);

    {
        ShortDialogWatcher repel_dialog(COLOR_RED);
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 2000ms, 100ms); // 10+
            },
            {{repel_dialog}});
        if (ret == 0){
            stream.log("go_to_clint_and_pokemon_center: Not activating repel again.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_B, 3000ms);
        }else if (ret < 0){
            stream.log("go_to_clint_and_pokemon_center: Didn't detect repel dialog box!", COLOR_RED);
            return false;
        }
    }

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {+1, 0}, 2000ms, 300ms); // 10+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 8);

    {
        ShortDialogWatcher talk_to_clint(COLOR_BLUE);
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 100000ms, 100ms);
            },
            {{talk_to_clint}});
        if (ret < 0){
            stream.log("go_to_clint_and_pokemon_center_clint: dialog box not detected!", COLOR_RED);
            return false;
        }else if (ret == 0){
            stream.log("go_to_clint_and_pokemon_center_clint: dialog box detected, starting battle.", COLOR_GREEN);
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
                stream.log("go_to_clint_and_pokemon_center_clint: black screen not detected!", COLOR_RED);
                return false;
            }else if (ret == 0){
                fight_trainer_using_moves(stream, context, stats, "canalave_clint", true);
                context.wait_for_all_requests();
                pbf_wait(context, 3000ms);
            }
        }
    }

    context.wait_for_all_requests();
    pbf_mash_button(context, BUTTON_B, 3000ms);
    pbf_wait(context, 1000ms);
    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {+1, 0}, 3000ms, 300ms); // 15+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 3);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_to_clint_and_pokemon_center: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_to_clint_and_pokemon_center: entered pokemon center", COLOR_GREEN);
    };
    return true;
}

static bool heal_and_exit_canalave_pokemon_center(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("heal_and_exit_canalave_pokemon_center: healing and exiting...");
    if (!heal_pokemon(stream, context, "Canalave city", &stats)){
        return false;
    }
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    return true;
}

static bool go_to_canalave_gym(VideoStream& stream, ProControllerContext& context){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_to_canalave_gym: going for the gym...");

    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 2, false);

    pbf_move_left_joystick(context, {-1, 0}, 5000ms, 100ms); // 26+
    pbf_move_left_joystick(context, {0, -1}, 1400ms, 100ms); // 9
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 7);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_to_canalave_gym: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_to_canalave_gym: entered gym", COLOR_GREEN);
    };
    context.wait_for_all_requests();
    // Let the entrance scene finish after the watched black-screen transition.
    pbf_wait(context, 5000ms);
    context.wait_for_all_requests();
    return true;
}

// Start inside Canalave Gym at the entrance, after the entry transition. End at Byron with
// battle/reward dialogue finished. The checkpoint then returns to the inside entrance.
static void canalave_gym_puzzle_and_byron(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryStats& stats){
    VideoStream& stream = env.console;
    struct Move {
        DpadPosition dir;
        int count;
        Milliseconds delay;
    };

    DpadState dpad;
    dpad.last_dir = DPAD_UP; // The Gym entry approach ends facing north.

    // Add a facing press on direction changes and synchronize NPC turns after battles. Pause for
    // at least one second after each platform/approach press.
    const std::vector<Move> sequence1 = {
        {DPAD_UP, 9, 0ms},
        {DPAD_LEFT, 7, 0ms},
        {DPAD_LEFT, 1, 1230ms},
        {DPAD_UP, 2, 0ms},
        {DPAD_UP, 1, 2011ms},
    };

    const std::vector<Move> sequence2 = {
        {DPAD_RIGHT, 1, 3644ms},
        {DPAD_UP, 1, 1611ms},
        {DPAD_UP, 1, 1339ms},
        {DPAD_RIGHT, 1, 2289ms},
        {DPAD_RIGHT, 7, 0ms},
        {DPAD_DOWN, 3, 0ms},
        {DPAD_RIGHT, 2, 0ms},
        {DPAD_RIGHT, 1, 2826ms},
        {DPAD_UP, 3, 0ms},
        {DPAD_UP, 1, 1989ms},
        {DPAD_RIGHT, 3, 0ms},
        {DPAD_DOWN, 6, 0ms},
        // Center the southbound approach on the lift instead of one tile east.
        {DPAD_RIGHT, 4, 0ms},
        {DPAD_DOWN, 2, 0ms},
        {DPAD_DOWN, 1, 2189ms},
        {DPAD_UP, 9, 0ms},
        {DPAD_LEFT, 1, 2182ms},
        {DPAD_RIGHT, 5, 0ms},
        {DPAD_DOWN, 8, 0ms},
        {DPAD_DOWN, 1, 1883ms},
        {DPAD_DOWN, 4, 0ms},
        {DPAD_LEFT, 1, 0ms},
        {DPAD_LEFT, 1, 1192ms},
    };

    const std::vector<Move> sequence3 = {
        {DPAD_LEFT, 6, 0ms},
        {DPAD_UP, 3, 0ms},
        {DPAD_UP, 1, 2106ms},
        {DPAD_LEFT, 2, 0ms},
        {DPAD_LEFT, 1, 1822ms},
        {DPAD_LEFT, 3, 0ms},
        {DPAD_UP, 2, 0ms},
        {DPAD_LEFT, 9, 0ms},
        {DPAD_DOWN, 6, 0ms},
        {DPAD_RIGHT, 3, 0ms},
        {DPAD_RIGHT, 1, 2346ms},
        {DPAD_LEFT, 3, 0ms},
        {DPAD_UP, 3, 0ms},
        {DPAD_UP, 1, 1593ms},
        {DPAD_UP, 6, 0ms},
        {DPAD_RIGHT, 2, 0ms},
        {DPAD_UP, 8, 0ms},
        // The third eastward step triggers the Worker before the horizontal lift.
        {DPAD_RIGHT, 3, 0ms},
    };

    const std::vector<Move> sequence4 = {
        // Resume the nine eastward steps after the Worker's battle. Preserve the platform pauses
        // and wait for dialogue to finish before moving.
        {DPAD_RIGHT, 3, 1696ms},
        {DPAD_RIGHT, 3, 0ms},
        {DPAD_RIGHT, 1, 1806ms},
        {DPAD_RIGHT, 1, 0ms},
        {DPAD_RIGHT, 1, 1855ms},
    };

    const std::vector<Move> sequence5 = {
        // David interrupts before this eastward step. Resume toward the stairs after battle.
        {DPAD_RIGHT, 1, 0ms},
        {DPAD_DOWN, 11, 0ms},
        {DPAD_DOWN, 1, 1825ms},
        {DPAD_DOWN, 2, 0ms},
        {DPAD_RIGHT, 3, 0ms},
        {DPAD_UP, 2, 0ms},
        {DPAD_UP, 1, 1773ms},
        {DPAD_UP, 5, 0ms},
        {DPAD_UP, 1, 2187ms},
        {DPAD_UP, 3, 0ms},
        // Reach Breanna's east-facing sight line after the descending lift.
        {DPAD_UP, 2, 1434ms},
    };

    const std::vector<Move> sequence6 = {
        {DPAD_UP, 1, 0ms},
        {DPAD_LEFT, 8, 0ms},
        {DPAD_DOWN, 5, 0ms},
        {DPAD_RIGHT, 1, 0ms},
        {DPAD_RIGHT, 1, 1965ms},
        {DPAD_LEFT, 8, 0ms},
        {DPAD_LEFT, 1, 5406ms},
        {DPAD_UP, 5, 0ms},
        {DPAD_UP, 1, 1071ms},
    };

    const std::vector<Move>* sequences[] = {
        &sequence1,
        &sequence2,
        &sequence3,
        &sequence4,
        &sequence5,
        &sequence6,
    };

    for (size_t stage = 0; stage < 6; stage++){
        const bool gym_leader = stage == 5;
        const std::string label =
            gym_leader ? "Byron" : "Canalave Gym trainer " + std::to_string(stage + 1);
        stream.log("[Canalave Gym puzzle] Approaching " + label + ".");
        for (const auto& move : *sequences[stage]){
            repeat_dpad(context, dpad, move.dir, 80ms, 300ms, move.count, true);
            if (move.delay > 0ms){
                pbf_wait(context, move.delay);
                context.wait_for_all_requests();
            }
        }
        context.wait_for_all_requests();
        if (stage == 2 || stage == 3 || stage == 4){
            wait_for_dialog(stream, context, label, 10000ms);
        }
        pbf_press_button(context, BUTTON_A, 150ms, 300ms);
        fight_trainer_using_moves(stream, context, stats, label, gym_leader);
        clear_dialog(stream, context, BUTTON_B, gym_leader ? 6000ms : 3000ms);
        if (stage == 1 || stage == 2 || stage == 3){
            // Cesar, the Worker, and David turn the player north after approaching. Synchronize
            // facing before resuming west/east movement.
            dpad.last_dir = DPAD_UP;
        }else if (stage == 4){
            // Breanna approaches from the west and turns the player toward her.
            dpad.last_dir = DPAD_LEFT;
        }
    }
    context.wait_for_all_requests();
    stream.log("[Canalave Gym puzzle] Byron defeated; reward dialogue cleared.", COLOR_GREEN);
}

// Start: beside Byron after his reward dialogue, facing north.
// End: inside Canalave Gym at the entrance, facing east; 07.01 owns the doorway exit.
static void return_to_canalave_gym_entrance(VideoStream& stream, ProControllerContext& context){
    struct Move {
        DpadPosition dir;
        int count;
        Milliseconds delay;
    };

    stream.log("[Canalave Gym return] Returning from Byron to the inside entrance.");
    // Preserve the opening input and settling pause.
    pbf_move_right_joystick(context, {0, 1}, 146ms, 0ms);
    pbf_wait(context, 3617ms);
    context.wait_for_all_requests();

    DpadState dpad;
    dpad.last_dir = DPAD_UP;
    const std::vector<Move> sequence = {
        {DPAD_UP, 1, 0ms},
        {DPAD_RIGHT, 3, 2655ms},
        {DPAD_RIGHT, 2, 0ms},
        {DPAD_DOWN, 3, 2787ms},
        {DPAD_DOWN, 2, 0ms},
        {DPAD_LEFT, 3, 2632ms},
        {DPAD_LEFT, 2, 0ms},
        {DPAD_UP, 2, 1189ms},
        {DPAD_UP, 1, 2467ms},
        {DPAD_UP, 2, 0ms},
        {DPAD_RIGHT, 5, 6081ms},
        {DPAD_DOWN, 19, 0ms},
        {DPAD_RIGHT, 9, 0ms},
    };

    for (const auto& move : sequence){
        repeat_dpad(context, dpad, move.dir, 80ms, 300ms, move.count, true);
        if (move.delay > 0ms){
            pbf_wait(context, move.delay);
            context.wait_for_all_requests();
        }
    }
    context.wait_for_all_requests();
    stream.log("[Canalave Gym return] Recorded return to the inside entrance finished.", COLOR_GREEN);
}

// ---------------------------------------------------------------------------
//  Segment wrappers
// ---------------------------------------------------------------------------

std::string AutoStory_Segment_06::name() const {
    return "06: Hearthome to Canalave";
}

std::string AutoStory_Segment_06::start_text() const {
    return AutoStory_Checkpoint_0601().start_text();
}

std::string AutoStory_Segment_06::end_text() const {
    return AutoStory_Checkpoint_0604().end_text();
}

void AutoStory_Segment_06::run_segment(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_0601().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0602().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0603().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0604().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}

// ---------------------------------------------------------------------------
//  Checkpoint wrappers
// ---------------------------------------------------------------------------

std::string AutoStory_Checkpoint_0601::name() const {
    return "06.01 - Hearthome to Jubilife";
}

std::string AutoStory_Checkpoint_0601::start_text() const {
    return "Outside Hearthome Gym at the post-Cynthia position, with her dialogue finished and the fifth badge acquired.";
}

std::string AutoStory_Checkpoint_0601::end_text() const {
    return "Healed and outside Jubilife City Pokemon Center at the doorway, facing south.";
}

void AutoStory_Checkpoint_0601::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0601(env, context, options, stats);
}

void checkpoint_0601(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            if (!fly_to(env.console, context, FlyPoint::JubilifeCity)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "06.01: Flight to Jubilife did not complete.",
                    env.console);
            }
            if (!enter_jubilife_pokemon_center(env.console, context)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "06.01: Jubilife Center entry did not complete.",
                    env.console);
            }
            if (!heal_and_exit_jubilife_pokemon_center(env.console, context, stats)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "06.01: Jubilife healing and exit did not complete.",
                    env.console);
            }
        },
        true);
}

std::string AutoStory_Checkpoint_0602::name() const {
    return "06.02 - Jubilife to Canalave";
}

std::string AutoStory_Checkpoint_0602::start_text() const {
    return "Outside Jubilife City Pokemon Center at the doorway, facing south.";
}

std::string AutoStory_Checkpoint_0602::end_text() const {
    return "Outside Canalave City east gate, facing west, after the scientist conversation.";
}

void AutoStory_Checkpoint_0602::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0602(env, context, options, stats);
}

void checkpoint_0602(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            if (!move_to_jubilife_west_exit(env.console, context)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "06.02: Jubilife west gate entry did not complete.",
                    env.console);
            }
            if (!go_through_jubilife_west_exit(env.console, context)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "06.02: Jubilife west gate exit did not complete.",
                    env.console);
            }
            if (!go_through_route_218(env.console, context, stats)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "06.02: Route 218 crossing did not complete.",
                    env.console);
            }
            if (!go_through_building_route_218(env.console, context)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "06.02: Canalave gate crossing and scientist conversation did not complete.",
                    env.console);
            }
        },
        true);
}

std::string AutoStory_Checkpoint_0603::name() const {
    return "06.03 - Canalave rival battle and healing";
}

std::string AutoStory_Checkpoint_0603::start_text() const {
    return "Outside Canalave City east gate, facing west, after the scientist conversation.";
}

std::string AutoStory_Checkpoint_0603::end_text() const {
    return "Rival defeated; healed and outside Canalave City Pokemon Center at the doorway, facing south.";
}

void AutoStory_Checkpoint_0603::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0603(env, context, options, stats);
}

void checkpoint_0603(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            if (!go_to_clint_and_pokemon_center(env.console, context, stats)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "06.03: Canalave rival battle and Center entry did not complete.",
                    env.console);
            }
            if (!heal_and_exit_canalave_pokemon_center(env.console, context, stats)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "06.03: Canalave healing and exit did not complete.",
                    env.console);
            }
        },
        true);
}

std::string AutoStory_Checkpoint_0604::name() const {
    return "06.04 - Canalave Gym and Byron";
}

std::string AutoStory_Checkpoint_0604::start_text() const {
    return "Outside Canalave City Pokemon Center at the doorway, facing south, with full HP and PP.";
}

std::string AutoStory_Checkpoint_0604::end_text() const {
    return "Inside Canalave Gym at the entrance, facing east, after defeating Byron and acquiring the Mine Badge, with reward dialogue finished.";
}

void AutoStory_Checkpoint_0604::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0604(env, context, options, stats);
}

void checkpoint_0604(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            if (!go_to_canalave_gym(env.console, context)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "06.04: Canalave Gym entry did not complete.",
                    env.console);
            }
            canalave_gym_puzzle_and_byron(env, context, stats);
            return_to_canalave_gym_entrance(env.console, context);
        },
        true);
}

}
}
}
