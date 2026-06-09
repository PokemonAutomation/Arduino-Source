/*  BDSP AutoStory - Segment 08
 *  From after acquiring the seventh badge until after acquiring the eighth badge.
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
#include "../PokemonBDSP_AutoStory.h"
#include "../PokemonBDSP_AutoStoryTools.h"
#include "../Utils/PokemonBDSP_AutoStory_Battle.h"
#include "PokemonBDSP_AutoStory_Segment_08.h"

using namespace std::chrono_literals;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


// ---------------------------------------------------------------------------
//  Legendary encounter (from Routes/Legendary_Segment.cpp)
// ---------------------------------------------------------------------------

static void approach_legendary(VideoStream& /*stream*/, ProControllerContext& context){
    pbf_wait(context, 2000ms);
    pbf_press_button(context, BUTTON_A, 100ms, 300ms);
    pbf_wait(context, 2000ms);
}

static void enter_legendary_battle(VideoStream& /*stream*/, ProControllerContext& context){
    pbf_press_button(context, BUTTON_A, 100ms, 100ms);
    pbf_wait(context, 3000ms);
}

static void weaken_legendary(VideoStream& /*stream*/, ProControllerContext& context){
    pbf_press_button(context, BUTTON_A, 100ms, 100ms);
    pbf_wait(context, 1500ms);
    pbf_press_dpad(context, DPAD_UP, 100ms, 100ms);
    pbf_press_button(context, BUTTON_A, 100ms, 100ms);
    pbf_wait(context, 2000ms);
}

static void throw_pokeball_at_legendary(VideoStream& /*stream*/, ProControllerContext& context){
    for (int i = 0; i < 3; i++){
        pbf_press_button(context, BUTTON_A, 100ms, 100ms);
        pbf_wait(context, 1000ms);
        pbf_press_dpad(context, DPAD_DOWN, 100ms, 100ms);
        pbf_press_button(context, BUTTON_A, 100ms, 100ms);
        pbf_wait(context, 2500ms);
    }
}

static void confirm_legendary_catch(VideoStream& /*stream*/, ProControllerContext& context){
    pbf_wait(context, 1500ms);
    pbf_press_button(context, BUTTON_A, 100ms, 100ms);
    pbf_wait(context, 2000ms);
}

static void exit_legendary_encounter(VideoStream& /*stream*/, ProControllerContext& context){
    pbf_press_button(context, BUTTON_A, 100ms, 100ms);
    pbf_wait(context, 2000ms);
}


// ---------------------------------------------------------------------------
//  Segment / Checkpoint classes
// ---------------------------------------------------------------------------

std::string AutoStory_Segment_08::name()       const{ return "08: Seventh Badge to Eighth Badge"; }
std::string AutoStory_Segment_08::start_text() const{ return "Start: Seventh Gym Badge acquired (Snowpoint City)."; }
std::string AutoStory_Segment_08::end_text()   const{ return "End: Eighth Gym Badge acquired (Sunyshore City)."; }

void AutoStory_Segment_08::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_111().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_112().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_113().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_114().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_115().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_116().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_117().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_118().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_119().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_120().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_121().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_122().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_123().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_124().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_125().run_checkpoint(env, context, options, stats);

    AutoStory_Checkpoint_126().run_checkpoint(env, context, options, stats);

    AutoStory_Checkpoint_127().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_128().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_129().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_130().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_131().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_132().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_133().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_134().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_135().run_checkpoint(env, context, options, stats);

    /*AutoStory_Checkpoint_021().run_checkpoint(env, context, options, stats);
    checkpoint_021_legendary(env, context, options, stats);*/

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}


std::string AutoStory_Checkpoint_021::name()       const{ return "021 - " + AutoStory_Segment_08().name(); }
std::string AutoStory_Checkpoint_021::start_text() const{ return "Seventh Gym Badge acquired (Snowpoint City)."; }
std::string AutoStory_Checkpoint_021::end_text()   const{ return "Eighth Gym Badge acquired (Sunyshore City)."; }


// ------------------------------------------------------------------ Functions logic --------------------------------------------------------- //


static void move_back_after_unexpected_battle(
    VideoStream& stream,
    ProControllerContext& context,
    const std::string& trainerid
){
    stream.log("Finishing battle with " + trainerid, COLOR_GREEN);
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    if (trainerid == "veilstone_warehouse_warehouse_grunt_1") {
        pbf_move_left_joystick(context, {0, +1}, 1200ms, 100ms); // 5+
    } else if (trainerid == "galactic_hq_grunt_1") {
        pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms); // 3+
    } else if (trainerid == "route_207_hiker_justin") {
        pbf_move_left_joystick(context, {+1, 0}, 800ms, 100ms); // 3+
    }
}

static bool handle_battle(
    VideoStream& stream,
    ProControllerContext& context,
    const std::string& trainerid
){
    context.wait_for_all_requests();
    stream.log("Battling " + trainerid + "...", COLOR_GREEN);
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
        trainerid == "veilstone_warehouse_warehouse_grunt_2" or
        trainerid == "galactic_hq_grunt_4" or
        trainerid == "galactic_hq_grunt_5"
    ) {
        // Select Bulldoze
        pbf_press_dpad(context, DPAD_DOWN, 280ms, 200ms);
    } else if (
        trainerid == "galactic_hq_grunt_1" or
        trainerid == "galactic_hq_double_grunts_1" or
        trainerid == "route_222_sailor_luther"
    ) {
        // Select Razor leaf
        pbf_press_dpad(context, DPAD_UP, 280ms, 200ms);
    } else if (
        trainerid == "veilstone_warehouse_warehouse_grunt_1" or
        trainerid == "galactic_hq_scientist_fredrick" or
        trainerid == "galactic_hq_grunt_2" or
        trainerid == "galactic_hq_grunt_3" or
        trainerid == "galactic_hq_cyrus" or
        trainerid == "galactic_hq_commander_saturn" or
        trainerid == "route_207_hiker_justin"
    ) {
        // Select Crunch
    }
    context.wait_for_all_requests();

    {
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
    }
    
    context.wait_for_all_requests();
    pbf_mash_button(context, BUTTON_B, 2000ms);
    stream.log("handle_battle_" + trainerid + ": transition confirmed.");
    return true;
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

