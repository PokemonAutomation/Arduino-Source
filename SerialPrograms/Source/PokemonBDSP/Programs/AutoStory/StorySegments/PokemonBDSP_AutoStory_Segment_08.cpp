/*  BDSP AutoStory - Segment 08
 *  From after acquiring the seventh badge until after acquiring the eighth badge.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <vector>

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

#include "PokemonBDSP/PokemonBDSP_Settings.h"

#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_StartBattleDetector.h"
#include "PokemonBDSP/Inference/BoxSystem/PokemonBDSP_BoxDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_MarkFinder.h"
#include "PokemonBDSP/Inference/PokemonBDSP_TransitionWatchers.h"
#include "PokemonBDSP/Programs/PokemonBDSP_BasicCatcher.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameNavigation.h"
#include "PokemonBDSP/Programs/PokemonBDSP_RunFromBattle.h"
#include "PokemonBDSP/Programs/AutoStory/Detect/PokemonBDSP_AutoStory_OverworldDetector.h"
#include "PokemonBDSP/Programs/AutoStory/PokemonBDSP_AutoStory.h"
#include "PokemonBDSP/Programs/AutoStory/PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP/Programs/AutoStory/Utils/PokemonBDSP_AutoStory_Battle.h"
#include "PokemonBDSP/Programs/AutoStory/Utils/PokemonBDSP_OverworldMatchers.h"
#include "PokemonBDSP/Programs/AutoStory/Utils/PokemonBDSP_AutoStory_Segment_00_tools.h"
#include "PokemonBDSP/Programs/AutoStory/Utils/PokemonBDSP_Autostory_Segment_01_tools.h"

#include "PokemonBDSP_AutoStory_Segment_02.h"
#include "PokemonBDSP_AutoStory_Segment_08.h"

using namespace std::chrono_literals;

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

    if (trainerid == "veilstone_warehouse_warehouse_grunt_1"){
        pbf_move_left_joystick(context, {0, +1}, 1200ms, 100ms); // 5+
    }else if (trainerid == "galactic_hq_grunt_1"){
        pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms); // 3+
    }else if (trainerid == "route_207_hiker_justin"){
        pbf_move_left_joystick(context, {+1, 0}, 800ms, 100ms); // 3+
    }
}

static bool heal_and_exit(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats, const std::string& label){
    return heal_pokemon(stream, context, label, &stats);
}

// ---------------------------------------------------------------------------
//  Named route navigation blocks
// ---------------------------------------------------------------------------


static bool heal_at_snowpoint_city_pokemon_center(VideoStream& stream, ProControllerContext& context){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("heal_at_snowpoint_city_pokemon_center: flying towards the pokemon center...");

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
            stream.log("heal_at_snowpoint_city_pokemon_center: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("heal_at_snowpoint_city_pokemon_center: entered Pokemon Center.", COLOR_GREEN);
    };
    return true;
}

static bool go_to_lake_acuity(VideoStream& stream, ProControllerContext& context){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_to_lake_acuity: travelling from Snowpoint to Lake Acuity...");

    pbf_move_left_joystick(context, {-1, 0}, 1000ms, 100ms); // 5+
    pbf_move_left_joystick(context, {0, +1}, 1600ms, 100ms); // 8+

    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 8);
    pbf_move_left_joystick(context, {0, -1}, 4000ms, 100ms); // 20+
    pbf_move_left_joystick(context, {-1, 0}, 1400ms, 100ms); // 7+
    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 1);
    pbf_move_left_joystick(context, {-1, 0}, 1400ms, 100ms); // 7+
    pbf_move_left_joystick(context, {0, +1}, 1600ms, 100ms); // 8+
    pbf_move_left_joystick(context, {-1, 0}, 1800ms, 100ms); // 9+
    pbf_move_left_joystick(context, {0, -1}, 1600ms, 100ms); // 8+

    context.wait_for_all_requests();
    if (!activate_repel(stream, context, 8, 1)){
        stream.log("go_to_lake_acuity: Could not activate a Super Repel.", COLOR_RED);
        return false;
    }

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {-1, 0}, 2400ms, 100ms); // 12+
    pbf_move_left_joystick(context, {0, -1}, 600ms, 100ms);  // 2+
    pbf_move_left_joystick(context, {-1, 0}, 8400ms, 100ms); // 28++ (snow)
    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 6);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                // With the preceding 3s approach, 8.4s west hold, and seven
                // D-pad pulses, cap total protected movement at 21.06s.
                pbf_move_left_joystick(context, {0, +1}, 7000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_to_lake_acuity: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_to_lake_acuity: entered Lake Acuity.", COLOR_GREEN);
    };
    return true;
}

static bool talk_to_clint_then_go_veilstone(VideoStream& stream, ProControllerContext& context){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("talk_to_clint_then_go_veilstone: first talking to Clint and then flying to Veilstone city...");

    {
        ShortDialogWatcher listen_to_clint_and_jupiter(COLOR_BLUE);
        int ret = wait_until(stream, context, std::chrono::seconds(30), {{listen_to_clint_and_jupiter}});
        if (ret < 0){
            stream.log("talk_to_clint_then_go_veilstone_listen_to_clint_and_jupiter: dialog box not detected!",
                COLOR_RED);
            return false;
        }else if (ret == 0){
            stream.log("talk_to_clint_then_go_veilstone_listen_to_clint_and_jupiter: dialog box detected.",
                COLOR_GREEN);
            pbf_mash_button(context, BUTTON_B, 22000ms);
            context.wait_for_all_requests();
        }
    }

    // TODO(live verification): verify west/east clearance after the Lake Acuity dialogue; Fly normalizes the return position.
    // Planned before the next leg exceeds the 22s Super Repel movement budget.
    force_repel_expiry(stream, context, RepelRenewalChoice::NO, false, true);

    if (!fly_to(stream, context, FlyPoint::VeilstoneCity)){
        stream.log("Flight to Veilstone failed; stopping before the next route block.", COLOR_RED);
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
            stream.log("talk_to_clint_then_go_veilstone: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("talk_to_clint_then_go_veilstone: entered Veilstone City Pokemon Center.", COLOR_GREEN);
    };
    return true;
}

static bool get_key_and_enter_warehouse(VideoStream& stream, ProControllerContext& context){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("get_key_and_enter_warehouse: Getting the warehouse key then entering the warehouse...");

    pbf_move_left_joystick(context, {0, -1}, 600ms, 100ms); // 2+
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 6);
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 4);
    pbf_move_left_joystick(context, {+1, 0}, 3200ms, 100ms); // 16+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 3);

    // Lake Acuity cleared Repel before the Veilstone flight.
    // Do not require another expiry during the warehouse approach.


    pbf_move_left_joystick(context, {0, +1}, 4600ms, 100ms); // 23+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 3);

    {
        ShortDialogWatcher talk_to_grunt(COLOR_RED);
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {0, +1}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{talk_to_grunt}});
        if (ret == 0){
            stream.log("get_key_and_enter_warehouse: Talking to the grunt.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_B, 9000ms);
        }else if (ret < 0){
            stream.log("get_key_and_enter_warehouse: Didn't detect grunt dialog box!", COLOR_RED);
            return false;
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    {
        ShortDialogWatcher pick_up_key(COLOR_RED);
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {0, +1}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{pick_up_key}});
        if (ret == 0){
            stream.log("get_key_and_enter_warehouse: Got the key.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 5000ms);
        }else if (ret < 0){
            stream.log("get_key_and_enter_warehouse: Didn't detect key pickup dialog box!", COLOR_RED);
            return false;
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 2);

    pbf_move_left_joystick(context, {0, -1}, 3200ms, 100ms); // 23
    pbf_move_left_joystick(context, {-1, 0}, 2700ms, 100ms); // 21
    pbf_move_left_joystick(context, {0, +1}, 1800ms, 100ms); // 9+
    pbf_move_left_joystick(context, {-1, 0}, 2600ms, 100ms); // 13+
    pbf_move_left_joystick(context, {0, +1}, 2600ms, 100ms); // 13+
    pbf_move_left_joystick(context, {+1, 0}, 2600ms, 100ms); // 12+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("get_key_and_enter_warehouse: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("get_key_and_enter_warehouse: entered warehouse.", COLOR_GREEN);
    };
    return true;
}

static bool go_through_warehouse(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_through_warehouse: Going through the warehouse...");

    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 1);

    {
        ShortDialogWatcher open_door(COLOR_RED);
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {0, +1}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{open_door}});
        if (ret == 0){
            stream.log("go_through_warehouse: Opened the door.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 4000ms);
        }else if (ret < 0){
            stream.log("go_through_warehouse: Didn't detect door dialog box!", COLOR_RED);
            return false;
        }
    }

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, +1}, 1000ms, 100ms); // 4+

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_warehouse: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_warehouse: got into the warehouse's B2F.", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {+1, 0}, 7400ms, 100ms); // 37+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);

    {
        MarkDetector warehouse_grunt_1_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 1200ms, 100ms);  // 5+
                pbf_move_left_joystick(context, {0, -1}, 400ms, 100ms);   // 2+
                pbf_move_left_joystick(context, {+1, 0}, 10800ms, 100ms); // 54+
            },
            {{warehouse_grunt_1_fight}});
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
                stream.log("veilstone_warehouse_warehouse_grunt_1: black screen not detected!", COLOR_RED);
                return false;
            }else if (ret == 0){
                fight_trainer_using_moves(stream, context, stats, "veilstone_warehouse_warehouse_grunt_1", true);
                context.wait_for_all_requests();
                pbf_wait(context, 3000ms);
                move_back_after_unexpected_battle(stream, context, "veilstone_warehouse_warehouse_grunt_1");
            }
        }
    }

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, -1}, 200ms, 100ms); // 1


    {
        MarkDetector warehouse_grunt_2_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 13000ms, 100ms); // 54+
            },
            {{warehouse_grunt_2_fight}});
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
                stream.log("veilstone_warehouse_warehouse_grunt_2: black screen not detected!", COLOR_RED);
                return false;
            }else if (ret == 0){
                fight_trainer_using_moves(stream, context, stats, "veilstone_warehouse_warehouse_grunt_2", true);
                context.wait_for_all_requests();
                pbf_wait(context, 3000ms);
                move_back_after_unexpected_battle(stream, context, "veilstone_warehouse_warehouse_grunt_2");
            }
        }
    }

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {+1, 0}, 1000ms, 100ms); // 4+
    pbf_move_left_joystick(context, {0, +1}, 1600ms, 100ms); // 8+

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_warehouse: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_warehouse: got into the warehouse's B1F.", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 1);
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 5);
    pbf_move_left_joystick(context, {0, +1}, 1200ms, 100ms); // 5+

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_warehouse: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_warehouse: got into the Galactic HQ 1F, first room.", COLOR_GREEN);
    };

    pbf_move_left_joystick(context, {+1, 0}, 1400ms, 100ms); // 6+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 2);
    pbf_move_left_joystick(context, {0, -1}, 1400ms, 100ms); // 6+

    {
        BlackScreenFlashWatcher black_screen(COLOR_BLUE, {0.1, 0.1, 0.8, 0.8});
        MarkDetector galactic_hq_grunt_1_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 2200ms, 100ms); // 11+
                pbf_move_left_joystick(context, {0, +1}, 1200ms, 100ms); // 6
            },
            {{black_screen}, {galactic_hq_grunt_1_fight}});
        if (ret < 0){
            stream.log("go_through_warehouse: Neither the room transition nor the grunt was detected.", COLOR_RED);
            return false;
        }
        if (ret == 0){
            stream.log("go_through_warehouse: got into Galactic HQ 1F, second room.", COLOR_GREEN);
        }else if (ret == 1){
            context.wait_for_all_requests();
            pbf_wait(context, 2000ms);

            BlackScreenOverWatcher black_screen_2(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            ret = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 200000ms);
                },
                {{black_screen_2}});
            if (ret < 0){
                stream.log("galactic_hq_grunt_1: black screen not detected!", COLOR_RED);
                return false;
            }else if (ret == 0){
                fight_trainer_using_moves(stream, context, stats, "galactic_hq_grunt_1", true);
                context.wait_for_all_requests();
                pbf_wait(context, 3000ms);
                move_back_after_unexpected_battle(stream, context, "galactic_hq_grunt_1");
            }

            context.wait_for_all_requests();
            pbf_wait(context, 1000ms);

            {
                BlackScreenFlashWatcher room_transition(COLOR_BLUE, {0.1, 0.1, 0.8, 0.8});
                int ret1 = run_until<ProControllerContext>(stream,
                    context,
                    [](ProControllerContext& context){
                        pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
                    },
                    {{room_transition}});
                if (ret1 < 0){
                    stream.log("go_through_warehouse: black screen not detected!", COLOR_RED);
                    return false;
                }
                stream.log("go_through_warehouse: got into Galactic HQ 1F, second room.", COLOR_GREEN);
            };
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {+1, 0}, 1000ms, 100ms); // 4+

    {
        BlackScreenFlashWatcher black_screen(COLOR_BLUE, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, -1}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_warehouse: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_warehouse: got into the Galactic HQ 1F, third room.", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {0, +1}, 1200ms, 100ms); // 5+

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_warehouse: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_warehouse: got into the Galactic HQ 2F, first room.", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {0, -1}, 800ms, 100ms);  // 3+
    pbf_move_left_joystick(context, {-1, 0}, 1400ms, 100ms); // 6+

    {
        BlackScreenFlashWatcher black_screen(COLOR_BLUE, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_warehouse: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_warehouse: got into the Galactic HQ 2F, second room.", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {+1, 0}, 400ms, 100ms); // 2
    pbf_move_left_joystick(context, {0, +1}, 600ms, 100ms); // 2+

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_warehouse: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_warehouse: got into the Galactic HQ 1F, fourth room.", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 4);

    {
        MarkDetector galactic_hq_scientist_fredrick_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 1000ms, 100ms);  // 6
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms); // 6+
            },
            {{galactic_hq_scientist_fredrick_fight}});
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
                stream.log("galactic_hq_scientist_fredrick: black screen not detected!", COLOR_RED);
                return false;
            }else if (ret == 0){
                fight_trainer_using_moves(stream, context, stats, "galactic_hq_scientist_fredrick", true);
            }
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    pbf_move_left_joystick(context, {0, +1}, 800ms, 100ms); // 3+

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_warehouse: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_warehouse: got into the Galactic Warehouse B2F.", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {-1, 0}, 4600ms, 100ms); // 23+
    pbf_move_left_joystick(context, {0, -1}, 1600ms, 100ms); // 8+

    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 7);

    {
        ShortDialogWatcher pick_up_galactic_key(COLOR_RED);
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {0, +1}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{pick_up_galactic_key}});
        if (ret == 0){
            stream.log("go_through_warehouse: Picked up the Galactic key.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_B, 3000ms);
        }else if (ret < 0){
            stream.log("go_through_warehouse: Didn't detect grunt dialog box!", COLOR_RED);
            return false;
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, -1}, 1200ms, 100ms); // 5+
    pbf_move_left_joystick(context, {-1, 0}, 1400ms, 100ms); // 6+

    {
        ShortDialogWatcher open_door(COLOR_RED);
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {0, +1}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{open_door}});
        if (ret == 0){
            stream.log("go_through_warehouse: Opened the first door with the Galactic Key.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 4000ms);
        }else if (ret < 0){
            stream.log("go_through_warehouse: Didn't detect door opening dialog box!", COLOR_RED);
            return false;
        }
    }

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, +1}, 1400ms, 100ms); // 6+
    pbf_move_left_joystick(context, {-1, 0}, 1800ms, 100ms); // 8+
    pbf_move_left_joystick(context, {0, -1}, 3200ms, 100ms); // 16+

    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 3);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_warehouse: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_warehouse: got into the Galactic Warehouse entrance area.", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {-1, 0}, 1200ms, 100ms); // 5+

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, -1}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_warehouse: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_warehouse: got out of the Galactic Warehouse.", COLOR_GREEN);
    };

    return true;
}

static bool heal_at_veilstone_pokemon_center(VideoStream& stream, ProControllerContext& context){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("heal_at_veilstone_pokemon_center: Flying to Veilstone Pokemon Center and entering...");

    if (!fly_to(stream, context, FlyPoint::CurrentCity)){
        stream.log("Flight to the current city failed; stopping before the next route block.", COLOR_RED);
        return false;
    }

    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("heal_at_veilstone_pokemon_center: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("heal_at_veilstone_pokemon_center: entered the Veilstone Pokemon Center.", COLOR_GREEN);
    };

    return true;
}

static bool go_to_galactic_hq(VideoStream& stream, ProControllerContext& context){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_to_galactic_hq: Going through the warehouse...");

    pbf_move_left_joystick(context, {0, -1}, 600ms, 100ms); // 2+
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 6);
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 4);
    pbf_move_left_joystick(context, {+1, 0}, 3200ms, 100ms); // 16+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 3);

    pbf_move_left_joystick(context, {0, +1}, 4600ms, 100ms); // 23+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 7);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_to_galactic_hq, entering the Galactic HQ from the front door: black screen not detected!",
                COLOR_RED);
            return false;
        }
        stream.log("go_to_galactic_hq: entered the Galactic HQ from the front door.", COLOR_GREEN);
    };

    return true;
}

static bool go_through_galactic_hq(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats, AutoStoryOptions /*options*/){
    // ------- This has returned one error and I couldn't reproduce it. It was somewhere at the other comment I left behind ----------- //

    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_through_galactic_hq: Going through the HQ again and battling Cyrus...");

    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 6);

    {
        ShortDialogWatcher open_door(COLOR_RED);
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {0, +1}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{open_door}});
        if (ret == 0){
            stream.log("go_through_galactic_hq: Opened the door.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 4000ms);
        }else if (ret < 0){
            stream.log("go_through_galactic_hq, opening the door: Didn't detect door dialog box!", COLOR_RED);
            return false;
        }
    }

    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {0, +1}, 1200ms, 100ms); // 5+

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_galactic_hq, entering Galactic HQ's 2F, first room: black screen not detected!",
                COLOR_RED);
            return false;
        }
        stream.log("go_through_galactic_hq: entered Galactic HQ's 2F, first room.", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {0, -1}, 1000ms, 100ms); // 4+


    {
        MarkDetector galactic_hq_grunt_2_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
        // Stop near the first grunt, then cross his sight line repeatedly.
        // Do not reach the east wall or the next grunt before this fight.

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                // The approach reaches this grunt's lane after about two seconds.
                pbf_move_left_joystick(context, {+1, 0}, 2000ms, 100ms);
                context.wait_for_all_requests();
                for (size_t pulse = 0; pulse < 20; pulse++){
                    pbf_move_left_joystick(context, {+1, 0}, 500ms, 100ms);
                    pbf_move_left_joystick(context, {-1, 0}, 500ms, 100ms);
                    context.wait_for_all_requests();
                }
            },
            {{galactic_hq_grunt_2_fight}});
        if (ret < 0){
            stream.log("galactic_hq_grunt_2: Required encounter not detected after the bounded approach.", COLOR_RED);
            return false;
        }else if (ret == 0){
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
                stream.log("galactic_hq_grunt_2: black screen not detected!", COLOR_RED);
                return false;
            }else if (ret == 0){
                fight_trainer_using_moves(stream, context, stats, "galactic_hq_grunt_2", true);
            }
        }
    }


    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    // Absorb the encounter wiggle offset at the east wall before the backward step.
    pbf_move_left_joystick(context, {+1, 0}, 3200ms, 100ms);
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);


    context.wait_for_all_requests();

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {0, -1}, 0ms, 10000ms);
                ssf_mash1_button(context, BUTTON_A, 10000ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("galactic_hq_grunt_3: black screen not detected!", COLOR_RED);
            return false;
        }else if (ret == 0){
            context.wait_for_all_requests();

            fight_trainer_using_moves(stream, context, stats, "galactic_hq_grunt_3", true);
            context.wait_for_all_requests();
            pbf_wait(context, 2000ms);
        }
    }


    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    pbf_move_left_joystick(context, {0, -1}, 1200ms, 100ms); // 5+
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 7);
    pbf_move_left_joystick(context, {0, +1}, 2600ms, 100ms); // 13+
    pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms);  // 3+

    {
        BlackScreenFlashWatcher black_screen(COLOR_BLUE, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, -1}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_galactic_hq: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_galactic_hq: got into the Galactic HQ 2F, third room.", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    pbf_move_left_joystick(context, {0, +1}, 800ms, 100ms); // 3+

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_galactic_hq: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_galactic_hq: entered Galactic HQ's 3F, first room.", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 1);

    {
        MarkDetector galactic_hq_grunt_4_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 10000ms, 100ms); // 5+
            },
            {{galactic_hq_grunt_4_fight}});
        if (ret == 0){
            context.wait_for_all_requests();
            pbf_wait(context, 3000ms);

            BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            ret = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 200000ms);
                },
                {{black_screen}});
            if (ret < 0){
                stream.log("galactic_hq_grunt_4: black screen not detected!", COLOR_RED);
                return false;
            }else if (ret == 0){
                fight_trainer_using_moves(stream, context, stats, "galactic_hq_grunt_4", true);
            }
        }
    }

    {
        // The next grunt can start while the previous battle's dialogue is
        // clearing. Watch for its command menu even if the fade already passed.
        BattleMenuWatcher next_grunt(BattleType::TRAINER);
        const int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_B, 30000ms);
            },
            {{next_grunt}});
        if (ret < 0){
            stream.log("galactic_hq_grunt_5: battle menu not detected!", COLOR_RED);
            return false;
        }
        stream.log("galactic_hq_grunt_5: battle menu detected; starting battle handler.");
        fight_trainer_using_moves(stream, context, stats, "galactic_hq_grunt_5", true);
    }
    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    pbf_move_left_joystick(context, {-1, 0}, 4400ms, 100ms); // 22+
    pbf_move_left_joystick(context, {0, -1}, 2200ms, 100ms); // 11+
    pbf_move_left_joystick(context, {-1, 0}, 1200ms, 100ms); // 5+

    {
        BlackScreenFlashWatcher black_screen(COLOR_BLUE, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_galactic_hq: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_galactic_hq: got into the Galactic HQ 1F, fifth room.", COLOR_GREEN);
    };

    // ---------------------------------------- The error happened around here ---------------------------------------- //

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    pbf_move_left_joystick(context, {0, +1}, 800ms, 100ms); // 3+

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_galactic_hq: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_galactic_hq: entered Galactic HQ's 4F.", COLOR_GREEN);
    };

    // ---------------------------------------- Until here ---------------------------------------- //

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 5);

    {
        ShortDialogWatcher open_door(COLOR_RED);
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {0, +1}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{open_door}});
        if (ret == 0){
            stream.log("go_through_galactic_hq: Opened the door with the Galactic Key.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 4000ms);
        }else if (ret < 0){
            stream.log("go_through_galactic_hq: Didn't detect door opening dialog box!", COLOR_RED);
            return false;
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    {
        ShortDialogWatcher cyrus_dialog(COLOR_RED);
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms); // 5+
            },
            {{cyrus_dialog}});
        if (ret < 0){
            stream.log("go_through_galactic_hq: Cyrus dialogue not detected.", COLOR_RED);
            return false;
        }
        if (ret == 0){
            stream.log("go_through_galactic_hq: Cyrus dialog box detected.", COLOR_GREEN);

            BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            ret = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 200000ms);
                },
                {{black_screen}});
            if (ret < 0){
                stream.log("galactic_hq_cyrus: black screen not detected!", COLOR_RED);
                return false;
            }else if (ret == 0){
                fight_trainer_using_moves(stream, context, stats, "galactic_hq_cyrus", true);
            }
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    pbf_mash_button(context, BUTTON_A, 17000ms);

    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {0, +1}, 800ms, 100ms);  // 3+
    pbf_move_left_joystick(context, {+1, 0}, 1400ms, 100ms); // 6+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 2);
    pbf_move_left_joystick(context, {+1, 0}, 1600ms, 100ms); // 8+

    {
        BlackScreenFlashWatcher black_screen(COLOR_BLUE, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_galactic_hq: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_galactic_hq: got into the Galactic HQ 4F, second room.", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    pbf_move_left_joystick(context, {+1, 0}, 7600ms, 100ms); // 38+
    pbf_move_left_joystick(context, {0, +1}, 1600ms, 100ms); // 8+
    pbf_move_left_joystick(context, {-1, 0}, 1400ms, 100ms); // 6+

    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 3);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_galactic_hq: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_galactic_hq: entered Galactic HQ's 4F Back.", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 2);
    pbf_move_left_joystick(context, {0, +1}, 1600ms, 100ms); // 7+

    {
        ShortDialogWatcher talk_to_commander_saturn(COLOR_RED);
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {+1, 0}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{talk_to_commander_saturn}});
        if (ret < 0){
            stream.log("go_through_galactic_hq: Didn't detect Commander Saturn dialog box!", COLOR_RED);
            return false;
        }else if (ret == 0){
            stream.log("go_through_galactic_hq: Starting the battle with Commander Saturn.", COLOR_GREEN);

            BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            ret = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 200000ms);
                },
                {{black_screen}});
            if (ret < 0){
                stream.log("galactic_hq_commander_saturn: black screen not detected!", COLOR_RED);
                return false;
            }else if (ret == 0){
                fight_trainer_using_moves(stream, context, stats, "galactic_hq_commander_saturn", true);
            }
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    pbf_mash_button(context, BUTTON_B, 2000ms);
    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {+1, 0}, 400ms, 100ms); // 1+
    pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms); // 1+

    {
        ShortDialogWatcher unlock_guardians(COLOR_RED);
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {0, +1}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{unlock_guardians}});
        if (ret == 0){
            stream.log("go_through_galactic_hq: Released the Lake Guardians.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 4000ms);
            pbf_mash_button(context, BUTTON_B, 8000ms);
        }else if (ret < 0){
            stream.log("go_through_galactic_hq: Didn't detect button dialog box!", COLOR_RED);
            return false;
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 2);
    pbf_move_left_joystick(context, {0, -1}, 1600ms, 100ms); // 7+
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 2);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, -1}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_galactic_hq: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_galactic_hq: exited Galactic HQ's 4F Back.", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 8);

    {
        BlackScreenFlashWatcher black_screen(COLOR_BLUE, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_galactic_hq: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_galactic_hq: exited Galactic HQ 4F second room, entering 4F first room.", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    pbf_move_left_joystick(context, {0, -1}, 1400ms, 100ms); // 6+
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 3);
    pbf_move_left_joystick(context, {0, -1}, 1800ms, 100ms); // 9+
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 1);

    {
        BlackScreenFlashWatcher black_screen(COLOR_BLUE, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_galactic_hq: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_galactic_hq: exited Galactic HQ 4F first room, entering 1F sixth room.", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    pbf_move_left_joystick(context, {0, -1}, 1600ms, 100ms); // 8+
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 1);

    {
        WhiteScreenOverWatcher white_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, -1}, 10000ms, 100ms);
            },
            {{white_screen}});
        if (ret < 0){
            stream.log("go_through_galactic_hq: white screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_galactic_hq: exited Galactic HQ.", COLOR_GREEN);
    };

    return true;
}

static bool fly_and_heal_at_eterna(VideoStream& stream, ProControllerContext& context, AutoStoryOptions options){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("fly_and_heal_at_eterna: Flying to Eterna City and healing pokemon...");

    if (!fly_to(stream, context, FlyPoint::EternaCity)){
        stream.log("Flight to Eterna failed; stopping before the next route block.", COLOR_RED);
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
            stream.log("fly_and_heal_at_eterna: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("fly_and_heal_at_eterna: entered Eterna City's Pokemon Center.", COLOR_GREEN);
    };

    return true;
}

static bool go_to_mount_coronet(VideoStream& stream,
    ProControllerContext& context,
    AutoStoryStats& stats,
    AutoStoryOptions /*options*/){
    stream.log("[08.05] Following the recorded Eterna approach with an optional Justin encounter.");
    context.wait_for_all_requests();
    // Try the saved gear first. An early reset tries the other setup by toggling B while mounted.
    stream.log("[08.05] Mounting the bicycle for the recorded approach.");
    pbf_press_button(context, BUTTON_PLUS, 80ms, 800ms);
    context.wait_for_all_requests();
    if (stats.coronet_toggle_bike_gear){
        stream.log("[08.05] Bicycle setup: toggle gear once with B.");
        pbf_wait(context, 500ms);
        pbf_press_button(context, BUTTON_B, 80ms, 800ms);
        context.wait_for_all_requests();
    }else{
        stream.log("[08.05] Bicycle setup: keep the saved gear without a B toggle.");
    }
    const char* const gate_labels[] = {
        "Cycling Road north gate entrance",
        "Cycling Road north gate exit",
        "Cycling Road south gate entrance",
        "Cycling Road south gate exit",
    };
    for (size_t gate = 0; gate < 4; gate++){
        context.wait_for_all_requests();
        BlackScreenWatcher black;
        WhiteScreenWatcher white;
        const int entered = run_until<ProControllerContext>(stream,
            context,
            [gate](ProControllerContext& ctx){
                switch (gate){
                case 0:
                    pbf_move_left_joystick(ctx, {0, -1}, 3502ms, 0ms);
                    break;
                case 1:
                    pbf_move_left_joystick(ctx, {0, -1}, 1880ms, 0ms);
                    break;
                case 2:
                    // Combine the 1572ms, 249ms, and 674ms downhill holds.
                    pbf_move_left_joystick(ctx, {0, -1}, 2495ms, 0ms);
                    pbf_wait(ctx, 4700ms);
                    // Clear the trainer, then let Cycling Road's slope carry us to Kayla.
                    pbf_move_left_joystick(ctx, {1, 0}, 2852ms, 0ms);
                    pbf_wait(ctx, 7922ms);
                    // Line up with the south-gate doorway.
                    pbf_move_left_joystick(ctx, {-1, 0}, 407ms, 0ms);
                    pbf_wait(ctx, 836ms);
                    pbf_move_left_joystick(ctx, {0, -1}, 985ms, 0ms);
                    break;
                case 3:
                    pbf_move_left_joystick(ctx, {0, -1}, 2126ms, 0ms);
                    break;
                }
                // Observe a delayed doorway fade; cancel as soon as it starts.
                pbf_wait(ctx, 2000ms);
            },
            {{black}, {white}});
        if (entered < 0){
            stream.log(std::string("[08.05] ") + gate_labels[gate] + ": transition did not start.", COLOR_RED);
            return false;
        }
        BlackScreenWatcher black_gone(COLOR_RED, {0.1, 0.1, 0.8, 0.8}, 100, 10,
            BlackScreenWatcher::FinderType::GONE, 500ms);
        WhiteScreenWatcher white_gone(COLOR_RED, {0.1, 0.1, 0.8, 0.8}, 500, 10,
            WhiteScreenWatcher::FinderType::GONE, 500ms);
        const int finished = entered == 1
            ? wait_until(stream, context, 30s, {{white_gone}})
            : wait_until(stream, context, 30s, {{black_gone}});
        if (finished != 0){
            stream.log(std::string("[08.05] ") + gate_labels[gate] + ": transition did not finish.", COLOR_RED);
            return false;
        }
        OverworldWatcher gate_ready;
        if (wait_until(stream, context, 10s, {{gate_ready}}) != 0){
            stream.log(std::string("[08.05] ") + gate_labels[gate] + ": destination not ready.", COLOR_RED);
            return false;
        }
        if (gate == 1){
            stats.coronet_approach_phase = CoronetApproachPhase::ON_CYCLING_ROAD;
        }
        stream.log(std::string("[08.05] ") + gate_labels[gate] + ": transition complete; destination ready.",
            COLOR_GREEN);
    }
    // Leave the gate, dismount, and approach Justin on foot.
    pbf_move_left_joystick(context, {0, -1}, 1931ms, 0ms);
    pbf_wait(context, 962ms);
    pbf_press_button(context, BUTTON_PLUS, 163ms, 0ms);
    pbf_wait(context, 994ms);
    pbf_move_left_joystick(context, {0, -1}, 472ms, 0ms);
    pbf_wait(context, 179ms);
    context.wait_for_all_requests();
    ShortDialogWatcher justin_dialogue;
    StartBattleDetector justin_starting(stream.overlay());
    BattleMenuWatcher justin_menu(BattleType::TRAINER);
    const int justin_encounter = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& ctx){
            pbf_move_left_joystick(ctx, {1, 0}, 6105ms, 0ms);
            pbf_wait(ctx, 1045ms);
            pbf_wait(ctx, 3000ms);
        },
        {{justin_dialogue}, {justin_starting}, {justin_menu}});
    if (justin_encounter < 0){
        stream.log("[Route 207 Hiker Justin] No encounter after approach; trainer dodged, continuing.",
            COLOR_GREEN);
    }else{
        stream.log("[Route 207 Hiker Justin] Encounter detected; route movement cancelled.");
        fight_trainer_using_moves(stream, context, stats, "Route 207 Hiker Justin", false);
    }
    // Position by the sign above the Coronet entrance.
    pbf_move_left_joystick(context, {1, 0}, 1376ms, 0ms);
    pbf_wait(context, 1069ms);
    pbf_move_left_joystick(context, {0, 1}, 816ms, 0ms);
    pbf_wait(context, 1822ms);
    context.wait_for_all_requests();
    BlackScreenWatcher cave_black;
    WhiteScreenWatcher cave_white;
    const int cave_entered = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& ctx){
            // Face south, step into the entrance lane, face east, then enter.
            pbf_press_dpad(ctx, DPAD_DOWN, 80ms, 100ms);
            pbf_press_dpad(ctx, DPAD_DOWN, 80ms, 100ms);
            pbf_press_dpad(ctx, DPAD_RIGHT, 80ms, 100ms);
            pbf_press_dpad(ctx, DPAD_RIGHT, 80ms, 100ms);
            // Keep watching for the entry fade after the final step.
            pbf_wait(ctx, 2000ms);
        },
        {{cave_black}, {cave_white}});
    if (cave_entered < 0){
        enter_cave_with_down_right_search(stream, context, "Mount Coronet west entrance");
    }
    BlackScreenWatcher cave_black_gone(COLOR_RED, {0.1, 0.1, 0.8, 0.8}, 100, 10,
        BlackScreenWatcher::FinderType::GONE, 500ms);
    WhiteScreenWatcher cave_white_gone(COLOR_RED, {0.1, 0.1, 0.8, 0.8}, 500, 10,
        WhiteScreenWatcher::FinderType::GONE, 500ms);
    const int cave_visible = cave_entered == 1
        ? wait_until(stream, context, 30s, {{cave_white_gone}})
        : wait_until(stream, context, 30s, {{cave_black_gone}});
    if (cave_visible != 0){
        stream.log("[08.05] Coronet entry transition did not finish.", COLOR_RED);
        return false;
    }
    OverworldWatcher coronet_interior;
    if (wait_until(stream, context, 10s, {{coronet_interior}}) != 0){
        stream.log("[08.05] Coronet interior is not ready for the Surf path.", COLOR_RED);
        return false;
    }
    stats.coronet_approach_phase = CoronetApproachPhase::INSIDE_CORONET;
    stream.log("[08.05] Inside Coronet; continuing with the new Surf ascent.", COLOR_GREEN);
    return true;
}