static bool heal_at_snowpoint_city_pokemon_center(
    VideoStream& stream,
    ProControllerContext& context
){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("heal_at_snowpoint_city_pokemon_center: flying towards the pokemon center...");

    fly_to(stream, context, FlyPoint::CurrentCity);

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

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
            stream.log("heal_at_snowpoint_city_pokemon_center: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("heal_at_snowpoint_city_pokemon_center: entered Pokemon Center.", COLOR_GREEN);
    };
    return true;
}

static bool heal_and_exit(
    VideoStream& stream,
    ProControllerContext& context,
    const std::string& label
){
    return heal_pokemon(stream, context, label);
}

static bool go_to_lake_acuity(
    VideoStream& stream,
    ProControllerContext& context
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("heal_at_snowpoint_city_pokemon_center: flying towards the pokemon center...");

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
    activate_repel(stream, context);

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {-1, 0}, 2400ms, 100ms); // 12+
    pbf_move_left_joystick(context, {0, -1}, 600ms, 100ms); // 2+
    pbf_move_left_joystick(context, {-1, 0}, 8400ms, 100ms); // 28++ (snow)
    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 6);

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
            stream.log("go_to_lake_acuity: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_to_lake_acuity: entered Acuity lakefront.", COLOR_GREEN);
    };
    return true;
}

static bool talk_to_clint_then_go_veilstone(
    VideoStream& stream,
    ProControllerContext& context
){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("talk_to_clint_then_go_veilstone: first talking to Clint and then flying to Veilstone city...");

    {
        ShortDialogWatcher listen_to_clint_and_jupiter(COLOR_BLUE);
        int ret = wait_until(stream, context, std::chrono::seconds(30), {{listen_to_clint_and_jupiter}});
        if (ret < 0){
            stream.log("talk_to_clint_then_go_veilstone_listen_to_clint_and_jupiter: dialog box not detected!", COLOR_RED);
            return false;
        } else if (ret == 0) {
            stream.log("talk_to_clint_then_go_veilstone_listen_to_clint_and_jupiter: dialog box detected.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_B, 22000ms);
            context.wait_for_all_requests();
        }
    }

    fly_to(stream, context, FlyPoint::VeilstoneCity);

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

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
            stream.log("talk_to_clint_then_go_veilstone: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("talk_to_clint_then_go_veilstone: entered Veilstone City Pokemon Center.", COLOR_GREEN);
    };
    return true;
}


static bool get_key_and_enter_warehouse(
    VideoStream& stream,
    ProControllerContext& context
){
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

    {
        ShortDialogWatcher repel_dialog(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 4600ms, 100ms); // 23+
            },
            {{repel_dialog}}
        );
        if (ret == 0){
            stream.log("go_through_mount_coronet: Not reactivating repel.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_B, 3000ms);
        } else if (ret < 0){
                stream.log("go_through_mount_coronet: Didn't detect repel dialog box!", COLOR_RED);
                return false;
        }
    }

    pbf_move_left_joystick(context, {0, +1}, 4600ms, 100ms); // 23+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 3);

    {
        ShortDialogWatcher talk_to_grunt(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {0, +1}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{talk_to_grunt}}
        );
        if (ret == 0){
            stream.log("get_key_and_enter_warehouse: Talking to the grunt.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_B, 9000ms);
        } else if (ret < 0){
                stream.log("get_key_and_enter_warehouse: Didn't detect grunt dialog box!", COLOR_RED);
                return false;
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    {
        ShortDialogWatcher pick_up_key(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {0, +1}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{pick_up_key}}
        );
        if (ret == 0){
            stream.log("get_key_and_enter_warehouse: Got the key.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 5000ms);
        } else if (ret < 0){
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("get_key_and_enter_warehouse: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("get_key_and_enter_warehouse: entered warehouse.", COLOR_GREEN);
    };
    return true;
}

static bool go_through_warehouse(
    VideoStream& stream,
    ProControllerContext& context
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_through_warehouse: Going through the warehouse...");

    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 1);

    {
        ShortDialogWatcher open_door(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {0, +1}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{open_door}}
        );
        if (ret == 0){
            stream.log("go_through_warehouse: Opened the door.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 4000ms);
        } else if (ret < 0){
                stream.log("go_through_warehouse: Didn't detect door dialog box!", COLOR_RED);
                return false;
        }
    }

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, +1}, 1000ms, 100ms); // 4+

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

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 1200ms, 100ms); // 5+
                pbf_move_left_joystick(context, {0, -1}, 400ms, 100ms); // 2+
                pbf_move_left_joystick(context, {+1, 0}, 10800ms, 100ms); // 54+
            },
            {{warehouse_grunt_1_fight}}
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
                stream.log("veilstone_warehouse_warehouse_grunt_1: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "veilstone_warehouse_warehouse_grunt_1");
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

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 13000ms, 100ms); // 54+
            },
            {{warehouse_grunt_2_fight}}
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
                stream.log("veilstone_warehouse_warehouse_grunt_2: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "veilstone_warehouse_warehouse_grunt_2");
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
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

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 2200ms, 100ms); // 11+
                pbf_move_left_joystick(context, {0, +1}, 1200ms, 100ms); // 6
            },
            {{black_screen},{galactic_hq_grunt_1_fight}}
        );
        if (ret == 0) {
            stream.log("go_through_warehouse: got into Galactic HQ 1F, second room.", COLOR_GREEN);
        } else if (ret == 1){
            context.wait_for_all_requests();
            pbf_wait(context, 2000ms);

            BlackScreenOverWatcher black_screen_2(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            ret = run_until<ProControllerContext>(
                stream, context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 200000ms);
                },
                {{black_screen_2}}
            );
            if (ret < 0){
                stream.log("galactic_hq_grunt_1: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "galactic_hq_grunt_1");
                context.wait_for_all_requests();
                pbf_wait(context, 3000ms);
                move_back_after_unexpected_battle(stream, context, "galactic_hq_grunt_1");
            }

            context.wait_for_all_requests();
            pbf_wait(context, 1000ms);

            {
                BlackScreenFlashWatcher black_screen(COLOR_BLUE, {0.1, 0.1, 0.8, 0.8});
                int ret = run_until<ProControllerContext>(
                    stream, context,
                    [](ProControllerContext& context){
                        pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
                    },
                    {{black_screen}}
                );
                if (ret < 0){
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, -1}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("go_through_warehouse: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_warehouse: got into the Galactic HQ 2F, first room.", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {0, -1}, 800ms, 100ms); // 3+
    pbf_move_left_joystick(context, {-1, 0}, 1400ms, 100ms); // 6+

    {
        BlackScreenFlashWatcher black_screen(COLOR_BLUE, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
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

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 1000ms, 100ms); // 6
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms); // 6+
            },
            {{galactic_hq_scientist_fredrick_fight}}
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
                stream.log("galactic_hq_scientist_fredrick: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "galactic_hq_scientist_fredrick");
            }
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {0, +1}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{pick_up_galactic_key}}
        );
        if (ret == 0){
            stream.log("go_through_warehouse: Picked up the Galactic key.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_B, 3000ms);
        } else if (ret < 0){
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {0, +1}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{open_door}}
        );
        if (ret == 0){
            stream.log("go_through_warehouse: Opened the first door with the Galactic Key.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 4000ms);
        } else if (ret < 0){
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, -1}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("go_through_warehouse: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_warehouse: got out of the Galactic Warehouse.", COLOR_GREEN);
    };

    return true;
}