static bool go_through_mount_coronet(VideoStream& stream,
    ProControllerContext& context,
    AutoStoryStats& stats,
    AutoStoryOptions options,
    bool summit_events = false){
    context.wait_for_all_requests();
    if (!summit_events){
        stream.log("[08.05] Recorded Coronet ascent to the Spear Pillar entrance.");
        if (!activate_repel(stream, context, 8, 1)){
            stream.log("[08.05] Could not activate the Super Repel.", COLOR_RED);
            return false;
        }
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 141ms, 0ms);
        pbf_wait(context, 611ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 322ms, 0ms);
        pbf_wait(context, 706ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 633ms, 0ms);
        pbf_wait(context, 158ms);
        // Surf: complete the approach, then clear the field-move prompts.
        pbf_press_button(context, BUTTON_A, 169ms, 0ms);
        clear_dialog(stream, context, BUTTON_A, 6000ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 4373ms, 0ms);
        pbf_wait(context, 627ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 37ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, -1.000000}, 6526ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 34ms, 0ms);
        pbf_wait(context, 871ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 90ms, 0ms);
        pbf_wait(context, 588ms);
        // Rock Climb: complete the approach, then clear the field-move prompts.
        pbf_press_button(context, BUTTON_A, 193ms, 0ms);
        clear_dialog(stream, context, BUTTON_A, 6000ms);
        // Enter the stair vestibule before continuing through the next room.
        context.wait_for_all_requests();
        BlackScreenWatcher vestibule_entry(COLOR_RED);
        const int vestibule_entered = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {-1, 0}, 747ms, 0ms);
                pbf_wait(ctx, 15ms);
                pbf_move_left_joystick(ctx, {0, 1}, 651ms, 0ms);
                pbf_move_left_joystick(ctx, {1, 1}, 50ms, 0ms);
                pbf_move_left_joystick(ctx, {1, 0}, 768ms, 0ms);
                pbf_wait(ctx, 1771ms);
            },
            {{vestibule_entry}});
        if (vestibule_entered != 0){
            stream.log("[08.05] Second-room starting stairs: entry transition missing.", COLOR_RED);
            return false;
        }
        BlackScreenWatcher vestibule_visible(COLOR_RED, {0.1, 0.1, 0.8, 0.8}, 100, 10,
            BlackScreenWatcher::FinderType::GONE, 500ms);
        if (wait_until(stream, context, 30s, {{vestibule_visible}}) != 0){
            stream.log("[08.05] Second-room starting stairs: transition did not finish.", COLOR_RED);
            return false;
        }
        OverworldWatcher vestibule_ready;
        if (wait_until(stream, context, 10s, {{vestibule_ready}}) != 0){
            stream.log("[08.05] Second-room starting stairs: destination not ready.", COLOR_RED);
            return false;
        }
        // secondroom.txt starts at the top of these entry stairs.
        struct RoomMove {
            JoystickPosition direction;
            Milliseconds hold;
            Milliseconds rest;
            DpadPosition backstep = DPAD_NONE;
        };
        const RoomMove second_room_to_renewal[] = {
            {{1, 0}, 677ms, 176ms},
            {{0, 1}, 1990ms, 105ms},
            {{-1, 0}, 2449ms, 125ms},
            {{0, -1}, 3277ms, 55ms},
            {{-1, 0}, 945ms, 135ms},
            {{0, 1}, 2175ms, 26ms},
            {{-1, 0}, 1877ms, 70ms},
            {{0, 1}, 3184ms, 0ms},
            {{1, 0}, 2413ms, 0ms},
            {{1, 1}, 30ms, 0ms},
            {{0, 1}, 1250ms, 708ms},
            {{-1, 0}, 1266ms, 587ms},
            // Seat against the lower wall, then back up one tile to the stairs.
            {{0, -1}, 1000ms, 737ms, DPAD_UP},
            {{1, 0}, 2596ms, 204ms},
            {{0, 1}, 2262ms, 44ms},
            {{1, 0}, 1455ms, 39ms},
            {{0, 1}, 1467ms, 0ms},
            {{1, 1}, 62ms, 0ms},
            {{1, 0}, 3815ms, 712ms},
            {{-1, 0}, 33ms, 0ms},
            {{-1, -1}, 489ms, 0ms},
            {{0, -1}, 323ms, 688ms},
        };
        // There are no trainers in this room. Wait for natural Repel expiry, then accept renewal
        // through the dialogue routine.
        size_t completed_room_moves = 0;
        ShortDialogWatcher room_repel_dialogue(COLOR_YELLOW);
        const int room_expiry = run_until<ProControllerContext>(stream,
            context,
            [&](ProControllerContext& ctx){
                for (const auto& move : second_room_to_renewal){
                    pbf_move_left_joystick(ctx, move.direction, move.hold, 0ms);
                    ctx.wait_for_all_requests();
                    if (move.backstep != DPAD_NONE){
                        // The backstep reverses the joystick approach: turn, then move one tile.
                        DpadState backstep_direction;
                        repeat_dpad(ctx, backstep_direction, move.backstep, 80ms, 300ms, 2, false);
                        ctx.wait_for_all_requests();
                    }
                    completed_room_moves++;
                    pbf_wait(ctx, move.rest);
                    ctx.wait_for_all_requests();
                }
                pbf_wait(ctx, 3000ms);
            },
            {{room_repel_dialogue}});
        constexpr size_t ROOM_MOVES_TO_RENEWAL = sizeof(second_room_to_renewal) / sizeof(RoomMove);
        if (room_expiry == 0){
            if (completed_room_moves < ROOM_MOVES_TO_RENEWAL){
                stream.log("[08.05] Bridge-room expiry occurred before the recorded renewal leg; stopping rather than skipping movement.", COLOR_RED);
                return false;
            }
            stream.log("[08.05] Bridge room: natural Repel expiry detected; accepting renewal.");
            clear_dialog(stream, context, BUTTON_A, 6000ms);
        }else{
            stream.log("[08.05] Bridge room: no expiry at the recorded stop; continuing without forcing expiration.");
        }
        const RoomMove second_room_to_expiry_lane[] = {
            {{1, 0}, 318ms, 0ms},
            {{1, -1}, 524ms, 0ms},
            {{0, -1}, 75ms, 1421ms},
            {{-1, 0}, 4ms, 0ms},
            {{-1, -1}, 3508ms, 0ms},
            {{-1, 0}, 171ms, 0ms},
            {{0, 1}, 1759ms, 108ms},
        };
        ShortDialogWatcher room_lane_repel(COLOR_YELLOW);
        const int lane_expiry = run_until<ProControllerContext>(stream,
            context,
            [&](ProControllerContext& ctx){
                for (const auto& move : second_room_to_expiry_lane){
                    pbf_move_left_joystick(ctx, move.direction, move.hold, move.rest);
                    ctx.wait_for_all_requests();
                }
            },
            {{room_lane_repel}});
        if (lane_expiry == 0){
            stream.log("[08.05] Repel expired before reaching the bridge-room renewal lane; stopping before skipping movement.", COLOR_RED);
            return false;
        }
        // Renew in the clear east/west corridor before the left turn. An interrupted 500ms wiggle
        // can leave the player west of the starting tile. Reestablish the east-boulder stop
        // before crossing; extending the west hold alone can overshoot the stairs.
        context.wait_for_all_requests();
        stream.log("[08.05] Bridge room: forcing east/west Repel expiration and renewal before restoring the east-boulder stop.");
        force_repel_expiry(stream, context, RepelRenewalChoice::YES, false, true);
        // Twice the expiry half-cycle allows the return to finish against the boulder.
        pbf_move_left_joystick(context, {1, 0}, 1000ms, 0ms);
        context.wait_for_all_requests();
        const RoomMove second_room_to_exit[] = {
            {{-1, 0}, 2491ms, 0ms},
            {{-1, 1}, 21ms, 0ms},
            {{0, 1}, 1733ms, 61ms},
            {{-1, 0}, 1251ms, 2209ms},
            {{-1, 0}, 1622ms, 829ms},
        };
        ShortDialogWatcher late_room_repel(COLOR_YELLOW);
        const int late_expiry = run_until<ProControllerContext>(stream,
            context,
            [&](ProControllerContext& ctx){
                for (const auto& move : second_room_to_exit){
                    pbf_move_left_joystick(ctx, move.direction, move.hold, move.rest);
                    ctx.wait_for_all_requests();
                }
            },
            {{late_room_repel}});
        if (late_expiry == 0){
            // Do not add a second renewal on this leg. Resume interrupted movement before
            // assuming the doorway position is established.
            stream.log("[08.05] Bridge-room exit leg interrupted by another expiry; position requires review.", COLOR_RED);
            return false;
        }
        // The boulder stop leaves us facing west. Allow each step to finish
        // before the next press: 100ms releases swallowed one doorway step.
        DpadState doorway_direction;
        doorway_direction.last_dir = DPAD_LEFT;
        repeat_dpad(context, doorway_direction, DPAD_RIGHT, 80ms, 300ms, 2, true);
        context.wait_for_all_requests();
        BlackScreenWatcher stair_room_entry(COLOR_RED);
        const int stair_room_entered = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {0, 1}, 528ms, 0ms);
                pbf_wait(ctx, 2000ms);
            },
            {{stair_room_entry}});
        if (stair_room_entered != 0){
            stream.log("[08.05] Bridge-room exit: transition did not start.", COLOR_RED);
            return false;
        }
        BlackScreenWatcher stair_room_visible(COLOR_RED, {0.1, 0.1, 0.8, 0.8}, 100, 10,
            BlackScreenWatcher::FinderType::GONE, 500ms);
        if (wait_until(stream, context, 30s, {{stair_room_visible}}) != 0){
            stream.log("[08.05] Bridge-room exit: transition did not finish.", COLOR_RED);
            return false;
        }
        OverworldWatcher stair_room_ready;
        if (wait_until(stream, context, 10s, {{stair_room_ready}}) != 0){
            stream.log("[08.05] Bridge-room exit: destination not ready.", COLOR_RED);
            return false;
        }
        stream.log("[08.05] Replacement second-room route complete; continuing toward the landing grunt.", COLOR_GREEN);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 893ms, 0ms);
        pbf_wait(context, 117ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 127ms, 0ms);
        pbf_wait(context, 513ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 1244ms, 0ms);
        pbf_wait(context, 817ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 760ms, 0ms);
        pbf_wait(context, 338ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 151ms, 0ms);
        pbf_wait(context, 461ms);
        context.wait_for_all_requests();
        ShortDialogWatcher landing_dialogue;
        StartBattleDetector landing_starting(stream.overlay());
        BattleMenuWatcher landing_menu(BattleType::TRAINER);
        const int landing_encounter = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& ctx){
                // Use the same simultaneous movement/mashing pattern as Segment 05.
                ssf_press_left_joystick(ctx, {0, 1}, 0ms, 2150ms);
                ssf_mash1_button(ctx, BUTTON_A, 2150ms);
                pbf_wait(ctx, 3000ms);
            },
            {{landing_dialogue}, {landing_starting}, {landing_menu}});
        if (landing_encounter < 0){
            stream.log("[08.05] Landing grunt dialogue or battle did not start.", COLOR_RED);
            return false;
        }
        fight_trainer_using_moves(stream, context, stats, "coronet_ascent_landing_grunt", true);
        // Face east, then move one tile clear of the grunt before expiring north/south.
        pbf_press_dpad(context, DPAD_RIGHT, 80ms, 100ms);
        pbf_press_dpad(context, DPAD_RIGHT, 80ms, 100ms);
        context.wait_for_all_requests();
        stream.log("[08.05] Third Repel renewal: north/south lane east of the defeated landing grunt.");
        force_repel_expiry(stream, context, RepelRenewalChoice::YES, true, true);
        // Restore the height against the north wall before the westward boulder stop.
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 2150ms, 0ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 900ms, 0ms);
        pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 500ms, 0ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1200ms, 0ms);
        pbf_press_dpad(context, DPAD_DOWN, 80ms, 100ms);
        pbf_press_dpad(context, DPAD_DOWN, 80ms, 100ms);
        pbf_press_dpad(context, DPAD_DOWN, 80ms, 100ms);
        pbf_press_dpad(context, DPAD_DOWN, 80ms, 100ms);
        pbf_press_dpad(context, DPAD_DOWN, 80ms, 100ms);
        pbf_press_dpad(context, DPAD_DOWN, 80ms, 100ms);
        pbf_press_dpad(context, DPAD_DOWN, 80ms, 100ms);
        pbf_press_dpad(context, DPAD_DOWN, 80ms, 100ms);
        // Let the approach settle before the extra south step into the patrol sight line.
        context.wait_for_all_requests();
        pbf_wait(context, 300ms);
        pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
        context.wait_for_all_requests();
        stream.log("[08.05] Waiting south of the boulder for the patrolling grunt.");
        {
            BattleMenuWatcher menu(BattleType::TRAINER);
            const int ret = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& ctx){
                    pbf_mash_button(ctx, BUTTON_B, 90000ms);
                },
                {{menu}});
            if (ret != 0){
                stream.log("coronet_ascent_grunt_1: battle menu not detected.", COLOR_RED);
                return false;
            }
            fight_trainer_using_moves(stream, context, stats, "coronet_ascent_grunt_1", true);
        }
        pbf_press_dpad(context, DPAD_LEFT, 80ms, 100ms);
        pbf_press_dpad(context, DPAD_LEFT, 80ms, 100ms);
        pbf_wait(context, 962ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 1790ms, 0ms);
        pbf_wait(context, 905ms);
        pbf_press_dpad(context, DPAD_RIGHT, 185ms, 0ms);
        pbf_wait(context, 632ms);
        pbf_press_dpad(context, DPAD_DOWN, 194ms, 0ms);
        pbf_wait(context, 7097ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 409ms, 0ms);
        pbf_wait(context, 100ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 2677ms, 0ms);
        pbf_wait(context, 79ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 1254ms, 0ms);
        pbf_wait(context, 358ms);
        // Finish the snowfield approach before tapping A at the Rock Climb wall. Keep movement
        // separate from field-move interaction.
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 189ms, 0ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 173ms, 0ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 1283ms, 0ms);
        pbf_wait(context, 77ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 378ms, 0ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 75ms, 0ms);
        pbf_press_button(context, BUTTON_A, 80ms, 100ms);
        wait_for_dialog(stream, context, "[08.05] Snowfield Rock Climb", 10s);
        clear_dialog(stream, context, BUTTON_A, 6000ms);
        // Turn west at the landing, then descend south into the grass. Do not detour east or
        // approach the next outdoor wall.
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 2078ms, 0ms);
        pbf_wait(context, 68ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 599ms, 0ms);
        pbf_wait(context, 120ms);
        // Keep the westward stop, then turn and step one tile east into the stairs' lane.
        context.wait_for_all_requests();
        DpadState snow_stair_direction;
        snow_stair_direction.last_dir = DPAD_LEFT;
        repeat_dpad(context, snow_stair_direction, DPAD_RIGHT, 80ms, 300ms, 1, true);
        context.wait_for_all_requests();
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 1458ms, 0ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 589ms, 0ms);
        pbf_move_left_joystick(context, {-1.000000, 1.000000}, 211ms, 0ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 665ms, 0ms);
        pbf_wait(context, 56ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 634ms, 0ms);
        pbf_wait(context, 68ms);
        context.wait_for_all_requests();
        stream.log("[08.05] Snowfield cave approach: renewing Repel east/west in the grass.");
        force_repel_expiry(stream, context, RepelRenewalChoice::YES, false, true);
        pbf_move_left_joystick(context, {-1, 0}, 1500ms, 0ms);
        context.wait_for_all_requests();
        {
            BlackScreenWatcher entered(COLOR_RED);
            const int ret = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& ctx){
                    // Observe the north approach too, in case it reaches the doorway early.
                    pbf_move_left_joystick(ctx, {0, 1}, 2000ms, 0ms);
                    pbf_move_left_joystick(ctx, {1, 1}, 10000ms, 0ms);
                },
                {{entered}});
            if (ret != 0){
                stream.log("[08.05] Snowfield cave entry: transition did not start.", COLOR_RED);
                return false;
            }
            BlackScreenWatcher visible(COLOR_RED, {0.1, 0.1, 0.8, 0.8}, 100, 10,
                BlackScreenWatcher::FinderType::GONE, 500ms);
            if (wait_until(stream, context, 30s, {{visible}}) != 0){
                stream.log("[08.05] Snowfield cave entry: transition did not finish.", COLOR_RED);
                return false;
            }
            OverworldWatcher ready;
            if (wait_until(stream, context, 10s, {{ready}}) != 0){
                stream.log("[08.05] Snowfield cave entry: destination not ready.", COLOR_RED);
                return false;
            }
        }
        stream.log("[08.05] Snowfield cave entered; continuing the recorded interior route.", COLOR_GREEN);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 1333ms, 0ms);
        pbf_wait(context, 66ms);
        // Continue Rock Climb inside the cave.
        pbf_press_button(context, BUTTON_A, 199ms, 0ms);
        clear_dialog(stream, context, BUTTON_A, 6000ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 1056ms, 0ms);
        pbf_wait(context, 315ms);
        pbf_press_button(context, BUTTON_A, 213ms, 0ms);
        {
            BattleMenuWatcher menu(BattleType::TRAINER);
            const int ret = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& ctx){
                    pbf_mash_button(ctx, BUTTON_B, 90000ms);
                },
                {{menu}});
            if (ret != 0){
                stream.log("coronet_ascent_grunt_2: battle menu not detected.", COLOR_RED);
                return false;
            }
            fight_trainer_using_moves(stream, context, stats, "coronet_ascent_grunt_2", true);
        }
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 544ms, 0ms);
        pbf_wait(context, 277ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 812ms, 0ms);
        pbf_wait(context, 123ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 344ms, 0ms);
        pbf_wait(context, 656ms);
        // This grunt can be dodged on the east pass and again on the south turn.
        // Watch dialogue and the battle transition too, so movement stops before the menu appears.
        context.wait_for_all_requests();
        bool grunt_3_defeated = false;
        ShortDialogWatcher east_dialogue;
        StartBattleDetector east_starting(stream.overlay());
        BattleMenuWatcher east_menu(BattleType::TRAINER);
        const int east_encounter = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {1, 0}, 2077ms, 0ms);
                pbf_wait(ctx, 922ms);
                pbf_move_left_joystick(ctx, {1, 0}, 555ms, 0ms);
                pbf_wait(ctx, 342ms);
            },
            {{east_menu}, {east_dialogue}, {east_starting}});
        if (east_encounter >= 0){
            fight_trainer_using_moves(stream, context, stats, "coronet_ascent_grunt_3", true);
            grunt_3_defeated = true;
            // Restore the east-wall stop if the encounter interrupted the crossing.
            pbf_move_left_joystick(context, {1, 0}, 2632ms, 0ms);
            context.wait_for_all_requests();
        }else{
            stream.log("[08.05] Grunt 3 dodged on the east pass; checking the south turn.");
        }
        if (!grunt_3_defeated){
            ShortDialogWatcher south_dialogue;
            StartBattleDetector south_starting(stream.overlay());
            BattleMenuWatcher south_menu(BattleType::TRAINER);
            const int south_encounter = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& ctx){
                    pbf_move_left_joystick(ctx, {0, -1}, 300ms, 0ms);
                    pbf_wait(ctx, 750ms);
                    // No encounter on the short turn: continue to the cliff edge.
                    pbf_move_left_joystick(ctx, {0, -1}, 2000ms, 0ms);
                    pbf_wait(ctx, 1000ms);
                },
                {{south_menu}, {south_dialogue}, {south_starting}});
            if (south_encounter >= 0){
                fight_trainer_using_moves(stream, context, stats, "coronet_ascent_grunt_3", true);
                grunt_3_defeated = true;
                pbf_move_left_joystick(context, {1, 0}, 555ms, 0ms);
            }else{
                stream.log("[08.05] Grunt 3 dodged on the south pass; continuing from the cliff edge.");
            }
        }
        if (grunt_3_defeated){
            // A battle can cancel the south hold. Both encounter branches restore this stop.
            pbf_move_left_joystick(context, {0, -1}, 2000ms, 0ms);
        }
        context.wait_for_all_requests();
        DpadState grunt_stair_direction;
        grunt_stair_direction.last_dir = DPAD_DOWN;
        repeat_dpad(context, grunt_stair_direction, DPAD_RIGHT, 80ms, 300ms, 2, true);
        context.wait_for_all_requests();
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 508ms, 0ms);
        pbf_wait(context, 434ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 777ms, 0ms);
        pbf_wait(context, 130ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 564ms, 0ms);
        pbf_wait(context, 85ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 1551ms, 0ms);
        pbf_wait(context, 951ms);
        // Face north and step into the exit lane, then face east and enter. A single Right press
        // may only turn the player.
        context.wait_for_all_requests();
        {
            BlackScreenWatcher snowfield_entry(COLOR_RED);
            WhiteScreenWatcher snowfield_entry_white;
            const int entered = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& ctx){
                    DpadState exit_direction;
                    exit_direction.last_dir = DPAD_RIGHT;
                    repeat_dpad(ctx, exit_direction, DPAD_UP, 80ms, 300ms, 1, true);
                    repeat_dpad(ctx, exit_direction, DPAD_RIGHT, 80ms, 300ms, 1, true);
                    pbf_wait(ctx, 2000ms);
                },
                {{snowfield_entry}, {snowfield_entry_white}});
            if (entered < 0){
                stream.log("[08.05] Upper snowfield exit: transition did not start.", COLOR_RED);
                return false;
            }
            BlackScreenWatcher visible(COLOR_RED, {0.1, 0.1, 0.8, 0.8}, 100, 10,
                BlackScreenWatcher::FinderType::GONE, 500ms);
            WhiteScreenWatcher white_visible(COLOR_RED, {0.1, 0.1, 0.8, 0.8}, 500, 10,
                WhiteScreenWatcher::FinderType::GONE, 500ms);
            const int finished = entered == 1
                ? wait_until(stream, context, 30s, {{white_visible}})
                : wait_until(stream, context, 30s, {{visible}});
            if (finished != 0){
                stream.log("[08.05] Upper snowfield exit: transition did not finish.", COLOR_RED);
                return false;
            }
            OverworldWatcher ready;
            if (wait_until(stream, context, 10s, {{ready}}) != 0){
                stream.log("[08.05] Upper snowfield exit: destination not ready.", COLOR_RED);
                return false;
            }
        }
        stream.log("[08.05] Upper snowfield exit complete; continuing toward the renewal lane.", COLOR_GREEN);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 988ms, 0ms);
        pbf_wait(context, 34ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 536ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, 1.000000}, 36ms, 0ms);
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 557ms, 0ms);
        pbf_wait(context, 84ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 566ms, 0ms);
        pbf_wait(context, 599ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 661ms, 0ms);
        pbf_wait(context, 306ms);
        // Renew in the clear north/south lane before the next Rock Climb approach.
        context.wait_for_all_requests();
        stream.log("[08.05] Snowy climb approach: renewing Repel north/south.");
        force_repel_expiry(stream, context, RepelRenewalChoice::YES, true, true);
        // Add one 500ms expiry half-cycle to restore the north-cliff stop.
        pbf_move_left_joystick(context, {0, 1}, 2360ms, 0ms);
        pbf_wait(context, 379ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 325ms, 0ms);
        pbf_wait(context, 403ms);
        // Rock Climb: complete the approach, then clear the field-move prompts.
        pbf_press_button(context, BUTTON_A, 167ms, 0ms);
        clear_dialog(stream, context, BUTTON_A, 6000ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 2156ms, 0ms);
        pbf_wait(context, 132ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 1125ms, 0ms);
        pbf_wait(context, 117ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1294ms, 0ms);
        // Complete the cave approach, then clear the field-move prompts.
        pbf_press_button(context, BUTTON_A, 194ms, 0ms);
        clear_dialog(stream, context, BUTTON_A, 6000ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1410ms, 0ms);
        pbf_wait(context, 196ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 400ms, 0ms);
        pbf_move_left_joystick(context, {-1.000000, 1.000000}, 26ms, 0ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1153ms, 0ms);
        pbf_wait(context, 120ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 640ms, 0ms);
        pbf_wait(context, 94ms);
        context.wait_for_all_requests();
        {
            BlackScreenWatcher passage_entry(COLOR_RED);
            WhiteScreenWatcher passage_entry_white;
            const int entered = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& ctx){
                    pbf_move_left_joystick(ctx, {1, 0}, 690ms, 0ms);
                    pbf_wait(ctx, 2000ms);
                },
                {{passage_entry}, {passage_entry_white}});
            if (entered < 0){
                stream.log("[08.05] Upper cave passage entry: transition did not start.", COLOR_RED);
                return false;
            }
            BlackScreenWatcher visible(COLOR_RED, {0.1, 0.1, 0.8, 0.8}, 100, 10,
                BlackScreenWatcher::FinderType::GONE, 500ms);
            WhiteScreenWatcher white_visible(COLOR_RED, {0.1, 0.1, 0.8, 0.8}, 500, 10,
                WhiteScreenWatcher::FinderType::GONE, 500ms);
            const int finished = entered == 1
                ? wait_until(stream, context, 30s, {{white_visible}})
                : wait_until(stream, context, 30s, {{visible}});
            if (finished != 0){
                stream.log("[08.05] Upper cave passage entry: transition did not finish.", COLOR_RED);
                return false;
            }
            OverworldWatcher ready;
            if (wait_until(stream, context, 10s, {{ready}}) != 0){
                stream.log("[08.05] Upper cave passage entry: destination not ready.", COLOR_RED);
                return false;
            }
        }
        // Restore both axes against fixed stops: south wall, west wall, then north boulder.
        // Clear the doorway and reach the south wall before expiring north/south.
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 317ms, 0ms);
        pbf_wait(context, 389ms);
        pbf_move_left_joystick(context, {0, -1}, 1500ms, 0ms);
        context.wait_for_all_requests();
        stream.log("[08.05] Upper cave south wall: renewing Repel north/south, clear of the entrance.");
        force_repel_expiry(stream, context, RepelRenewalChoice::YES, true, true);
        // An interrupted wiggle can leave us north of the wall; restore the stop.
        pbf_move_left_joystick(context, {0, -1}, 1500ms, 0ms);
        pbf_move_left_joystick(context, {-1, 0}, 1500ms, 0ms);
        pbf_move_left_joystick(context, {0, 1}, 1000ms, 0ms);
        context.wait_for_all_requests();
        {
            BlackScreenWatcher stairs_entry(COLOR_RED);
            const int stairs_entered = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& ctx){
                    pbf_move_left_joystick(ctx, {1, 0}, 4000ms, 0ms);
                    pbf_wait(ctx, 2000ms);
                },
                {{stairs_entry}});
            if (stairs_entered != 0){
                stream.log("[08.05] Aligned cave stairs: transition did not start.", COLOR_RED);
                return false;
            }
            BlackScreenWatcher stairs_visible(COLOR_RED, {0.1, 0.1, 0.8, 0.8}, 100, 10,
                BlackScreenWatcher::FinderType::GONE, 500ms);
            if (wait_until(stream, context, 30s, {{stairs_visible}}) != 0){
                stream.log("[08.05] Aligned cave stairs: transition did not finish.", COLOR_RED);
                return false;
            }
            OverworldWatcher stairs_ready;
            if (wait_until(stream, context, 10s, {{stairs_ready}}) != 0){
                stream.log("[08.05] Aligned cave stairs: destination not ready.", COLOR_RED);
                return false;
            }
        }
        // Clear the stair landing to the east before descending toward the next grunt.
        pbf_move_left_joystick(context, {1, 0}, 423ms, 0ms);
        pbf_wait(context, 205ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 1784ms, 0ms);
        pbf_wait(context, 2522ms);
        pbf_press_button(context, BUTTON_A, 223ms, 0ms);
        {
            BattleMenuWatcher menu(BattleType::TRAINER);
            const int ret = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& ctx){
                    pbf_mash_button(ctx, BUTTON_B, 90000ms);
                },
                {{menu}});
            if (ret != 0){
                stream.log("coronet_ascent_grunt_4: battle menu not detected.", COLOR_RED);
                return false;
            }
            fight_trainer_using_moves(stream, context, stats, "coronet_ascent_grunt_4", true);
        }
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 506ms, 0ms);
        pbf_wait(context, 444ms);
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 1131ms, 0ms);
        pbf_wait(context, 166ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1011ms, 0ms);
        pbf_wait(context, 376ms);
        pbf_press_button(context, BUTTON_A, 157ms, 0ms);
        {
            BattleMenuWatcher menu(BattleType::TRAINER);
            const int ret = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& ctx){
                    pbf_mash_button(ctx, BUTTON_B, 90000ms);
                },
                {{menu}});
            if (ret != 0){
                stream.log("coronet_ascent_grunt_5: battle menu not detected.", COLOR_RED);
                return false;
            }
            fight_trainer_using_moves(stream, context, stats, "coronet_ascent_grunt_5", true);
        }
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1744ms, 0ms);
        pbf_wait(context, 1295ms);
        pbf_move_left_joystick(context, {-1.000000, -0.000000}, 625ms, 0ms);
        pbf_wait(context, 255ms);
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 2011ms, 0ms);
        pbf_wait(context, 3323ms);
        pbf_press_button(context, BUTTON_A, 196ms, 0ms);
        {
            BattleMenuWatcher menu(BattleType::TRAINER);
            const int ret = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& ctx){
                    pbf_mash_button(ctx, BUTTON_B, 90000ms);
                },
                {{menu}});
            if (ret != 0){
                stream.log("coronet_ascent_grunt_6: battle menu not detected.", COLOR_RED);
                return false;
            }
            fight_trainer_using_moves(stream, context, stats, "coronet_ascent_grunt_6", true);
        }
        // The north hold reaches the wall one tile west of the summit doorway.
        pbf_move_left_joystick(context, {0.000000, 1.000000}, 1577ms, 0ms);
        pbf_wait(context, 881ms);
        context.wait_for_all_requests();
        // Take one eastward step with a separate facing press. Longer presses can carry past the
        // doorway. Arm both fade watchers before entry.
        {
            BlackScreenWatcher entered(COLOR_RED);
            WhiteScreenWatcher entered_white;
            const int ret = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& ctx){
                    DpadState doorway_direction;
                    doorway_direction.last_dir = DPAD_UP;
                    repeat_dpad(ctx, doorway_direction, DPAD_RIGHT, 80ms, 300ms, 1, true);
                    pbf_move_left_joystick(ctx, {0, 1}, 4000ms, 0ms);
                    pbf_wait(ctx, 2000ms);
                },
                {{entered}, {entered_white}});
            if (ret < 0){
                stream.log("[08.05] Spear Pillar doorway transition did not start.", COLOR_RED);
                return false;
            }
            BlackScreenWatcher visible(COLOR_RED, {0.1, 0.1, 0.8, 0.8}, 100, 10,
                BlackScreenWatcher::FinderType::GONE, 500ms);
            WhiteScreenWatcher white_visible(COLOR_RED, {0.1, 0.1, 0.8, 0.8}, 500, 10,
                WhiteScreenWatcher::FinderType::GONE, 500ms);
            const int finished = ret == 1
                ? wait_until(stream, context, 30s, {{white_visible}})
                : wait_until(stream, context, 30s, {{visible}});
            if (finished != 0){
                stream.log("[08.05] Spear Pillar doorway transition did not finish.", COLOR_RED);
                return false;
            }
            OverworldWatcher ready;
            if (wait_until(stream, context, 10s, {{ready}}) != 0){
                stream.log("[08.05] Spear Pillar doorway: destination not ready.", COLOR_RED);
                return false;
            }
            stream.log("[08.05] Spear Pillar doorway transition complete; outside and ready.");
        }
        context.wait_for_all_requests();
        return true;
    }

    stream.log("[08.06] Spear Pillar grunts, commanders and Cyrus.");
    if (!clear_new_item_flags(stream, context, 8)){
        stream.log("[08.06] Could not clear new-item flags before the summit route.", COLOR_RED);
        return false;
    }
    // Outside-help setup leaves a usable Pokemon in Box 1, slot 1, with Box 1 selected. Move it
    // into party slot 2 without replacing the starter. These inputs transfer the Pokemon without
    // field movement.
    stream.log("[08.06] Withdrawing Box 1, slot 1 into party slot 2 for the summit double battle.");
    open_menu(stream, context, MenuCursorPosition::POKEMON, 8);
    pbf_wait(context, GameSettings::instance().MENU_TO_POKEMON_DELAY0);
    pbf_press_button(context, BUTTON_R, 160ms, GameSettings::instance().POKEMON_TO_BOX_DELAY1);
    context.wait_for_all_requests();
    {
        BoxWatcher boxes;
        if (wait_until(stream, context, 10s, {{boxes}}) != 0){
            stream.log("[08.06] Box screen not detected before withdrawing the second Pokemon.", COLOR_RED);
            return false;
        }
    }
    pbf_press_button(context, BUTTON_A, 171ms, 642ms);
    pbf_press_button(context, BUTTON_A, 133ms, 481ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 388ms);
    pbf_press_dpad(context, DPAD_DOWN, 80ms, 309ms);
    pbf_press_button(context, BUTTON_A, 144ms, 921ms);
    context.wait_for_all_requests();
    box_to_overworld(stream, context);
    context.wait_for_all_requests();
    {
        OverworldWatcher ready;
        if (wait_until(stream, context, 10s, {{ready}}) != 0){
            stream.log("[08.06] Overworld not ready after the box transfer.", COLOR_RED);
            return false;
        }
    }
    // Start the summit approach only after returning from the box system.
    {
        BattleMenuWatcher intro(BattleType::TRAINER);
        const int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& ctx){
                for (size_t attempt = 0; attempt < 20; attempt++){
                    pbf_move_left_joystick(ctx, {0, 1}, 300ms, 100ms);
                    pbf_press_button(ctx, BUTTON_A, 100ms, 100ms);
                    pbf_mash_button(ctx, BUTTON_B, 1000ms);
                }
            },
            {{intro}});
        if (ret != 0){
            stream.log("[08.06] Summit grunt battle menu not detected during the approach.", COLOR_RED);
            return false;
        }
    }
    fight_trainer_using_moves(stream, context, stats, "spear_pillar_grunts", true, true);
    // Return the extra Pokemon to the slot freed by the withdrawal, even
    // if it fainted. Leave the starter in party slot 1 for the remaining fights.
    {
        stream.log("[08.06] Returning party slot 2 to Box 1, slot 1 after the grunt double battle.");
        open_menu(stream, context, MenuCursorPosition::POKEMON, 8);
        pbf_wait(context, GameSettings::instance().MENU_TO_POKEMON_DELAY0);
        pbf_press_button(context, BUTTON_R, 160ms, GameSettings::instance().POKEMON_TO_BOX_DELAY1);
        context.wait_for_all_requests();
        // With the extra Pokemon in the party, Box 1 can now be empty.
        BoxWatcher boxes(COLOR_RED, true);
        if (wait_until(stream, context, 10s, {{boxes}}) != 0){
            stream.log("[08.06] Box screen not detected before depositing the second Pokemon.", COLOR_RED);
            return false;
        }
        // Reverse the withdrawal: select party slot 2, pick it up, then move up and right to the
        // empty first slot of Box 1.
        pbf_press_dpad(context, DPAD_LEFT, 80ms, 388ms);
        pbf_press_dpad(context, DPAD_DOWN, 80ms, 309ms);
        pbf_press_button(context, BUTTON_A, 171ms, 642ms);
        pbf_press_button(context, BUTTON_A, 133ms, 481ms);
        pbf_press_dpad(context, DPAD_UP, 80ms, 309ms);
        pbf_press_dpad(context, DPAD_RIGHT, 80ms, 388ms);
        pbf_press_button(context, BUTTON_A, 144ms, 921ms);
        context.wait_for_all_requests();
        box_to_overworld(stream, context);
        context.wait_for_all_requests();
        OverworldWatcher ready;
        if (wait_until(stream, context, 10s, {{ready}}) != 0){
            stream.log("[08.06] Overworld not ready after depositing the second Pokemon.", COLOR_RED);
            return false;
        }
    }
    {
        BattleMenuWatcher commanders(BattleType::TRAINER);
        const int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {0, 1}, 2504ms, 0ms);
                for (size_t attempt = 0; attempt < 180; attempt++){
                    pbf_mash_button(ctx, BUTTON_B, 1000ms);
                    pbf_move_left_joystick(ctx, {0, 1}, 160ms, 100ms);
                }
            },
            {{commanders}});
        if (ret != 0){
            stream.log("[08.06] Mars/Jupiter battle menu not detected after the summit story.", COLOR_RED);
            return false;
        }
        fight_trainer_using_moves(stream, context, stats, "spear_pillar_mars_jupiter", false);
    }
    {
        BattleMenuWatcher cyrus(BattleType::TRAINER);
        const int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& ctx){
                pbf_mash_button(ctx, BUTTON_B, 240000ms);
            },
            {{cyrus}});
        if (ret != 0){
            stream.log("[08.06] Cyrus battle menu not detected after Barry's recovery and the cutscene.", COLOR_RED);
            return false;
        }
        // Barry restores full HP and move PP between the commanders and Cyrus.
        stats.move_pp_remaining = stats.move_pp_capacity;
        stream.log("[Move PP] Barry's summit recovery restored the party before Cyrus.");
        fight_trainer_using_moves(stream, context, stats, "spear_pillar_cyrus", true);
    }
    pbf_move_left_joystick(context, {0, 1}, 694ms, 0ms);
    pbf_press_button(context, BUTTON_A, 100ms, 100ms);
    clear_dialog(stream, context, BUTTON_A, 6000ms);
    pbf_move_left_joystick(context, {0, 1}, 1188ms, 0ms);
    {
        BattleMenuWatcher legendary(BattleType::STANDARD);
        const int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& ctx){
                pbf_mash_button(ctx, BUTTON_B, 30000ms);
            },
            {{legendary}});
        if (ret != 0){
            stream.log("[08.06] Box legendary battle menu not detected.", COLOR_RED);
            return false;
        }
    }
    if (options.catch_box_legendary){
        stream.log("[08.06] Catch Box Legendary enabled; throwing the Master Ball.");
        const CatchResults caught = basic_catcher(stream, context, options.language, "master-ball", 1);
        if (caught.result != CatchResult::POKEMON_CAUGHT || caught.balls_used != 1){
            stream.log("[08.06] Master Ball capture was not confirmed; stopping before the exit route.", COLOR_RED);
            return false;
        }
        stream.log("[08.06] Box legendary capture confirmed.");
    }else{
        // Reuse the wild-battle escape routine, which requires Run to be selected.
        BattleMenuDetector commands(BattleType::STANDARD);
        bool run_selected = false;
        for (size_t attempt = 0; attempt < 12; attempt++){
            context.wait_for_all_requests();
            const auto frame = stream.video().snapshot();
            const BattleMenuOption selected = frame
                ? commands.selected_option(*frame.frame) : BattleMenuOption::UNKNOWN;
            if (selected == BattleMenuOption::RUN){
                run_selected = true;
                break;
            }
            if (selected == BattleMenuOption::UNKNOWN){
                pbf_wait(context, 100ms);
            }else{
                pbf_press_dpad(context, DPAD_DOWN, 80ms, 160ms);
            }
        }
        if (!run_selected || !run_from_battle(stream, context, 30000ms)){
            stream.log("[08.06] Could not confirm escaping the box legendary battle.", COLOR_RED);
            return false;
        }
    }
    clear_dialog(stream, context, BUTTON_B, 6000ms);
    if (options.catch_box_legendary){
        // The grunt partner is back in Box 1, slot 1. The caught legendary
        // occupies party slot 2; leave the starter in slot 1 for the exit route.
        stream.log("[08.06] Depositing the caught box legendary in Box 1, slot 2.");
        open_menu(stream, context, MenuCursorPosition::POKEMON, 8);
        pbf_wait(context, GameSettings::instance().MENU_TO_POKEMON_DELAY0);
        pbf_press_button(context, BUTTON_R, 160ms, GameSettings::instance().POKEMON_TO_BOX_DELAY1);
        context.wait_for_all_requests();
        BoxWatcher boxes;
        if (wait_until(stream, context, 10s, {{boxes}}) != 0){
            stream.log("[08.06] Box screen not detected before depositing the box legendary.", COLOR_RED);
            return false;
        }
        // Box 1 remains selected with its cursor at slot 1. Move party slot 2
        // to the empty next slot beside the returned grunt partner.
        pbf_press_dpad(context, DPAD_LEFT, 80ms, 388ms);
        pbf_press_dpad(context, DPAD_DOWN, 80ms, 309ms);
        pbf_press_button(context, BUTTON_A, 171ms, 642ms);
        pbf_press_button(context, BUTTON_A, 133ms, 481ms);
        pbf_press_dpad(context, DPAD_UP, 80ms, 309ms);
        pbf_press_dpad(context, DPAD_RIGHT, 80ms, 388ms);
        pbf_press_dpad(context, DPAD_RIGHT, 80ms, 388ms);
        pbf_press_button(context, BUTTON_A, 144ms, 921ms);
        context.wait_for_all_requests();
        box_to_overworld(stream, context);
        context.wait_for_all_requests();
        OverworldWatcher ready;
        if (wait_until(stream, context, 10s, {{ready}}) != 0){
            stream.log("[08.06] Overworld not ready after depositing the box legendary.", COLOR_RED);
            return false;
        }
    }
    // Expire on the clear summit platform along the doorway column.
    // Scripted movement may already have exhausted it; no prompt is valid.
    force_repel_expiry(stream, context, RepelRenewalChoice::NO, true, false);
    // Walk south into Coronet before using the Escape Rope.
    {
        BlackScreenWatcher entered(COLOR_RED);
        const int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {0, -1}, 10000ms, 0ms);
            },
            {{entered}});
        if (ret != 0){
            stream.log("[08.06] Coronet return doorway transition did not start.", COLOR_RED);
            return false;
        }
        BlackScreenWatcher visible(COLOR_RED, {0.1, 0.1, 0.8, 0.8}, 100, 10,
            BlackScreenWatcher::FinderType::GONE, 500ms);
        if (wait_until(stream, context, 30s, {{visible}}) != 0){
            stream.log("[08.06] Coronet return doorway transition did not finish.", COLOR_RED);
            return false;
        }
    }
    if (!use_escape_rope(stream, context, 8)){
        stream.log("[08.06] Escape Rope could not be used.", COLOR_RED);
        return false;
    }
    pbf_wait(context, 10000ms);
    context.wait_for_all_requests();
    return true;
}

static bool fly_and_heal_at_veilstone(VideoStream& stream, ProControllerContext& context, AutoStoryOptions options){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("fly_and_heal_at_veilstone: Flying to Veilstone City and healing there...");

    if (!fly_to(stream, context, FlyPoint::VeilstoneCity)){
        stream.log("Flight to Veilstone failed; stopping before the next route block.", COLOR_RED);
        return false;
    }

    context.wait_for_all_requests();

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("fly_and_heal_at_veilstone: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("fly_and_heal_at_veilstone: entered Veilstone City Pokemon Center.", COLOR_GREEN);
    };

    return true;
}

static bool go_to_house_south_veilstone(VideoStream& stream, ProControllerContext& context, AutoStoryOptions options){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_to_house_south_veilstone: heading South...");

    pbf_move_left_joystick(context, {0, -1}, 600ms, 100ms);  // 2+
    pbf_move_left_joystick(context, {-1, 0}, 4000ms, 100ms); // 20+

    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 3);
    pbf_move_left_joystick(context, {0, -1}, 1000ms, 100ms); // 5+
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 9);

    pbf_move_left_joystick(context, {0, -1}, 1400ms, 100ms); // 8
    pbf_move_left_joystick(context, {-1, 0}, 1200ms, 100ms); // 6+
    pbf_move_left_joystick(context, {0, -1}, 2000ms, 100ms); // 10+
    pbf_move_left_joystick(context, {+1, 0}, 1800ms, 100ms); // 7+

    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 3, true);

    pbf_move_left_joystick(context, {+1, 0}, 2200ms, 100ms); // 15
    context.wait_for_all_requests();

    {
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
    }

    return true;
}