static bool heal_at_veilstone_pokemon_center(
    VideoStream& stream,
    ProControllerContext& context
){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("heal_at_veilstone_pokemon_center: Flying to Veilstone Pokemon Center and entering...");

    fly_to(stream, context, FlyPoint::CurrentCity);

    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

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
            stream.log("heal_at_veilstone_pokemon_center: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("heal_at_veilstone_pokemon_center: entered the Veilstone Pokemon Center.", COLOR_GREEN);
    };

    return true;
}

static bool go_to_galactic_hq(
    VideoStream& stream,
    ProControllerContext& context
){
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("go_to_galactic_hq, entering the Galactic HQ from the front door: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_to_galactic_hq: entered the Galactic HQ from the front door.", COLOR_GREEN);
    };

    return true;
}

static bool go_through_galactic_hq(
    VideoStream& stream,
    ProControllerContext& context,
    AutoStoryOptions options
){
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {0, +1}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{open_door}}
        );
        if (ret == 0){
            stream.log("go_through_galactic_hq: Opened the door.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 4000ms);
        } else if (ret < 0){
                stream.log("go_through_galactic_hq, opening the door: Didn't detect door dialog box!", COLOR_RED);
                return false;
        }
    }

    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {0, +1}, 1200ms, 100ms); // 5+

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
            stream.log("go_through_galactic_hq, entering Galactic HQ's 2F, first room: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_galactic_hq: entered Galactic HQ's 2F, first room.", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {0, -1}, 1000ms, 100ms); // 4+


    bool grunt_2_done = false;
    {
        MarkDetector galactic_hq_grunt_2_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
        // First run to the right, and see grunt 2 detects you

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 3200ms, 100ms); // 16+
                context.wait_for_all_requests();
                pbf_wait(context, 2000ms);
                context.wait_for_all_requests();
            },
            {{galactic_hq_grunt_2_fight}}
        );
        if (ret < 0) {
            // If it doesn't, move on, but consider that grunt_2 hasn't been done
            stream.log("galactic_hq_grunt_2: mark not detected, moving on...");
        } else if (ret == 0){
            // If it does, handle the fight and consider that grunt_2 has been done
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
                stream.log("galactic_hq_grunt_2: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "galactic_hq_grunt_2");
                grunt_2_done = true;
            }
        }
    }
    

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    // Move to the end against the wall
    pbf_move_left_joystick(context, {+1, 0}, 1000ms, 100ms); // 5+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);

    if (options.help_mode == HelpMode::OUTSIDE_HELP) {
        if (!grunt_2_done) {
            {
                // Now run downwards and check if either mark is detected (it should)
                MarkDetector galactic_hq_grunt_2_3_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

                int ret = run_until<ProControllerContext>(
                    stream, context,
                    [](ProControllerContext& context){
                        pbf_move_left_joystick(context, {0, -1}, 5000ms, 100ms); // 16+
                    },
                    {{galactic_hq_grunt_2_3_fight}}
                );

                context.wait_for_all_requests();
                pbf_wait(context, 2000ms);

                if (ret < 0 ) {
                    // Mark isn't detected, there is an issue
                    stream.log("galactic_hq_grunt_2_3: mark not detected!", COLOR_RED);
                    return false;
                } else if (ret == 0){
                    context.wait_for_all_requests();

                    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
                    ret = run_until<ProControllerContext>(
                        stream, context,
                        [](ProControllerContext& context){
                            pbf_mash_button(context, BUTTON_A, 200000ms);
                        },
                        {{black_screen}}
                    );
                    if (ret < 0){
                        stream.log("galactic_hq_grunt_2: black screen not detected!", COLOR_RED);
                        return false;
                    } else if (ret == 0) {
                        handle_battle(stream, context, "galactic_hq_grunt_2");
                    }
                }
            }
        }

        context.wait_for_all_requests();
        
        {
            BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            int ret = run_until<ProControllerContext>(
                stream, context,
                [](ProControllerContext& context){
                    ssf_press_left_joystick(context, {0, -1}, 0ms, 10000ms);
                    ssf_mash1_button(context, BUTTON_A, 10000ms);
                },
                {{black_screen}}
            );
            if (ret < 0){
                stream.log("galactic_hq_grunt_2: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                context.wait_for_all_requests();

                handle_battle(stream, context, "galactic_hq_grunt_3");
                context.wait_for_all_requests();
                pbf_wait(context, 2000ms);
            }
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    pbf_move_left_joystick(context, {0, -1}, 1200ms, 100ms); // 5+
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 7);
    pbf_move_left_joystick(context, {0, +1}, 2600ms, 100ms); // 13+
    pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms); // 3+

    {
        BlackScreenFlashWatcher black_screen(COLOR_BLUE, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, -1}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
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

    if (options.help_mode == HelpMode::OUTSIDE_HELP) {
        {
            MarkDetector galactic_hq_grunt_4_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

            int ret = run_until<ProControllerContext>(
                stream, context,
                [](ProControllerContext& context){
                    pbf_move_left_joystick(context, {-1, 0}, 10000ms, 100ms); // 5+
                },
                {{galactic_hq_grunt_4_fight}}
            );
            if (ret == 0){
                context.wait_for_all_requests();
                pbf_wait(context, 3000ms);

                BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
                ret = run_until<ProControllerContext>(
                    stream, context,
                    [](ProControllerContext& context){
                        pbf_mash_button(context, BUTTON_A, 200000ms);
                    },
                    {{black_screen}}
                );
                if (ret < 0){
                    stream.log("galactic_hq_grunt_4: black screen not detected!", COLOR_RED);
                    return false;
                } else if (ret == 0){
                    handle_battle(stream, context, "galactic_hq_grunt_4");
                }
            }
        }

        {
            BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            int ret = run_until<ProControllerContext>(
                stream, context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 10000ms);
                },
                {{black_screen}}
            );
            if (ret < 0){
                stream.log("galactic_hq_grunt_5: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "galactic_hq_grunt_5");
            }
        }
    };

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    pbf_move_left_joystick(context, {-1, 0}, 4400ms, 100ms); // 22+
    pbf_move_left_joystick(context, {0, -1}, 2200ms, 100ms); // 11+
    pbf_move_left_joystick(context, {-1, 0}, 1200ms, 100ms); // 5+

    {
        BlackScreenFlashWatcher black_screen(COLOR_BLUE, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {0, +1}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{open_door}}
        );
        if (ret == 0){
            stream.log("go_through_galactic_hq: Opened the door with the Galactic Key.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 4000ms);
        } else if (ret < 0){
                stream.log("go_through_galactic_hq: Didn't detect door opening dialog box!", COLOR_RED);
                return false;
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    {
        ShortDialogWatcher cyrus_dialog(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms); // 5+
            },
            {{cyrus_dialog}}
        );
        if (ret == 0){
            stream.log("go_through_galactic_hq: Cyrus dialog box detected.", COLOR_GREEN);

            BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            ret = run_until<ProControllerContext>(
                stream, context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 200000ms);
                },
                {{black_screen}}
            );
            if (ret < 0){
                stream.log("galactic_hq_cyrus: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "galactic_hq_cyrus");
            }
        }
    }
    
    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    pbf_mash_button(context, BUTTON_A, 17000ms);

    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {0, +1}, 800ms, 100ms); // 3+
    pbf_move_left_joystick(context, {+1, 0}, 1400ms, 100ms); // 6+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 2);
    pbf_move_left_joystick(context, {+1, 0}, 1600ms, 100ms); // 8+

    {
        BlackScreenFlashWatcher black_screen(COLOR_BLUE, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {+1, 0}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{talk_to_commander_saturn}}
        );
        if (ret < 0){
            stream.log("go_through_galactic_hq: Didn't detect Commander Saturn dialog box!", COLOR_RED);
            return false;
        } else if (ret == 0){
            stream.log("go_through_galactic_hq: Starting the battle with Commander Saturn.", COLOR_GREEN);

            BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            ret = run_until<ProControllerContext>(
                stream, context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 200000ms);
                },
                {{black_screen}}
            );
            if (ret < 0){
                stream.log("galactic_hq_commander_saturn: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "galactic_hq_commander_saturn");
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {0, +1}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{unlock_guardians}}
        );
        if (ret == 0){
            stream.log("go_through_galactic_hq: Released the Lake Guardians.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 4000ms);
            pbf_mash_button(context, BUTTON_B, 8000ms);
        } else if (ret < 0){
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, -1}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, -1}, 10000ms, 100ms);
            },
            {{white_screen}}
        );
        if (ret < 0){
            stream.log("go_through_galactic_hq: white screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_galactic_hq: exited Galactic HQ.", COLOR_GREEN);
    };

    return true;
}


static bool fly_and_heal_at_oreburgh(
    VideoStream& stream,
    ProControllerContext& context,
    AutoStoryOptions options
){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("fly_and_heal_at_oreburgh: Flying to Oreburgh City and healing pokemon...");

    fly_to(stream, context, FlyPoint::OreburghCity);

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

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
            stream.log("fly_and_heal_at_oreburgh: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("fly_and_heal_at_oreburgh: entered Oreburgh City's Pokemon Center.", COLOR_GREEN);
    };

    return true;
}

static bool go_to_mount_coronet(
    VideoStream& stream,
    ProControllerContext& context,
    AutoStoryOptions options
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_to_mount_coronet: Heading towards Mount Coronet...");

    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 6);
    pbf_move_left_joystick(context, {0, +1}, 6000ms, 100ms); // 30+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 3);
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 2);
    pbf_move_left_joystick(context, {+1, 0}, 1600ms, 100ms); // 7+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);

    context.wait_for_all_requests();
    get_on_bicycle(stream, context, GearLevel::Fast);

    pbf_move_left_joystick(context, {0, +1}, 1300ms, 100ms); // 9- (bike fast gear)
    pbf_press_button(context, BUTTON_PLUS, 80ms, 300ms);
    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {+1, 0}, 3400ms, 100ms); // 26
    pbf_move_left_joystick(context, {0, +1}, 1000ms, 100ms); // 4+

    {
        MarkDetector hiker_justin_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 5000ms, 100ms); // 11+
            },
            {{hiker_justin_fight}}
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
                stream.log("go_to_mount_coronet, route_207_hiker_justin: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "route_207_hiker_justin");
                context.wait_for_all_requests();
                pbf_wait(context, 3000ms);
                move_back_after_unexpected_battle(stream, context, "route_207_hiker_justin");
            }
        }
    }

    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 2);

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
            stream.log("go_to_mount_coronet: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_to_mount_coronet: entered Mt. Coronet.", COLOR_GREEN);
    };

    return true;
}