static bool
go_through_house_south_veilstone(VideoStream& stream, ProControllerContext& context, AutoStoryOptions options){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_through_house_south_veilstone: moving towards exit...");

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, -1}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_house_south_veilstone: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_house_south_veilstone: exited the house", COLOR_GREEN);
    }

    return true;
}

static bool go_to_house_west_sunyshore(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats, AutoStoryOptions options){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_to_house_west_sunyshore: going towards Sunyshore City...");

    pbf_move_left_joystick(context, {+1, 0}, 600ms, 100ms);  // 2+
    pbf_move_left_joystick(context, {0, -1}, 3800ms, 100ms); // 19+
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 8);

    if (!activate_repel(stream, context, 8, 1)){
        stream.log("go_to_house_west_sunyshore: Could not activate a Super Repel.", COLOR_RED);
        return false;
    }

    pbf_move_left_joystick(context, {0, -1}, 2600ms, 100ms); // 13+

    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);
    pbf_move_left_joystick(context, {0, -1}, 1200ms, 100ms); // 6+
    pbf_move_left_joystick(context, {-1, 0}, 400ms, 100ms);  // 2
    pbf_move_left_joystick(context, {0, -1}, 1000ms, 100ms); // 5+

    pbf_move_left_joystick(context, {+1, 0}, 1600ms, 100ms); // 8+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);
    pbf_move_left_joystick(context, {0, -1}, 1600ms, 100ms); // 8+

    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);
    pbf_move_left_joystick(context, {0, -1}, 2200ms, 100ms); // 11+

    pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms); // 4+
    pbf_move_left_joystick(context, {0, -1}, 600ms, 100ms); // 3+
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 2);

    pbf_move_left_joystick(context, {0, -1}, 6200ms, 100ms); // 31+
    context.wait_for_all_requests();

    // Keep the original Super Repel through the grass. Without an expiry wiggle, no eastward
    // reset is needed. Use nine left presses: one facing change followed by eight walking steps.
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 8);
    // This southward crossing leaves the grass after about three seconds. Stop at four seconds in
    // the clearing and decline renewal. North/south expiry movement keeps the column for the
    // remaining south hold.
    pbf_move_left_joystick(context, {0, -1}, 4000ms, 100ms);
    force_repel_expiry(stream, context, RepelRenewalChoice::NO, true, true);
    pbf_move_left_joystick(context, {0, -1}, 4400ms, 100ms);
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 4);
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 5);
    // Repel is already inactive; continue through the cleared route to Sunyshore.
    pbf_move_left_joystick(context, {+1, 0}, 7400ms, 100ms); // 37+

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {+1, 0}, 7400ms, 100ms); // 37+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 3);
    pbf_move_left_joystick(context, {+1, 0}, 1400ms, 100ms); // 7+
    pbf_move_left_joystick(context, {0, +1}, 800ms, 100ms);  // 3+
    pbf_move_left_joystick(context, {+1, 0}, 5200ms, 100ms); // 26+
    pbf_move_left_joystick(context, {0, -1}, 400ms, 100ms);  // 1+

    use_HM(stream, context, "Rock Smash");

    context.wait_for_all_requests();

    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 2);

    {
        MarkDetector route_222_sailor_luther_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 10000ms, 100ms); // 5+
            },
            {{route_222_sailor_luther_fight}});
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
                stream.log("go_to_house_west_sunyshore_route_222_sailor_luther: black screen not detected!", COLOR_RED);
                return false;
            }else if (ret == 0){
                fight_trainer_using_moves(stream, context, stats, "route_222_sailor_luther", true);
            }
        }
    }

    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {+1, 0}, 2400ms, 100ms); // 12+
    pbf_move_left_joystick(context, {0, -1}, 1400ms, 100ms); // 6+
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 1);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_to_house_west_sunyshore: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_to_house_west_sunyshore: entered the house", COLOR_GREEN);
    }

    return true;
}