static bool go_through_mount_coronet(
    VideoStream& stream,
    ProControllerContext& context,
    AutoStoryOptions options
){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_through_mount_coronet: Going through Mount Coronet...");

    // @GumGum: Here the code, you can add more of these if you need them, but remember to adapt the numbering.

    return true;
}

static bool fly_and_heal_at_veilstone(
    VideoStream& stream,
    ProControllerContext& context,
    AutoStoryOptions options
){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("fly_and_heal_at_veilstone: Flying to Veilstone City and healing there...");

    fly_to(stream, context, FlyPoint::VeilstoneCity);

    context.wait_for_all_requests();

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
            stream.log("fly_and_heal_at_veilstone: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("fly_and_heal_at_veilstone: entered Veilstone City Pokemon Center.", COLOR_GREEN);
    };

    return true;
}

static bool go_to_house_south_veilstone(
    VideoStream& stream,
    ProControllerContext& context,
    AutoStoryOptions options
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_to_house_south_veilstone: heading South...");

    pbf_move_left_joystick(context, {0, -1}, 600ms, 100ms); // 2+
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, -1}, 10000ms, 100ms);
            },
            {{black_screen_1}}
        );
        if (ret < 0){
            stream.log("go_to_route_214_enter_house: black screen not detected!", COLOR_RED);
            return false;
        }
    }
    
    return true;
}

static bool go_through_house_south_veilstone(
    VideoStream& stream,
    ProControllerContext& context,
    AutoStoryOptions options
){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_through_house_south_veilstone: moving towards exit...");

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
            stream.log("go_through_house_south_veilstone: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_house_south_veilstone: exited the house", COLOR_GREEN);
    }
    
    return true;
}

static bool go_to_house_west_sunyshore(
    VideoStream& stream,
    ProControllerContext& context,
    AutoStoryOptions options
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_to_house_west_sunyshore: going towards Sunyshore City...");

    pbf_move_left_joystick(context, {+1, 0}, 600ms, 100ms); // 2+
    pbf_move_left_joystick(context, {0, -1}, 3800ms, 100ms); // 19+
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 8);

    activate_repel(stream, context);

    pbf_move_left_joystick(context, {0, -1}, 2600ms, 100ms); // 13+

    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);
    pbf_move_left_joystick(context, {0, -1}, 1200ms, 100ms); // 6+
    pbf_move_left_joystick(context, {-1, 0}, 400ms, 100ms); // 2
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

    {
        ShortDialogWatcher repel_dialog(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 2000ms, 100ms); // 10+
                pbf_move_left_joystick(context, {+1, 0}, 2000ms, 100ms); // 10+
            },
            {{repel_dialog}}
        );
        if (ret == 0){
            stream.log("go_to_house_west_sunyshore: Reactivating repel.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 3000ms);
        } else if (ret < 0){
                stream.log("go_to_house_west_sunyshore: Didn't detect repel dialog box (1)!", COLOR_RED);
                return false;
        }
    }

    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {+1, 0}, 3000ms, 100ms); // 15+

    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 9);
    pbf_move_left_joystick(context, {0, -1}, 8400ms, 100ms); // 42+
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 4);
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 5);
    pbf_move_left_joystick(context, {+1, 0}, 7400ms, 100ms); // 37+

    {
        ShortDialogWatcher repel_dialog(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 7400ms, 100ms); // 37+
                pbf_move_left_joystick(context, {+1, 0}, 7400ms, 100ms); // 37+
            },
            {{repel_dialog}}
        );
        if (ret == 0){
            stream.log("go_to_house_west_sunyshore: Not reactivating repel.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_B, 3000ms);
        } else if (ret < 0){
                stream.log("go_to_house_west_sunyshore: Didn't detect repel dialog box (2)!", COLOR_RED);
                return false;
        }
    }

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {+1, 0}, 7400ms, 100ms); // 37+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 3);
    pbf_move_left_joystick(context, {+1, 0}, 1400ms, 100ms); // 7+
    pbf_move_left_joystick(context, {0, +1}, 800ms, 100ms); // 3+
    pbf_move_left_joystick(context, {+1, 0}, 5200ms, 100ms); // 26+
    pbf_move_left_joystick(context, {0, -1}, 400ms, 100ms); // 1+

    use_rock_smash(stream, context);

    context.wait_for_all_requests();

    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 2);

    {
        MarkDetector route_222_sailor_luther_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 10000ms, 100ms); // 5+
            },
            {{route_222_sailor_luther_fight}}
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
                stream.log("go_to_house_west_sunyshore_route_222_sailor_luther: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "route_222_sailor_luther");
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("go_to_house_west_sunyshore: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_to_house_west_sunyshore: entered the house", COLOR_GREEN);
    }
    
    return true;
}

static bool go_through_house_west_sunyshore(
    VideoStream& stream,
    ProControllerContext& context,
    AutoStoryOptions options
){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_through_house_west_sunyshore: moving towards exit...");

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
            stream.log("go_through_house_south_veilstone: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_house_south_veilstone: exited the house", COLOR_GREEN);
    }
    
    return true;
}

static bool go_to_sunyshore_lighthouse(
    VideoStream& stream,
    ProControllerContext& context,
    AutoStoryOptions options
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_to_sunyshore_lighthouse: going towards the lighthouse to talk to Volkner...");

    {
        ShortDialogWatcher listen_to_flint(COLOR_BLUE);
        int ret = wait_until(stream, context, std::chrono::seconds(30), {{listen_to_flint}});
        if (ret < 0){
            stream.log("go_to_house_west_sunyshore, listen_to_flint: dialog box not detected!", COLOR_RED);
            return false;
        } else if (ret == 0) {
            stream.log("go_to_house_west_sunyshore, listen_to_flint: dialog box detected.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_B, 15000ms);
            context.wait_for_all_requests();
        }
    }

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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("go_through_house_west_sunyshore: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_house_west_sunyshore: entered the lighthouse", COLOR_GREEN);
    }
    
    return true;
}

static bool talk_to_volkner_at_the_lighthouse(
    VideoStream& stream,
    ProControllerContext& context,
    AutoStoryOptions options
){
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
        } else if (ret == 0) {
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

    {
        ShortDialogWatcher talk_to_volkner(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {+1, 0}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{talk_to_volkner}}
        );
        if (ret == 0){
            stream.log("talk_to_volkner_at_the_lighthouse: Talking to Volkner.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_B, 11000ms);
        } else if (ret < 0){
                stream.log("talk_to_volkner_at_the_lighthouse: Didn't detect Volkner dialog box!", COLOR_RED);
                return false;
        }
    }

    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms); // 3+
    pbf_move_left_joystick(context, {0, -1}, 1400ms, 100ms); // 7+
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 4);

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
            stream.log("talk_to_volkner_at_the_lighthouse: entering elevator to exit black screen not detected!", COLOR_RED);
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
        } else if (ret == 0) {
            stream.log("talk_to_volkner_at_the_lighthouse: exited the elevator.", COLOR_GREEN);
        }
    }

    return true;
}

static bool go_to_sunyshore_gym(
    VideoStream& stream,
    ProControllerContext& context,
    AutoStoryOptions options
){
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
    pbf_move_left_joystick(context, {0, +1}, 600ms, 100ms); // 2+

    {
        ShortDialogWatcher talk_to_flint(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {+1, 0}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{talk_to_flint}}
        );
        if (ret == 0){
            stream.log("go_to_sunyshore_gym: Talking to Flint.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_B, 9000ms);
        } else if (ret < 0){
                stream.log("go_to_sunyshore_gym: Didn't detect Flint dialog box!", COLOR_RED);
                return false;
        }
    }

    context.wait_for_all_requests();

    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 1);

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
            stream.log("go_to_sunyshore_gym: black screen entering the gym not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_to_sunyshore_gym: entered the gym.", COLOR_GREEN);
    }

    return true;
}



// ------------------------------------------------------------------ Checkpoints logic --------------------------------------------------------- //


void checkpoint_111(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!leave_gym(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "leave_gym: transition not detected.", env.console);
            }
        },
        true
    );
}