static bool
go_through_house_west_sunyshore(VideoStream& stream, ProControllerContext& context, AutoStoryOptions options){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_through_house_west_sunyshore: moving towards exit...");

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_house_south_veilstone: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_house_west_sunyshore: exited the gate", COLOR_GREEN);
    }

    // Flint interrupts immediately after the gate. Finish the arrival scene
    // before the next checkpoint attempts its save.
    wait_for_dialog(stream, context, "[08.07] Flint arrival", 30000ms);
    clear_dialog(stream, context, ClearDialogMode::STOP_OVERWORLD, 120, BUTTON_B, 6000ms);

    return true;
}

static bool go_to_sunyshore_lighthouse(VideoStream& stream, ProControllerContext& context, AutoStoryOptions options){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_to_sunyshore_lighthouse: going towards the lighthouse to talk to Volkner...");

    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {+1, 0}, 2000ms, 100ms); // 10+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 2);
    pbf_move_left_joystick(context, {0, +1}, 3000ms, 100ms); // 15+

    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 8);
    pbf_move_left_joystick(context, {0, -1}, 3400ms, 100ms); // 17+
    pbf_move_left_joystick(context, {+1, 0}, 7600ms, 100ms); // 38+
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
            stream.log("go_through_house_west_sunyshore: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_house_west_sunyshore: entered the lighthouse", COLOR_GREEN);
    }

    return true;
}

static bool
talk_to_volkner_at_the_lighthouse(VideoStream& stream, ProControllerContext& context, AutoStoryOptions options){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);
    context.wait_for_all_requests();
    stream.log("talk_to_volkner_at_the_lighthouse: Going up and talking to Volkner...");

    {
        BlackScreenFlashWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = wait_until(stream, context, std::chrono::seconds(30), {{black_screen}});
        if (ret < 0){
            stream.log("talk_to_volkner_at_the_lighthouse: black screen exiting elevator not detected!", COLOR_RED);
            return false;
        }else if (ret == 0){
            stream.log("talk_to_volkner_at_the_lighthouse: exited the elevator.", COLOR_GREEN);
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 3);
    pbf_move_left_joystick(context, {0, +1}, 1400ms, 100ms); // 7+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 1);
    // Finish the approach before a dialog callback can cancel queued movement.
    context.wait_for_all_requests();

    {
        ShortDialogWatcher talk_to_volkner(COLOR_RED);
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {+1, 0}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{talk_to_volkner}});
        if (ret == 0){
            stream.log("talk_to_volkner_at_the_lighthouse: Talking to Volkner.", COLOR_GREEN);
            clear_dialog(stream, context, ClearDialogMode::STOP_OVERWORLD, 120, BUTTON_B, 6000ms);
        }else if (ret < 0){
            stream.log("talk_to_volkner_at_the_lighthouse: Didn't detect Volkner dialog box!", COLOR_RED);
            return false;
        }
    }

    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms);  // 3+
    pbf_move_left_joystick(context, {0, -1}, 1400ms, 100ms); // 7+
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 4);
    context.wait_for_all_requests();
    // The return route clears the elevator building before visual correction.
    // Align its door column; the indoor camera cannot establish the player's
    // north/south distance reliably near the room edge. Watch entry below.
    align_to_overworld_landmarks(stream, context, match_sunyshore_lighthouse_elevator,
        "Sunyshore lighthouse elevator exit");

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("talk_to_volkner_at_the_lighthouse: entering elevator to exit black screen not detected!",
                COLOR_RED);
            return false;
        }
        stream.log("talk_to_volkner_at_the_lighthouse: entered the elevator", COLOR_GREEN);
    }

    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    {
        BlackScreenFlashWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = wait_until(stream, context, std::chrono::seconds(30), {{black_screen}});
        if (ret < 0){
            stream.log("talk_to_volkner_at_the_lighthouse: black screen exiting elevator not detected!", COLOR_RED);
            return false;
        }else if (ret == 0){
            stream.log("talk_to_volkner_at_the_lighthouse: exited the elevator.", COLOR_GREEN);
        }
    }

    return true;
}