void checkpoint_112(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!heal_at_snowpoint_city_pokemon_center(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "heal_at_snowpoint_city_pokemon_center: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_113(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!heal_and_exit(env.console, context, "Snowpoint City")){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "heal_and_exit_snowpoint_city: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_114(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_to_lake_acuity(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_to_lake_acuity: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_115(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!talk_to_clint_then_go_veilstone(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "talk_to_clint_then_go_veilstone: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_116(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!heal_and_exit(env.console, context, "Veilstone City")){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "heal_and_exit_veilstone_city: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_117(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!get_key_and_enter_warehouse(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "get_key_and_enter_warehouse: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_118(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_through_warehouse(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_through_warehouse: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_119(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!heal_at_veilstone_pokemon_center(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "heal_at_veilstone_pokemon_center: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_120(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!heal_and_exit(env.console, context, "Veilstone City")){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "heal_and_exit_veilstone_city: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_121(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_to_galactic_hq(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_to_galactic_hq: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_122(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_through_galactic_hq(env.console, context, options)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_through_galactic_hq: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_123(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!fly_and_heal_at_oreburgh(env.console, context, options)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "fly_and_heal_at_oreburgh: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_124(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!heal_and_exit(env.console, context, "Oreburgh City")){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "heal_and_exit_oreburgh_city: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_125(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_to_mount_coronet(env.console, context, options)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_to_mount_coronet: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_126(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_through_mount_coronet(env.console, context, options)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_through_mount_coronet: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_127(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!fly_and_heal_at_veilstone(env.console, context, options)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "fly_and_heal_at_veilstone: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_128(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!heal_and_exit(env.console, context, "Veilstone City")){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "heal_and_exit_veilstone_city: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_129(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_to_house_south_veilstone(env.console, context, options)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_to_house_south_veilstone: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_130(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_through_house_south_veilstone(env.console, context, options)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_through_house_south_veilstone: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_131(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_to_house_west_sunyshore(env.console, context, options)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_to_house_west_sunyshore: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_132(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_through_house_west_sunyshore(env.console, context, options)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_through_house_west_sunyshore: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_133(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_to_sunyshore_lighthouse(env.console, context, options)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_to_sunyshore_lighthouse: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_134(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!talk_to_volkner_at_the_lighthouse(env.console, context, options)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "talk_to_volkner_at_the_lighthouse: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_135(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_to_sunyshore_gym(env.console, context, options)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_to_sunyshore_gym: transition not detected.", env.console);
            }
        },
        false
    );
}



std::string AutoStory_Checkpoint_111::name()       const{ return "111 - Segment 08 - Gym badge acquired"; }
std::string AutoStory_Checkpoint_111::start_text() const{ return "Seventh Gym Badge acquired (Snowpoint City)."; }
std::string AutoStory_Checkpoint_111::end_text()   const{ return "Leave Snowpoint Gym."; }
void AutoStory_Checkpoint_111::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_111(env, context, options, stats);
}

std::string AutoStory_Checkpoint_112::name()       const{ return "112 - Segment 08 - Left Snowpoint Gym"; }
std::string AutoStory_Checkpoint_112::start_text() const{ return "Left Snowpoint Gym."; }
std::string AutoStory_Checkpoint_112::end_text()   const{ return "Arrived at Snowpoint City Pokémon Center."; }
void AutoStory_Checkpoint_112::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_112(env, context, options, stats);
}

std::string AutoStory_Checkpoint_113::name()       const{ return "113 - Segment 08 - Arrived at Snowpoint Pokemon Center"; }
std::string AutoStory_Checkpoint_113::start_text() const{ return "Arrived at Snowpoint City Pokémon Center."; }
std::string AutoStory_Checkpoint_113::end_text()   const{ return "Healed at Snowpoint City Pokemon Center."; }
void AutoStory_Checkpoint_113::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_113(env, context, options, stats);
}

std::string AutoStory_Checkpoint_114::name()       const{ return "114 - Segment 08 - Healed at Snowpoint City Pokemon Center"; }
std::string AutoStory_Checkpoint_114::start_text() const{ return "Healed at Snowpoint City Pokemon Center."; }
std::string AutoStory_Checkpoint_114::end_text()   const{ return "Entered Lake Acuity Lakefront."; }
void AutoStory_Checkpoint_114::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_114(env, context, options, stats);
}

std::string AutoStory_Checkpoint_115::name()       const{ return "115 - Segment 08 - Entered Acuity Lakefront"; }
std::string AutoStory_Checkpoint_115::start_text() const{ return "Entered Lake Acuity Lakefront."; }
std::string AutoStory_Checkpoint_115::end_text()   const{ return "Arrived at Veilstone City Pokemon Center."; }
void AutoStory_Checkpoint_115::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_115(env, context, options, stats);
}

std::string AutoStory_Checkpoint_116::name()       const{ return "116 - Segment 08 - Arrived at Veilstone Pokemon Center"; }
std::string AutoStory_Checkpoint_116::start_text() const{ return "Arrived at Veilstone City Pokemon Center."; }
std::string AutoStory_Checkpoint_116::end_text()   const{ return "Healed at Veilstone City Pokemon Center."; }
void AutoStory_Checkpoint_116::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_116(env, context, options, stats);
}

std::string AutoStory_Checkpoint_117::name()       const{ return "117 - Segment 08 - Healed at Veilstone Pokemon Center"; }
std::string AutoStory_Checkpoint_117::start_text() const{ return "Healed at Veilstone City Pokemon Center."; }
std::string AutoStory_Checkpoint_117::end_text()   const{ return "Entered Team Galactic Warehouse."; }
void AutoStory_Checkpoint_117::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_117(env, context, options, stats);
}

std::string AutoStory_Checkpoint_118::name()       const{ return "118 - Segment 08 - Entered Team Galactic Warehouse"; }
std::string AutoStory_Checkpoint_118::start_text() const{ return "Entered Team Galactic Warehouse."; }
std::string AutoStory_Checkpoint_118::end_text()   const{ return "Got out of the warehouse with the key."; }
void AutoStory_Checkpoint_118::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_118(env, context, options, stats);
}

std::string AutoStory_Checkpoint_119::name()       const{ return "119 - Segment 08 - Fly back to Veilstone Pokemon Center for healing"; }
std::string AutoStory_Checkpoint_119::start_text() const{ return "Got out of the warehouse with the key."; }
std::string AutoStory_Checkpoint_119::end_text()   const{ return "Entered Veilstone Pokemon Center for healing."; }
void AutoStory_Checkpoint_119::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_119(env, context, options, stats);
}

std::string AutoStory_Checkpoint_120::name()       const{ return "120 - Segment 08 - Heal Pokemon"; }
std::string AutoStory_Checkpoint_120::start_text() const{ return "Entered Veilstone Pokemon Center for healing."; }
std::string AutoStory_Checkpoint_120::end_text()   const{ return "Exited Veilstone Pokemon Center."; }
void AutoStory_Checkpoint_120::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_120(env, context, options, stats);
}

std::string AutoStory_Checkpoint_121::name()       const{ return "121 - Segment 08 - Go to Galactic HQ"; }
std::string AutoStory_Checkpoint_121::start_text() const{ return "Exited Veilstone Pokemon Center."; }
std::string AutoStory_Checkpoint_121::end_text()   const{ return "Entered Galactic HQ."; }
void AutoStory_Checkpoint_121::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_121(env, context, options, stats);
}

std::string AutoStory_Checkpoint_122::name()       const{ return "122 - Segment 08 - Deal with Cyrus at the Galactic HQ"; }
std::string AutoStory_Checkpoint_122::start_text() const{ return "Entered Galactic HQ."; }
std::string AutoStory_Checkpoint_122::end_text()   const{ return "Dealt with Cyrus and left the Galactic HQ."; }
void AutoStory_Checkpoint_122::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_122(env, context, options, stats);
}

std::string AutoStory_Checkpoint_123::name()       const{ return "123 - Segment 08 - Fly to Oreburgh City"; }
std::string AutoStory_Checkpoint_123::start_text() const{ return "Dealt with Cyrus and left the Galactic HQ."; }
std::string AutoStory_Checkpoint_123::end_text()   const{ return "Flew to Oreburgh city and entered Pokemon Center."; }
void AutoStory_Checkpoint_123::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_123(env, context, options, stats);
}