static bool go_to_sunyshore_gym(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats, AutoStoryOptions options){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);
    context.wait_for_all_requests();
    stream.log("go_to_sunyshore_gym: heading towards Sunyshore City's gym...");

    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 14);
    pbf_move_left_joystick(context, {0, +1}, 4000ms, 100ms); // 20+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 4);
    pbf_move_left_joystick(context, {0, +1}, 5400ms, 100ms); // 27+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 6);
    pbf_move_left_joystick(context, {-1, 0}, 6800ms, 100ms); // 34+
    pbf_move_left_joystick(context, {0, +1}, 600ms, 100ms);  // 2+

    context.wait_for_all_requests();
    {
        ShortDialogWatcher talk_to_flint(COLOR_RED);
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {+1, 0}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{talk_to_flint}});
        if (ret == 0){
            stream.log("go_to_sunyshore_gym: Talking to Flint.", COLOR_GREEN);
            clear_dialog(stream, context, ClearDialogMode::STOP_OVERWORLD, 120, BUTTON_B, 6000ms);
        }else if (ret < 0){
            stream.log("go_to_sunyshore_gym: Didn't detect Flint dialog box!", COLOR_RED);
            return false;
        }
    }

    context.wait_for_all_requests();

    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 1);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_to_sunyshore_gym: black screen entering the gym not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_to_sunyshore_gym: entered the gym.", COLOR_GREEN);
    }

    // The gym entry tour takes about seven seconds after the entry fade. Let it finish before
    // sending puzzle inputs.
    pbf_wait(context, 8000ms);
    context.wait_for_all_requests();
    OverworldWatcher entrance_ready;
    if (wait_until(stream, context, 10s, {{entrance_ready}}) != 0){
        stream.log("[Sunyshore Gym puzzle] Entrance not ready after the entry cutscene.", COLOR_RED);
        return false;
    }

    // Counted walking steps use 80ms presses and 300ms releases.
    // Keep explicit waits for switch rotations, room transitions, and trainer approaches.
    struct Move {
        DpadPosition dir;
        int count;
        Milliseconds hold = 80ms;
        Milliseconds delay = 300ms;
        JoystickPosition stick{0, 0};
    };
    const std::vector<Move> sequences[] = {
        {
            // Sunyshore Gym School Kid Tiera
            {DPAD_UP, 1, 158ms, 300ms},
            {DPAD_UP, 5, 80ms, 300ms},
            {DPAD_RIGHT, 1, 144ms, 300ms},
            {DPAD_RIGHT, 4, 80ms, 300ms},
            {DPAD_RIGHT, 1, 159ms, 1909ms},
            // Turn south with a short tap, then walk three tiles to the lower platform.
            {DPAD_DOWN, 4, 80ms, 300ms},
            // Approach the bridge along the north rail. Give the northwest
            // hold time to cover the last two tiles to the opening.
            {DPAD_LEFT, 4, 80ms, 300ms},
            {DPAD_NONE, 1, 1050ms, 0ms, {-1, 1}},
            // Hold north into the elbow to absorb small timing differences.
            {DPAD_NONE, 1, 700ms, 300ms, {0, 1}},
            {DPAD_LEFT, 1, 179ms, 300ms},
            {DPAD_LEFT, 3, 80ms, 300ms},
            {DPAD_LEFT, 1, 144ms, 1898ms},
            {DPAD_RIGHT, 1, 145ms, 300ms},
            {DPAD_RIGHT, 5, 80ms, 300ms},
            {DPAD_UP, 1, 180ms, 300ms},
            {DPAD_UP, 2, 80ms, 300ms},
            {DPAD_UP, 1, 173ms, 2371ms},
        },
        {
            // Sunyshore Gym School Kid Forrest. Preserve the switch waits. Use a separate facing
            // tap when aligning west with the blue bridge lane.
            {DPAD_UP, 1, 127ms, 573ms},
            {DPAD_UP, 1, 127ms, 806ms},
            {DPAD_UP, 1, 127ms, 2232ms},
            {DPAD_UP, 1, 218ms, 458ms},
            {DPAD_UP, 1, 178ms, 481ms},
            // Face west, then walk three tiles into the blue bridge lane.
            {DPAD_LEFT, 4, 80ms, 300ms},
            {DPAD_UP, 1, 132ms, 585ms},
            {DPAD_UP, 1, 177ms, 456ms},
            {DPAD_UP, 1, 167ms, 455ms},
            {DPAD_UP, 1, 154ms, 480ms},
            {DPAD_UP, 1, 170ms, 602ms},
            {DPAD_UP, 1, 175ms, 650ms},
            {DPAD_UP, 1, 190ms, 1663ms},
            {DPAD_LEFT, 1, 192ms, 520ms},
            {DPAD_RIGHT, 1, 177ms, 1813ms},
            {DPAD_UP, 1, 193ms, 800ms},
            {DPAD_DOWN, 1, 216ms, 1417ms},
            {DPAD_RIGHT, 1, 185ms, 566ms},
            {DPAD_RIGHT, 1, 161ms, 554ms},
            {DPAD_RIGHT, 1, 151ms, 472ms},
            {DPAD_RIGHT, 1, 194ms, 433ms},
            {DPAD_RIGHT, 1, 127ms, 676ms},
            {DPAD_DOWN, 1, 157ms, 548ms},
            {DPAD_DOWN, 1, 172ms, 454ms},
            {DPAD_DOWN, 1, 169ms, 420ms},
            {DPAD_DOWN, 1, 179ms, 432ms},
            {DPAD_DOWN, 1, 179ms, 459ms},
            {DPAD_DOWN, 1, 164ms, 389ms},
            {DPAD_DOWN, 1, 177ms, 473ms},
            {DPAD_DOWN, 1, 193ms, 521ms},
            {DPAD_DOWN, 1, 166ms, 616ms},
            {DPAD_LEFT, 1, 181ms, 516ms},
            {DPAD_LEFT, 1, 184ms, 532ms},
            {DPAD_LEFT, 1, 145ms, 558ms},
            {DPAD_LEFT, 1, 198ms, 434ms},
            {DPAD_LEFT, 1, 166ms, 456ms},
            {DPAD_LEFT, 1, 155ms, 446ms},
            {DPAD_LEFT, 1, 171ms, 441ms},
            {DPAD_LEFT, 1, 159ms, 472ms},
            {DPAD_LEFT, 1, 167ms, 416ms},
            {DPAD_UP, 1, 196ms, 378ms},
            {DPAD_UP, 1, 171ms, 524ms},
            {DPAD_UP, 1, 181ms, 455ms},
            {DPAD_UP, 1, 190ms, 522ms},
            {DPAD_UP, 1, 194ms, 389ms},
            {DPAD_UP, 1, 183ms, 522ms},
            {DPAD_UP, 1, 190ms, 543ms},
            {DPAD_UP, 1, 189ms, 492ms},
            {DPAD_UP, 1, 211ms, 421ms},
            {DPAD_UP, 1, 233ms, 501ms},
            {DPAD_UP, 1, 232ms, 417ms},
            {DPAD_UP, 1, 211ms, 442ms},
            {DPAD_UP, 1, 209ms, 404ms},
            {DPAD_UP, 1, 198ms, 471ms},
            {DPAD_UP, 1, 195ms, 421ms},
            {DPAD_UP, 1, 211ms, 544ms},
            {DPAD_UP, 1, 210ms, 618ms},
            {DPAD_RIGHT, 1, 159ms, 457ms},
            {DPAD_RIGHT, 1, 166ms, 589ms},
            {DPAD_RIGHT, 1, 173ms, 1642ms},
        },
        {
            // Sunyshore Gym Guitarist Jerry
            // Slide southeast along the raised platform wall into the blue bridge.
            {DPAD_NONE, 1, 850ms, 0ms, {1, -1}},
            // The south end stops the hold at the switch. Let the bridge rotate.
            {DPAD_NONE, 1, 700ms, 1473ms, {0, -1}},
            // Hold east into the end rail to reach the green switch despite missed taps.
            // Let it rotate before the watched southeast approach to Jerry.
            {DPAD_NONE, 1, 1500ms, 1405ms, {1, 0}},
        },
        {
            // Sunyshore Gym Poke Kid Meghan
            // Let each step finish so the south/east turn reaches the foot of the stairs.
            {DPAD_DOWN, 3, 80ms, 300ms},
            {DPAD_RIGHT, 2, 80ms, 300ms},
            // Walk north into the wall, then turn and step south once.
            {DPAD_UP, 17, 80ms, 300ms},
            {DPAD_DOWN, 2, 80ms, 300ms},
            {DPAD_LEFT, 3, 80ms, 300ms},
        },
        {
            // Sunyshore Gym Guitarist Lonnie
            // Let each step finish; 100ms releases stop short of the doorway.
            // Turn and step north behind Meghan, then turn and walk four tiles west to the doorway.
            {DPAD_UP, 2, 80ms, 300ms},
            {DPAD_LEFT, 5, 80ms, 300ms},
            {DPAD_UP, 1, 198ms, 1975ms},
            {DPAD_UP, 1, 80ms, 300ms},
            {DPAD_UP, 1, 178ms, 1114ms},
            {DPAD_UP, 5, 80ms, 300ms},
            // Turn east, walk five tiles onto the red switch, and wait for rotation.
            {DPAD_RIGHT, 5, 80ms, 300ms},
            {DPAD_RIGHT, 1, 80ms, 2178ms},
        },
        {
            // Sunyshore Gym Ace Trainer Destiny
            // Move west of Lonnie's bridge before approaching the next opening.
            {DPAD_LEFT, 5, 80ms, 300ms},
            // Settle against the north edge east of the bridge before sliding into it.
            // Lonnie can engage one row farther south; starting there catches the corner.
            {DPAD_UP, 4, 80ms, 300ms},
            // Drag northwest along the platform edge and bridge rails to the red switch.
            // Allow for slower wall sliding, then wait for the switch rotation.
            {DPAD_NONE, 1, 6000ms, 2790ms, {-1, 1}},
            {DPAD_DOWN, 5, 80ms, 300ms},
            {DPAD_DOWN, 1, 210ms, 1870ms},
        },
        {
            // Sunyshore Gym Guitarist Preston
            {DPAD_DOWN, 4},
            // Turn west and walk four tiles to align with the bridge above the stairs.
            {DPAD_LEFT, 5},
            {DPAD_UP, 21},
            {DPAD_RIGHT, 1, 181ms, 1419ms},
        },
        {
            // Sunyshore Gym Ace Trainer Zachery
            // Reach the east edge of Preston's platform to align with the south bridge.
            {DPAD_RIGHT, 6},
            {DPAD_DOWN, 4},
            {DPAD_DOWN, 1, 169ms, 2139ms},
            // Activate the green switch twice total. Step south, turn north,
            // then step back onto it and wait for the second rotation.
            {DPAD_DOWN, 1},
            {DPAD_UP, 1},
            {DPAD_UP, 1, 80ms, 1824ms},
            // Preserve walking distances and gear-rotation waits. Add one facing press at each
            // turn.
            {DPAD_DOWN, 6},
            {DPAD_RIGHT, 6},
            {DPAD_UP, 6},
            {DPAD_RIGHT, 5},
            {DPAD_RIGHT, 1, 80ms, 1550ms},
            {DPAD_DOWN, 5},
            {DPAD_DOWN, 1, 80ms, 1588ms},
            {DPAD_UP, 9},
            {DPAD_RIGHT, 5},
            {DPAD_DOWN, 5},
            {DPAD_DOWN, 1, 80ms, 1388ms},
        },
        {
            // Volkner. Include a facing press at each turn so the 80ms taps preserve walking
            // distances and clear the stairs before turning west.
            {DPAD_DOWN, 16},
            {DPAD_LEFT, 4},
            {DPAD_UP, 5},
            {DPAD_LEFT, 2},
            {DPAD_UP, 3},
            // Activate the right red switch once and wait for rotation.
            // Return south directly; the left red switch is not needed.
            {DPAD_UP, 1, 80ms, 2482ms},
            {DPAD_DOWN, 4},
            {DPAD_RIGHT, 3},
            {DPAD_DOWN, 5},
            {DPAD_RIGHT, 3},
            {DPAD_UP, 21},
            {DPAD_LEFT, 5},
            {DPAD_DOWN, 4},
            {DPAD_LEFT, 6},
            {DPAD_UP, 4},
            {DPAD_UP, 1, 80ms, 1910ms},
        },
    };
    const char* trainers[] = {
        "Sunyshore Gym School Kid Tiera",
        "Sunyshore Gym School Kid Forrest",
        "Sunyshore Gym Guitarist Jerry",
        "Sunyshore Gym Poke Kid Meghan",
        "Sunyshore Gym Guitarist Lonnie",
        "Sunyshore Gym Ace Trainer Destiny",
        "Sunyshore Gym Guitarist Preston",
        "Sunyshore Gym Ace Trainer Zachery",
        "Volkner",
    };
    for (size_t stage = 0; stage < 9; stage++){
        stream.log("[Sunyshore Gym puzzle] Approaching " + std::string(trainers[stage]) + ".");
        for (const Move& move : sequences[stage]){
            if (move.dir == DPAD_NONE){
                pbf_move_left_joystick(context, move.stick, move.hold, move.delay);
                continue;
            }
            for (int press = 0; press < move.count; press++){
                pbf_press_dpad(context, move.dir, move.hold, move.delay);
            }
        }
        context.wait_for_all_requests();

        BlackScreenWatcher battle_fade(COLOR_RED);
        StartBattleDetector battle_intro(stream.overlay());
        BattleMenuWatcher battle_menu(BattleType::TRAINER);
        const int ret = run_until<ProControllerContext>(stream,
            context,
            [stage](ProControllerContext& ctx){
                if (stage == 2){
                    // Slide down the bridge's east rail and through its side opening.
                    // Continue across the side bridge to Jerry's platform; stop on battle entry.
                    pbf_move_left_joystick(ctx, {1, -1}, 6000ms, 0ms);
                    pbf_mash_button(ctx, BUTTON_A, 30000ms);
                }else if (stage == 4){
                    // Lonnie may face away. Walk south and interact directly,
                    // stopping at the transition even if his sight line starts the battle.
                    for (size_t attempt = 0; attempt < 60; attempt++){
                        ssf_press_left_joystick(ctx, {0, -1}, 0ms, 500ms);
                        ssf_mash1_button(ctx, BUTTON_A, 500ms);
                        pbf_wait(ctx, 100ms);
                    }
                }else{
                    pbf_mash_button(ctx, BUTTON_A, 30000ms);
                }
            },
            {{battle_fade}, {battle_intro}, {battle_menu}});
        if (ret < 0){
            stream.log("[Sunyshore Gym puzzle] Battle start not detected for " + std::string(trainers[stage]) + ".", COLOR_RED);
            return false;
        }
        // The move-list handler uses B while waiting for the command menu.
        fight_trainer_using_moves(stream, context, stats, trainers[stage], true);
        clear_dialog(stream, context, BUTTON_B, stage == 8 ? 6000ms : 3000ms);
    }

    // Include the facing press when turning away from Volkner and at each bridge turn.
    const Move return_to_stairs[] = {
        {DPAD_DOWN, 4},
        {DPAD_RIGHT, 6},
        {DPAD_UP, 4},
        {DPAD_RIGHT, 5},
        {DPAD_DOWN, 11},
        {DPAD_DOWN, 1, 80ms, 1291ms},
    };
    // The return crosses two internal doorways before the outdoor Gym exit.
    // Finish each transition before sending movement for the next room.
    const char* exit_destinations[] = {
        "middle puzzle room",
        "entrance puzzle room",
        "outside the Gym",
    };
    for (size_t room = 0; room < 3; room++){
        const std::string destination = exit_destinations[room];
        stream.log("[Sunyshore Gym puzzle] Returning to " + destination + ".");
        BlackScreenWatcher exit_black(COLOR_RED);
        const int exit_ret = run_until<ProControllerContext>(stream,
            context,
            [&](ProControllerContext& ctx){
                if (room == 0){
                    for (const Move& move : return_to_stairs){
                        for (int press = 0; press < move.count; press++){
                            pbf_press_dpad(ctx, move.dir, move.hold, move.delay);
                        }
                    }
                    pbf_move_left_joystick(ctx, {0.000000, -1.000000}, 1942ms, 0ms);
                    pbf_wait(ctx, 271ms);
                    pbf_move_left_joystick(ctx, {-1.000000, -0.000000}, 685ms, 0ms);
                    pbf_move_left_joystick(ctx, {-1.000000, -1.000000}, 2742ms, 0ms);
                }else if (room == 1){
                    pbf_move_left_joystick(ctx, {0.000000, -1.000000}, 833ms, 0ms);
                    pbf_move_left_joystick(ctx, {1.000000, -1.000000}, 4576ms, 0ms);
                    pbf_move_left_joystick(ctx, {0.000000, -1.000000}, 1965ms, 0ms);
                    pbf_move_left_joystick(ctx, {-1.000000, -1.000000}, 105ms, 0ms);
                    pbf_move_left_joystick(ctx, {-1.000000, -0.000000}, 1638ms, 0ms);
                    pbf_move_left_joystick(ctx, {-1.000000, 1.000000}, 1765ms, 0ms);
                    pbf_move_left_joystick(ctx, {-1.000000, -0.000000}, 111ms, 0ms);
                    pbf_wait(ctx, 392ms);
                    pbf_move_left_joystick(ctx, {0.000000, -1.000000}, 780ms, 0ms);
                    pbf_move_left_joystick(ctx, {-1.000000, -1.000000}, 918ms, 0ms);
                    pbf_move_left_joystick(ctx, {0.000000, -1.000000}, 1330ms, 0ms);
                    pbf_move_left_joystick(ctx, {-1.000000, -1.000000}, 1469ms, 0ms);
                }else{
                    pbf_move_left_joystick(ctx, {0.000000, -1.000000}, 3935ms, 0ms);
                    pbf_wait(ctx, 252ms);
                    pbf_move_left_joystick(ctx, {-1.000000, -0.000000}, 1465ms, 0ms);
                    pbf_wait(ctx, 1470ms);
                    pbf_move_left_joystick(ctx, {-1.000000, -0.000000}, 885ms, 0ms);
                    pbf_wait(ctx, 116ms);
                    pbf_move_left_joystick(ctx, {1.000000, -0.000000}, 1112ms, 0ms);
                    pbf_wait(ctx, 921ms);
                    pbf_move_left_joystick(ctx, {-1.000000, -0.000000}, 1139ms, 0ms);
                    pbf_wait(ctx, 91ms);
                    pbf_move_left_joystick(ctx, {1.000000, -0.000000}, 857ms, 0ms);
                    pbf_wait(ctx, 1141ms);
                    pbf_move_left_joystick(ctx, {1.000000, -0.000000}, 3093ms, 0ms);
                    pbf_wait(ctx, 75ms);
                    pbf_move_left_joystick(ctx, {0.000000, -1.000000}, 1725ms, 0ms);
                }
                pbf_wait(ctx, 5000ms);
            },
            {{exit_black}});
        if (exit_ret != 0){
            stream.log("[Sunyshore Gym puzzle] Transition to " + destination + " did not start.", COLOR_RED);
            return false;
        }
        BlackScreenWatcher visible(COLOR_RED, {0.1, 0.1, 0.8, 0.8}, 100, 10,
            BlackScreenWatcher::FinderType::GONE, 500ms);
        if (wait_until(stream, context, 30s, {{visible}}) != 0){
            stream.log("[Sunyshore Gym puzzle] Transition to " + destination + " did not finish.", COLOR_RED);
            return false;
        }
        context.wait_for_all_requests();
        OverworldWatcher ready;
        if (wait_until(stream, context, 10s, {{ready}}) != 0){
            stream.log("[Sunyshore Gym puzzle] Overworld not ready in " + destination + ".", COLOR_RED);
            return false;
        }
    }
    stream.log("[Sunyshore Gym puzzle] Rewards finished; outside the Gym doorway for 09.01.", COLOR_GREEN);
    return true;
}

// ---------------------------------------------------------------------------
//  Segment wrappers
// ---------------------------------------------------------------------------

std::string AutoStory_Segment_08::name() const {
    return "08: Snowpoint to Sunyshore";
}

std::string AutoStory_Segment_08::start_text() const {
    return AutoStory_Checkpoint_0801().start_text();
}

std::string AutoStory_Segment_08::end_text() const {
    return AutoStory_Checkpoint_0809().end_text();
}

void AutoStory_Segment_08::run_segment(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);
    AutoStory_Checkpoint_0801().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0802().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0803().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0804().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0805().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0806().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0807().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0808().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0809().run_checkpoint(env, context, options, stats);
    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}

// ---------------------------------------------------------------------------
//  Checkpoint wrappers
// ---------------------------------------------------------------------------

std::string AutoStory_Checkpoint_0801::name() const {
    return "08.01 - Snowpoint recovery";
}

std::string AutoStory_Checkpoint_0801::start_text() const {
    return "Outside Snowpoint Gym at the doorway, facing south, with seventh-badge rewards and dialogue finished, Repel inactive.";
}

std::string AutoStory_Checkpoint_0801::end_text() const {
    return "Healed and outside Snowpoint Pokemon Center at the doorway.";
}

void AutoStory_Checkpoint_0801::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0801(env, context, options, stats);
}

void checkpoint_0801(SingleSwitchProgramEnvironment& env,
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
                if (!heal_at_snowpoint_city_pokemon_center(env.console, context)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "heal_at_snowpoint_city_pokemon_center: transition not detected.",
                        env.console);
                }
            }
            {
                if (!heal_and_exit(env.console, context, stats, "Snowpoint City")){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "heal_and_exit_snowpoint_city: transition not detected.",
                        env.console);
                }
            }
            context.wait_for_all_requests();
        },
        true);
}

std::string AutoStory_Checkpoint_0802::name() const {
    return "08.02 - Lake Acuity and Veilstone";
}

std::string AutoStory_Checkpoint_0802::start_text() const {
    return "Healed and outside Snowpoint Pokemon Center at the doorway.";
}

std::string AutoStory_Checkpoint_0802::end_text() const {
    return "Healed and outside Veilstone Pokemon Center after the Lake Acuity events, Repel inactive.";
}

void AutoStory_Checkpoint_0802::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0802(env, context, options, stats);
}

void checkpoint_0802(SingleSwitchProgramEnvironment& env,
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
                if (!go_to_lake_acuity(env.console, context)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "go_to_lake_acuity: transition not detected.",
                        env.console);
                }
            }
            {
                if (!talk_to_clint_then_go_veilstone(env.console, context)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "talk_to_clint_then_go_veilstone: transition not detected.",
                        env.console);
                }
            }
            {
                if (!heal_and_exit(env.console, context, stats, "Veilstone City")){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "heal_and_exit_veilstone_city: transition not detected.",
                        env.console);
                }
            }
            context.wait_for_all_requests();
        },
        true);
}

std::string AutoStory_Checkpoint_0803::name() const {
    return "08.03 - Galactic Warehouse";
}

std::string AutoStory_Checkpoint_0803::start_text() const {
    return "Healed and outside Veilstone Pokemon Center after the Lake Acuity events, Repel inactive.";
}

std::string AutoStory_Checkpoint_0803::end_text() const {
    return "Healed and outside Veilstone Pokemon Center with the Galactic Key obtained.";
}

void AutoStory_Checkpoint_0803::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0803(env, context, options, stats);
}

void checkpoint_0803(SingleSwitchProgramEnvironment& env,
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
                if (!get_key_and_enter_warehouse(env.console, context)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "get_key_and_enter_warehouse: transition not detected.",
                        env.console);
                }
            }
            {
                if (!go_through_warehouse(env.console, context, stats)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "go_through_warehouse: transition not detected.",
                        env.console);
                }
            }
            {
                if (!heal_at_veilstone_pokemon_center(env.console, context)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "heal_at_veilstone_pokemon_center: transition not detected.",
                        env.console);
                }
            }
            {
                if (!heal_and_exit(env.console, context, stats, "Veilstone City")){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "heal_and_exit_veilstone_city: transition not detected.",
                        env.console);
                }
            }
            context.wait_for_all_requests();
        },
        true);
}

std::string AutoStory_Checkpoint_0804::name() const {
    return "08.04 - Galactic HQ and Eterna";
}

std::string AutoStory_Checkpoint_0804::start_text() const {
    return "Healed and outside Veilstone Pokemon Center with the Galactic Key obtained.";
}

std::string AutoStory_Checkpoint_0804::end_text() const {
    return "Healed and outside Eterna Pokemon Center after the Galactic HQ events.";
}

void AutoStory_Checkpoint_0804::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0804(env, context, options, stats);
}

void checkpoint_0804(SingleSwitchProgramEnvironment& env,
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
                if (!go_to_galactic_hq(env.console, context)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "go_to_galactic_hq: transition not detected.",
                        env.console);
                }
            }
            {
                if (!go_through_galactic_hq(env.console, context, stats, options)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "go_through_galactic_hq: transition not detected.",
                        env.console);
                }
            }
            {
                if (!fly_and_heal_at_eterna(env.console, context, options)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "fly_and_heal_at_eterna: transition not detected.",
                        env.console);
                }
            }
            {
                if (!heal_and_exit(env.console, context, stats, "Eterna City")){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "heal_and_exit_eterna_city: transition not detected.",
                        env.console);
                }
            }
            context.wait_for_all_requests();
        },
        true);
}

std::string AutoStory_Checkpoint_0805::name() const {
    return "08.05 - Eterna to Spear Pillar";
}

std::string AutoStory_Checkpoint_0805::start_text() const {
    return "Healed and outside Eterna Pokemon Center after the Galactic HQ events.";
}

std::string AutoStory_Checkpoint_0805::end_text() const {
    return "Outside the Spear Pillar entrance doorway, before approaching the summit grunts.";
}

void AutoStory_Checkpoint_0805::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0805(env, context, options, stats);
}

void checkpoint_0805(SingleSwitchProgramEnvironment& env,
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
                const bool reloaded = stats.game_reset_generation != stats.coronet_attempt_reset_generation;
                if (reloaded && stats.coronet_approach_phase == CoronetApproachPhase::BEFORE_CYCLING_ROAD){
                    stats.coronet_toggle_bike_gear = !stats.coronet_toggle_bike_gear;
                    env.console.log(stats.coronet_toggle_bike_gear
                        ? "[08.05] Reset before Cycling Road: retry with the mounted B gear toggle."
                        : "[08.05] Reset before Cycling Road: retry with the saved bicycle gear.");
                    pbf_wait(context, 500ms);
                    context.wait_for_all_requests();
                }
                stats.coronet_attempt_reset_generation = stats.game_reset_generation;
                stats.coronet_approach_phase = CoronetApproachPhase::BEFORE_CYCLING_ROAD;
                if (!go_to_mount_coronet(env.console, context, stats, options)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "go_to_mount_coronet: transition not detected.",
                        env.console);
                }
            }
            {
                if (!go_through_mount_coronet(env.console, context, stats, options)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "go_through_mount_coronet: transition not detected.",
                        env.console);
                }
            }
            context.wait_for_all_requests();
        },
        true);
}

std::string AutoStory_Checkpoint_0806::name() const {
    return "08.06 - Spear Pillar and Veilstone";
}

std::string AutoStory_Checkpoint_0806::start_text() const {
    return "Outside the Spear Pillar entrance doorway, before approaching the summit grunts.";
}

std::string AutoStory_Checkpoint_0806::end_text() const {
    return "Summit story finished; legendary caught with the Master Ball or fled according to the option; healed outside Veilstone Center.";
}

void AutoStory_Checkpoint_0806::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0806(env, context, options, stats);
}

void checkpoint_0806(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            if (!go_through_mount_coronet(env.console, context, stats, options, true)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "08.06: Summit story did not complete.", env.console);
            }
            // The summit route handles the capture option and returns after the Escape Rope.
            {
                if (!fly_and_heal_at_veilstone(env.console, context, options)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "fly_and_heal_at_veilstone: transition not detected.",
                        env.console);
                }
            }
            {
                if (!heal_and_exit(env.console, context, stats, "Veilstone City")){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "heal_and_exit_veilstone_city: transition not detected.",
                        env.console);
                }
            }
            context.wait_for_all_requests();
        },
        true);
}

std::string AutoStory_Checkpoint_0807::name() const {
    return "08.07 - Route 222 to Sunyshore";
}

std::string AutoStory_Checkpoint_0807::start_text() const {
    return "Healed and outside Veilstone Pokemon Center after completing the summit story.";
}

std::string AutoStory_Checkpoint_0807::end_text() const {
    return "In Sunyshore after the west gate and arrival dialogue, Repel inactive.";
}

void AutoStory_Checkpoint_0807::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0807(env, context, options, stats);
}

void checkpoint_0807(SingleSwitchProgramEnvironment& env,
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
                if (!go_to_house_south_veilstone(env.console, context, options)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "go_to_house_south_veilstone: transition not detected.",
                        env.console);
                }
            }
            {
                if (!go_through_house_south_veilstone(env.console, context, options)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "go_through_house_south_veilstone: transition not detected.",
                        env.console);
                }
            }
            {
                if (!go_to_house_west_sunyshore(env.console, context, stats, options)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "go_to_house_west_sunyshore: transition not detected.",
                        env.console);
                }
            }
            {
                if (!go_through_house_west_sunyshore(env.console, context, options)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "go_through_house_west_sunyshore: transition not detected.",
                        env.console);
                }
            }
            context.wait_for_all_requests();
        },
        true);
}

std::string AutoStory_Checkpoint_0808::name() const {
    return "08.08 - Volkner at the lighthouse";
}

std::string AutoStory_Checkpoint_0808::start_text() const {
    return "In Sunyshore after the west gate and arrival dialogue, Repel inactive.";
}

std::string AutoStory_Checkpoint_0808::end_text() const {
    return "Outside Sunyshore lighthouse after Volkner dialogue.";
}

void AutoStory_Checkpoint_0808::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0808(env, context, options, stats);
}

void checkpoint_0808(SingleSwitchProgramEnvironment& env,
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
                if (!go_to_sunyshore_lighthouse(env.console, context, options)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "go_to_sunyshore_lighthouse: transition not detected.",
                        env.console);
                }
            }
            {
                if (!talk_to_volkner_at_the_lighthouse(env.console, context, options)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "talk_to_volkner_at_the_lighthouse: transition not detected.",
                        env.console);
                }
            }
            context.wait_for_all_requests();
        },
        true);
}

std::string AutoStory_Checkpoint_0809::name() const {
    return "08.09 - Sunyshore Gym";
}

std::string AutoStory_Checkpoint_0809::start_text() const {
    return "Outside Sunyshore lighthouse after Volkner dialogue.";
}

std::string AutoStory_Checkpoint_0809::end_text() const {
    return "Outside Sunyshore Gym at the doorway, facing south, with eighth-badge rewards and dialogue finished.";
}

void AutoStory_Checkpoint_0809::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0809(env, context, options, stats);
}

void checkpoint_0809(SingleSwitchProgramEnvironment& env,
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
                if (!go_to_sunyshore_gym(env.console, context, stats, options)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "go_to_sunyshore_gym: transition not detected.",
                        env.console);
                }
            }
            context.wait_for_all_requests();
        },
        true);
}

}
}
}