std::string AutoStory_Checkpoint_124::name()       const{ return "124 - Segment 08 - Heal Pokemon at Oreburgh City Pokemon Center"; }
std::string AutoStory_Checkpoint_124::start_text() const{ return "Flew to Oreburgh City and entered Pokemon Center."; }
std::string AutoStory_Checkpoint_124::end_text()   const{ return "Healed Pokemon at Oreburgh City Pokemon Center."; }
void AutoStory_Checkpoint_124::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_124(env, context, options, stats);
}

std::string AutoStory_Checkpoint_125::name()       const{ return "125 - Segment 08 - Go from Oreburgh City to Mount Coronet entrance"; }
std::string AutoStory_Checkpoint_125::start_text() const{ return "Healed Pokemon at Oreburgh City Pokemon Center."; }
std::string AutoStory_Checkpoint_125::end_text()   const{ return "Entered Mount Coronet."; }
void AutoStory_Checkpoint_125::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_125(env, context, options, stats);
}

std::string AutoStory_Checkpoint_126::name()       const{ return "126 - Segment 08 - Go through Mt. Coronet to deal with Cyrus and Dialga/Palkia"; }
std::string AutoStory_Checkpoint_126::start_text() const{ return "Entered Mount Coronet."; }
std::string AutoStory_Checkpoint_126::end_text()   const{ return "Exited Mt. Coronet, having dealt with Cyrus."; }
void AutoStory_Checkpoint_126::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_126(env, context, options, stats);
}

std::string AutoStory_Checkpoint_127::name()       const{ return "127 - Segment 08 - Fly to Veilstone City"; }
std::string AutoStory_Checkpoint_127::start_text() const{ return "Exited Mt. Coronet, having dealt with Cyrus."; }
std::string AutoStory_Checkpoint_127::end_text()   const{ return "Entered the Veilstone Pokemon Center."; }
void AutoStory_Checkpoint_127::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_127(env, context, options, stats);
}

std::string AutoStory_Checkpoint_128::name()       const{ return "126 - Segment 08 - Heal and exit"; }
std::string AutoStory_Checkpoint_128::start_text() const{ return "Entered the Veilstone Pokemon Center."; }
std::string AutoStory_Checkpoint_128::end_text()   const{ return "Exited the Veilstone Pokemon Center."; }
void AutoStory_Checkpoint_128::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_128(env, context, options, stats);
}

std::string AutoStory_Checkpoint_129::name()       const{ return "129 - Segment 08 - Go to house South of Veilstone City"; }
std::string AutoStory_Checkpoint_129::start_text() const{ return "Exited the Veilstone Pokemon Center."; }
std::string AutoStory_Checkpoint_129::end_text()   const{ return "Arrived at the house South of Veilstone City."; }
void AutoStory_Checkpoint_129::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_129(env, context, options, stats);
}

std::string AutoStory_Checkpoint_130::name()       const{ return "130 - Segment 08 - Go through the house South of Veilstone City"; }
std::string AutoStory_Checkpoint_130::start_text() const{ return "Arrived at the house South of Veilstone City."; }
std::string AutoStory_Checkpoint_130::end_text()   const{ return "Exited the house South of Veilstone City."; }
void AutoStory_Checkpoint_130::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_130(env, context, options, stats);
}

std::string AutoStory_Checkpoint_131::name()       const{ return "131 - Segment 08 - Go through routes 214 and 222 to the house West of Sunyshore City"; }
std::string AutoStory_Checkpoint_131::start_text() const{ return "Exited house South of Veilstone City."; }
std::string AutoStory_Checkpoint_131::end_text()   const{ return "Entered the house West of Sunyshore City."; }
void AutoStory_Checkpoint_131::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_131(env, context, options, stats);
}

std::string AutoStory_Checkpoint_132::name()       const{ return "132 - Segment 08 - Go through the house West of Sunyshore City"; }
std::string AutoStory_Checkpoint_132::start_text() const{ return "Entered the house West of Sunyshore City."; }
std::string AutoStory_Checkpoint_132::end_text()   const{ return "Entered Sunyshore City."; }
void AutoStory_Checkpoint_132::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_132(env, context, options, stats);
}

std::string AutoStory_Checkpoint_133::name()       const{ return "133 - Segment 08 - Go to Sunyshore City's lighthouse"; }
std::string AutoStory_Checkpoint_133::start_text() const{ return "Entered Sunyshore City."; }
std::string AutoStory_Checkpoint_133::end_text()   const{ return "Entered Sunyshore City's lighthouse."; }
void AutoStory_Checkpoint_133::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_133(env, context, options, stats);
}

std::string AutoStory_Checkpoint_134::name()       const{ return "134 - Segment 08 - Talk to Volkner at the lighthouse"; }
std::string AutoStory_Checkpoint_134::start_text() const{ return "Entered Sunyshore City's lighthouse."; }
std::string AutoStory_Checkpoint_134::end_text()   const{ return "Exited Sunyshore City's lighthouse."; }
void AutoStory_Checkpoint_134::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_134(env, context, options, stats);
}

std::string AutoStory_Checkpoint_135::name()       const{ return "135 - Segment 08 - Go to the Sunyshore City gym"; }
std::string AutoStory_Checkpoint_135::start_text() const{ return "Exited Sunyshore City's lighthouse."; }
std::string AutoStory_Checkpoint_135::end_text()   const{ return "Entered Sunyshore City's gym."; }
void AutoStory_Checkpoint_135::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_135(env, context, options, stats);
}



void AutoStory_Checkpoint_021::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_021(env, context, options, stats);
}


void checkpoint_021(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            // TODO: implement Segment 08 gameplay logic (seventh badge area)
        },
        true
    );
}

void checkpoint_021_legendary(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            approach_legendary(env.console, context);
            enter_legendary_battle(env.console, context);
            weaken_legendary(env.console, context);
            throw_pokeball_at_legendary(env.console, context);
            confirm_legendary_catch(env.console, context);
            exit_legendary_encounter(env.console, context);
        },
        true
    );
}


}
}
}
