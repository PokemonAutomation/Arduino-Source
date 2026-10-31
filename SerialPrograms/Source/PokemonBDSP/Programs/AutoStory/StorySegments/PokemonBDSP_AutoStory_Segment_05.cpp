/*  BDSP AutoStory - Segment 05
 *  From after acquiring the fourth badge until after acquiring the fifth badge.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

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
#include "PokemonBDSP/Programs/AutoStory/Detect/PokemonBDSP_AutoStory_OverworldDetector.h"
#include "PokemonBDSP/Programs/AutoStory/PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP/Programs/AutoStory/Utils/PokemonBDSP_AutoStory_Battle.h"
#include "PokemonBDSP/Programs/AutoStory/Utils/PokemonBDSP_AutoStory_Segment_00_tools.h"

#include "PokemonBDSP_AutoStory_Segment_02.h"

#include "PokemonBDSP_AutoStory_Segment_05.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonBDSP {

// ---------------------------------------------------------------------------
//  Helpers
// ---------------------------------------------------------------------------

static void
move_back_after_unexpected_battle(VideoStream& stream, ProControllerContext& context, const std::string& trainerid){
    stream.log("Starting battle with " + trainerid, COLOR_GREEN);
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    if (trainerid == "route_213_tuber_chelsea"){
        DpadState dpad;

        pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms);  // 4+
        pbf_move_left_joystick(context, {0, +1}, 1000ms, 100ms); // 4+
        dpad.last_dir = DPAD_UP;
        repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 6);
    }else if (trainerid == "route_213_tuber_jared"){
        // Just need to wait, nothing else
    }else if (trainerid == "route_213_beauty_cindy"){
        pbf_move_left_joystick(context, {0, -1}, 400ms, 100ms); // 1+
    }else if (trainerid == "route_210_ace_ernest"){
        pbf_move_left_joystick(context, {-1, 0}, 3800ms, 100ms); // 19+
    }
}

// ---------------------------------------------------------------------------
//  Named route navigation blocks
// ---------------------------------------------------------------------------

static bool leave_gym(VideoStream& stream, ProControllerContext& context){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("leave_gym: walking out of gym");

    pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms);  // 1+
    pbf_move_left_joystick(context, {+1, 0}, 600ms, 100ms);  // 2+
    pbf_move_left_joystick(context, {0, +1}, 600ms, 100ms);  // 2+
    pbf_move_left_joystick(context, {+1, 0}, 2000ms, 100ms); // 10+
    pbf_move_left_joystick(context, {0, -1}, 6000ms, 100ms); // 34+
    pbf_move_left_joystick(context, {-1, 0}, 1800ms, 100ms); // 9+
    pbf_move_left_joystick(context, {0, -1}, 1000ms, 100ms); // 5+
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 3, true);
    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, -1}, 100000ms, 100ms);
        },
        {{black_screen}});
    if (ret < 0){
        stream.log("leave_gym: Didn't enter building!", COLOR_RED);
        return false;
    }
    stream.log("leave_gym: transition confirmed.");
    return true;
}

// Starts at 04.18's completed Gym exit. Fly within Pastoria to use the
// Pokemon Center's fixed arrival position instead of an unanchored city walk.
static bool go_to_pastoria_pokemon_center(VideoStream& stream, ProControllerContext& context){
    context.wait_for_all_requests();
    stream.log("[Pastoria Center] Flying from outside the Gym to the city's Pokemon Center.");
    if (!fly_to(stream, context, FlyPoint::CurrentCity)){
        stream.log("[Pastoria Center] Flight to the current city's Pokemon Center failed.", COLOR_RED);
        return false;
    }

    BlackScreenWatcher landed(COLOR_RED, {0.1, 0.1, 0.8, 0.8}, 100, 10, BlackScreenWatcher::FinderType::GONE, 500ms);
    if (wait_until(stream, context, 30s, {{landed}}) != 0){
        stream.log("[Pastoria Center] Flight transition did not finish.", COLOR_RED);
        return false;
    }
    pbf_wait(context, 500ms);
    context.wait_for_all_requests();

    // The Fly arrival is directly south of the Center entrance.
    BlackScreenWatcher entry_black(COLOR_RED);
    if (run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {0, +1}, 5000ms, 0ms);
                pbf_wait(ctx, 1000ms);
            },
        {{entry_black}}) != 0){
        stream.log("[Pastoria Center] Entrance transition did not start.", COLOR_RED);
        return false;
    }
    BlackScreenWatcher inside(COLOR_RED, {0.1, 0.1, 0.8, 0.8}, 100, 10, BlackScreenWatcher::FinderType::GONE, 500ms);
    if (wait_until(stream, context, 30s, {{inside}}) != 0){
        stream.log("[Pastoria Center] Entrance transition did not finish.", COLOR_RED);
        return false;
    }
    pbf_wait(context, 500ms);
    context.wait_for_all_requests();
    stream.log("[Pastoria Center] Entered after the completed Gym return.", COLOR_GREEN);
    return true;
}

static bool exit_pastoria_pokemon_center(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats){
    return heal_pokemon(stream, context, "Pastoria", &stats);
}

static bool
pastoria_city_drama(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("pastoria_city_drama: starting the drama");

    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 2, true);
    pbf_move_left_joystick(context, {+1, 0}, 1600ms, 100ms); // 10
    pbf_move_left_joystick(context, {0, +1}, 500ms, 100ms);  // 3
    pbf_move_left_joystick(context, {+1, 0}, 1000ms, 100ms); // 7
    pbf_move_left_joystick(context, {0, +1}, 1400ms, 100ms); // 7+
    pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms);  // 2+

    context.wait_for_all_requests();

    {
        ShortDialogWatcher talk_to_grunt_1(COLOR_BLUE);
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, 2000ms);
            },
            {{talk_to_grunt_1}});
        if (ret < 0){
            stream.log("pastoria_city_drama_talk_to_grunt_1: dialog box not detected!", COLOR_RED);
            return false;
        }
        if (ret == 0){
            stream.log("pastoria_city_drama_talk_to_grunt_1: talking to grunt!");
            clear_post_battle_dialogue(stream, context, BUTTON_B, 4000ms);
        };
    }


    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {+1, 0}, 4800ms, 100ms); // 24+
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 2, true);
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 1, true);

    {
        ShortDialogWatcher talk_to_grunt_2(COLOR_BLUE);
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, 2000ms);
            },
            {{talk_to_grunt_2}});
        if (ret < 0){
            stream.log("pastoria_city_drama_talk_to_grunt_2: dialog box not detected!", COLOR_RED);
            return false;
        }
        if (ret == 0){
            stream.log("pastoria_city_drama_talk_to_grunt_2: talking to grunt!");

            BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            int ret1 = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_B, 200000ms);
                },
                {{black_screen}});
            if (ret1 < 0){
                stream.log("pastoria_city_drama_talk_to_clint: Didn't detect start of battle with Clint!", COLOR_RED);
                return false;
            }
            stream.log("pastoria_city_drama_talk_to_clint: battling Clint.");
            fight_trainer_using_moves(stream, context, stats, "pastoria_clint", true);
            pbf_mash_button(context, BUTTON_B, 3000ms);
        };
    }


    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 1, true);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("pastoria_city_drama: transition not detected!", COLOR_RED);
            return false;
        }
        stream.log("pastoria_city_drama: transition detected.");
    }

    return true;
}

static bool go_through_house_east_pastoria(VideoStream& stream, ProControllerContext& context){
    stream.log("Going through the building at the entrance of Pastoria city");
    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {+1, 0}, 100000ms, 100ms);
        },
        {{black_screen}});
    if (ret < 0){
        stream.log("go_through_house_east_pastoria: Didn't exit building!", COLOR_RED);
        return false;
    }
    stream.log("go_through_house_east_pastoria: transition confirmed.");

    return true;
}

static bool
go_through_route_213_chasing(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_through_route_213_chasing: continuing the drama");

    pbf_move_left_joystick(context, {+1, 0}, 1600ms, 100ms); // 12
    if (!activate_repel(stream, context)){
        return false;
    }
    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);
    pbf_move_left_joystick(context, {+1, 0}, 3600ms, 100ms); // 18+

    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 5, true);

    MarkDetector beauty_cindy_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
    int ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, -1}, 5000ms, 100ms); // 15+
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
            fight_trainer_using_moves(stream, context, stats, "route_213_beauty_cindy", true);
            context.wait_for_all_requests();
            pbf_wait(context, 3000ms);

            // Cindy approaches from the west and blocks the next westward leg.
            // Step south into the gap above the cliff before walking around it.
            dpad.last_dir = DPAD_LEFT;
            repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 1, true);
            context.wait_for_all_requests();
        }
    }
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {-1, 0}, 600ms, 100ms);  // 4
    pbf_move_left_joystick(context, {0, -1}, 1200ms, 100ms); // 9
    pbf_move_left_joystick(context, {+1, 0}, 1600ms, 100ms); // 8+
    pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms);  // 2+
    pbf_move_left_joystick(context, {+1, 0}, 600ms, 100ms);  // 4
    pbf_move_left_joystick(context, {0, +1}, 600ms, 100ms);  // 3+
    pbf_move_left_joystick(context, {-1, 0}, 600ms, 100ms);  // 3+

    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 2, true);

    context.wait_for_all_requests();

    // Rock Smash uses the same short dialogue box as an NPC conversation.
    ShortDialogPromptDetector unexpected_choice(stream.overlay(), {0.70, 0.60, 0.20, 0.20}, COLOR_RED);
    ShortDialogWatcher talk_to_grunt(COLOR_BLUE);
    ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            // Finish approaching and facing east before interacting once.
            pbf_move_left_joystick(context, {+1, 0}, 5000ms, 100ms);
            pbf_press_button(context, BUTTON_A, 80ms, 1000ms);
        },
        {{unexpected_choice}, {talk_to_grunt}});
    if (ret < 0){
        stream.log("go_through_route_213_chasing_talk_to_grunt: dialog box not detected!", COLOR_RED);
        return false;
    }
    if (ret == 0){
        stream.log("[Route 213 grunt] Unexpected choice prompt; stopping the approach.", COLOR_RED);
        clear_post_battle_dialogue(stream, context, BUTTON_B, 4000ms);
        return false;
    }

    // The blank left edge of the NPC nameplate is independent of language.
    // A Rock Smash prompt has no nameplate, even if its arrow appears late.
    BlackScreenWatcher npc_nameplate(COLOR_BLUE, {0.21, 0.765, 0.01, 0.02});
    if (wait_until(stream, context, 1000ms, {{npc_nameplate}}) != 0){
        stream.log("[Route 213 grunt] NPC nameplate not detected; refusing this dialogue.", COLOR_RED);
        clear_post_battle_dialogue(stream, context, BUTTON_B, 4000ms);
        return false;
    }
    stream.log("[Route 213 grunt] NPC dialogue detected at the grunt approach.");
    clear_post_battle_dialogue(stream, context, BUTTON_B, 4000ms);

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms); // 2+
    context.wait_for_all_requests();

    force_repel_expiry(stream, context, RepelRenewalChoice::NO, false, true);

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {-1, 0}, 2000ms, 100ms);

    MarkDetector tuber_jared_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
    ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {+1, 0}, 3200ms, 100ms); // 17
            pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms);  // 3
            context.wait_for_all_requests();
            pbf_wait(context, 2000ms);
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
            fight_trainer_using_moves(stream, context, stats, "route_213_tuber_jared", true);
            context.wait_for_all_requests();
            pbf_wait(context, 3000ms);
        }
    }

    context.wait_for_all_requests();

    MarkDetector tuber_chelsea_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
    ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            DpadState dpad;
            pbf_move_left_joystick(context, {0, +1}, 1200ms, 100ms); // 6+
            pbf_move_left_joystick(context, {-1, 0}, 600ms, 100ms);  // 3+
            dpad.last_dir = DPAD_LEFT;
            repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 6, true);
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
            stream.log("go_through_route_213_tuber_chelsea: Didn't detect battle starting!");
        }else if (ret == 0){
            stream.log("go_through_route_213_tuber_chelsea: Starting battle.");
            fight_trainer_using_moves(stream, context, stats, "route_213_tuber_chelsea", true);
            context.wait_for_all_requests();
            pbf_wait(context, 3000ms);
        }
    }

    BlackScreenOverWatcher route_213_east_building(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, +1}, 20000ms, 100ms);
        },
        {{route_213_east_building}});
    if (ret < 0){
        stream.log("go_through_route_213_east_building: Didn't detect entering the building!", COLOR_RED);
        return false;
    };

    return true;
}

static bool go_through_building_route_213(VideoStream& stream, ProControllerContext& context){
    stream.log("Going through building between routes 214 and 213");
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, +1}, 30000ms, 100ms);
        },
        {{black_screen}});
    if (ret < 0){
        stream.log("go_through_building_route_214: Didn't exit building!", COLOR_RED);
        return false;
    }
    stream.log("go_through_building_route_213: transition confirmed.");

    return true;
}

static bool
go_through_route_214(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_through_route_214: continuing the drama");

    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 1, true);
    pbf_move_left_joystick(context, {0, +1}, 4000ms, 100ms); // 21+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 2, true);
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 1, true);

    context.wait_for_all_requests();

    ShortDialogWatcher talk_to_grunt(COLOR_BLUE);
    int ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            ssf_press_left_joystick(context, {+1, 0}, 0ms, 5000ms);
            ssf_mash1_button(context, BUTTON_A, 5000ms);
        },
        {{talk_to_grunt}});
    if (ret < 0){
        stream.log("go_through_route_214_talk_to_grunt: dialog box not detected!", COLOR_RED);
        return false;
    }
    if (ret == 0){
        stream.log("go_through_route_214_talk_to_grunt: talking to grunt!");
        clear_post_battle_dialogue(stream, context, BUTTON_B, 4000ms);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {0, +1}, 2200ms, 100ms); // 11+
    pbf_move_left_joystick(context, {+1, 0}, 1200ms, 100ms); // 6+
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 2, true);

    ShortDialogWatcher talk_to_grunt_2(COLOR_BLUE);
    ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            ssf_press_left_joystick(context, {0, +1}, 0ms, 5000ms);
            ssf_mash1_button(context, BUTTON_A, 5000ms);
        },
        {{talk_to_grunt_2}});
    if (ret < 0){
        stream.log("go_through_route_214_talk_to_grunt_2: dialog box not detected!", COLOR_RED);
        return false;
    }
    if (ret == 0){
        stream.log("go_through_route_214_talk_to_grunt_2: talking to grunt!");
        context.wait_for_all_requests();
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, 200000ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_route_214_galactic_grunt: Didn't detect battle starting!", COLOR_RED);
            return false;
        }else if (ret == 0){
            stream.log("go_through_route_214_galactic_grunt: Starting battle.");
            fight_trainer_using_moves(stream, context, stats, "route_214_galactic_grunt", true);
            context.wait_for_all_requests();
            pbf_wait(context, 5000ms);
        }
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 1, true);

    ShortDialogWatcher talk_to_cynthia(COLOR_BLUE);
    ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            ssf_press_left_joystick(context, {0, +1}, 0ms, 10000ms);
            ssf_mash1_button(context, BUTTON_A, 10000ms);
        },
        {{talk_to_cynthia}});
    if (ret < 0){
        stream.log("go_through_route_214_talk_to_cynthia: dialog box not detected!", COLOR_RED);
        return false;
    }
    if (ret == 0){
        stream.log("go_through_route_214_talk_to_cynthia: talking to Cynthia!");
        clear_post_battle_dialogue(stream, context, BUTTON_A, 6000ms);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 4000ms);

    if (!fly_to(stream, context, FlyPoint::SolaceonTown)){
        return false;
    }

    return true;
}

static bool go_inside_solaceon_pokemon_center(VideoStream& stream, ProControllerContext& context){
    stream.log("go_inside_solaceon_pokemon_center: going inside");
    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, +1}, 200000ms, 100ms);
        },
        {{black_screen}});
    if (ret < 0){
        stream.log("go_inside_solaceon_pokemon_center: black screen not detected!", COLOR_RED);
        return false;
    }else if (ret == 0){
        stream.log("go_inside_solaceon_pokemon_center: black screen detected!", COLOR_GREEN);
        return true;
    }
    return true;
}

static bool heal_at_solaceon_pokemon_center(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats){
    stream.log("heal_at_solaceon_pokemon_center: healing pokemon");
    context.wait_for_all_requests();
    return heal_pokemon(stream, context, "Solaceon", &stats);
}

static bool
go_to_the_psyduck_route_210(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_to_the_psyduck_route_210: going towards the Psyducks!");

    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 4);
    pbf_move_left_joystick(context, {0, +1}, 5500ms, 100ms);
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 1);
    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);
    if (!activate_repel(stream, context)){
        return false;
    }

    context.wait_for_all_requests();
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);
    pbf_move_left_joystick(context, {0, +1}, 200ms, 100ms); // 1+

    {
        MarkDetector battle_dialogue(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 20000ms, 100ms);
            },
            {{battle_dialogue}});
        if (ret == 0){
            context.wait_for_all_requests();
            pbf_wait(context, 2000ms);
            stream.log("go_to_the_psyduck_route_210_rancher_marco: marker detected!", COLOR_GREEN);

            BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            ret = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 200000ms);
                },
                {{black_screen}});
            if (ret < 0){
                stream.log("go_to_the_psyduck_route_210_rancher_marco: black screen not detected!", COLOR_RED);
                return false;
            }else if (ret == 0){
                fight_trainer_using_moves(stream, context, stats, "route_210_rancher_marco", true);
            }
        }
    }

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, +1}, 2400ms, 100ms); // 8+ tall grass
    pbf_move_left_joystick(context, {+1, 0}, 2000ms, 100ms); // 6+ tall grass


    {
        MarkDetector battle_dialogue(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
        ShortDialogWatcher battle_dialogue_2(COLOR_BLUE);
        BattleMenuWatcher amber_menu(BattleType::TRAINER, COLOR_GREEN);

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                DpadState dpad;
                pbf_move_left_joystick(context, {0, +1}, 6600ms,
                    100ms); // 22+ tall grass

                dpad.last_dir = DPAD_UP;
                repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 4);
                repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);
                pbf_mash_button(context, BUTTON_A, 200000ms);
            },
            {{battle_dialogue}, {battle_dialogue_2}, {amber_menu}});
        if (ret >= 0){
            context.wait_for_all_requests();
            pbf_wait(context, 2000ms);
            stream.log("go_to_the_psyduck_route_210_breeder_amber: marker detected!", COLOR_GREEN);

            if (ret != 2){
                // Confirm the command menu, not a black fade that could be battle exit.
                BattleMenuWatcher ready(BattleType::TRAINER, COLOR_GREEN);
                if (run_until<ProControllerContext>(stream,
                    context,
                    [](ProControllerContext& ctx){
                        pbf_mash_button(ctx, BUTTON_A, 30000ms);
                    },
                    {{ready}}) != 0){
                    stream.log("Route 210 Amber: Battle command menu not detected.", COLOR_RED);
                    return false;
                }
            }
            fight_trainer_using_moves(stream, context, stats, "route_210_breeder_amber", true);
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);
    pbf_move_left_joystick(context, {0, +1}, 800ms, 100ms); // 4+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 3);

    ShortDialogWatcher talk_to_psyducks(COLOR_BLUE);
    int ret_psyducks = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& context){
            ssf_press_left_joystick(context, {0, +1}, 0ms, 5000ms);
            ssf_mash1_button(context, BUTTON_A, 5000ms);
        },
        {{talk_to_psyducks}});
    if (ret_psyducks < 0){
        stream.log("go_to_the_psyduck_route_210_talk_to_psyducks: dialog box not detected!", COLOR_RED);
        return false;
    }
    if (ret_psyducks == 0){
        stream.log("go_to_the_psyduck_route_210_talk_to_psyducks: talking to Psyducks!");
        // Finish the medicine prompts, then wait through the departure animation.
        // Its dialogue-free gap does not mean Cynthia's follow-up is finished.
        mash_until_dialogue_ends(stream, context, BUTTON_A, 60s);
        stream.log("[Route 210] Waiting for Cynthia after the Psyducks leave.");
        wait_for_dialogue(stream, context, "Route 210 - Cynthia after Psyducks", 30s);
        clear_post_battle_dialogue(stream, context, BUTTON_A, 6000ms);
        stream.log("[Route 210] Cynthia follow-up cleared; resuming movement and Repel setup.");
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 1);
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 9);
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 3);
    context.wait_for_all_requests();

    if (!activate_repel(stream, context)){
        return false;
    }

    pbf_move_left_joystick(context, {0, +1}, 8000ms, 100ms); // 29+ tall grass
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);

    force_repel_expiry(stream, context, RepelRenewalChoice::YES, true, true);

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, +1}, 1000ms, 100ms); // 5+

    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);
    pbf_move_left_joystick(context, {0, +1}, 2400ms, 100ms); // 12+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);

    context.wait_for_all_requests();

    {
        MarkDetector battle_dialogue(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, +1}, 10000ms, 100ms);
            },
            {{battle_dialogue}});
        if (ret < 0){
            stream.log("go_to_the_psyduck_route_210_ace_alyssa: dialog box not detected!", COLOR_RED);
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
                stream.log("go_to_the_psyduck_route_210_ace_alyssa: black screen not detected!", COLOR_RED);
                return false;
            }else if (ret == 0){
                fight_trainer_using_moves(stream, context, stats, "route_210_ace_alyssa", true);
            }
        }
    }

    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 4);

    pbf_move_left_joystick(context, {0, +1}, 1000ms, 100ms); // 5+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 4);
    pbf_move_left_joystick(context, {0, +1}, 1200ms, 100ms); // 6+

    {
        MarkDetector battle_dialogue(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 3800ms, 100ms); // 19+
            },
            {{battle_dialogue}});
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
                stream.log("go_to_the_psyduck_route_210_ace_ernest: black screen not detected!", COLOR_RED);
                return false;
            }else if (ret == 0){
                fight_trainer_using_moves(stream, context, stats, "route_210_ace_ernest", true);
                context.wait_for_all_requests();
                pbf_wait(context, 3000ms);
                move_back_after_unexpected_battle(stream, context, "route_210_ace_ernest");
            }
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    {
        MarkDetector battle_dialogue(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, -1}, 20000ms, 100ms);
            },
            {{battle_dialogue}});
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
                stream.log("go_to_the_psyduck_route_210_blackbelt_adam: black screen not detected!", COLOR_RED);
                return false;
            }else if (ret == 0){
                fight_trainer_using_moves(stream, context, stats, "route_210_blackbelt_adam", true);
            }
        }
    }

    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {0, -1}, 200ms, 100ms);
    pbf_move_left_joystick(context, {+1, 0}, 200ms, 100ms);
    force_repel_expiry(stream, context, RepelRenewalChoice::NO, false, true);

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {+1, 0}, 1000ms, 100ms);

    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 5);

    pbf_move_left_joystick(context, {0, +1}, 1400ms, 100ms); // 7+

    {
        MarkDetector battle_dialogue(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 20000ms, 100ms);
            },
            {{battle_dialogue}});
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
                stream.log("go_to_the_psyduck_route_210_birdkeeper_brianna: black screen not detected!", COLOR_RED);
                return false;
            }else if (ret == 0){
                fight_trainer_using_moves(stream, context, stats, "route_210_birdkeeper_brianna", true);
            }
        }
    }

    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {-1, 0}, 200ms, 100ms); // 1+
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 4);

    {
        MarkDetector battle_dialogue(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 6000ms, 100ms);
            },
            {{battle_dialogue}});
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
                stream.log("go_to_the_psyduck_route_210_veteran_brian: black screen not detected!", COLOR_RED);
                return false;
            }else if (ret == 0){
                fight_trainer_using_moves(stream, context, stats, "route_210_veteran_brian", true);
            }
        }
    }

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {-1, 0}, 4000ms, 100ms); // 20+
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 1);
    pbf_move_left_joystick(context, {0, -1}, 2200ms, 100ms); // 11+
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 3);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 20000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_to_the_psyduck_route_210_enter_celestic_town_pokemon_center: black screen not detected!",
                COLOR_RED);
            return false;
        }else if (ret == 0){
            return true;
        }
    }
    return true;
}

static bool heal_at_celestic_pokemon_center(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats){
    stream.log("heal_at_celestic_pokemon_center: healing pokemon");
    context.wait_for_all_requests();
    return heal_pokemon(stream, context, "Celestic", &stats);
}

static bool
celestic_town_drama(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("celestic_town_drama: continuing the drama.");

    pbf_move_left_joystick(context, {-1, 0}, 2000ms, 100ms); // 10+

    {
        ShortDialogWatcher talk_to_cynthias_grandma(COLOR_BLUE);
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {0, +1}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{talk_to_cynthias_grandma}});
        if (ret < 0){
            stream.log("celestic_town_talk_to_cynthias_grandma: dialog box not detected!", COLOR_RED);
            return false;
        }
        if (ret == 0){
            stream.log("celestic_town_talk_to_cynthias_grandma: talking to Cynthia's grandma!");
            clear_post_battle_dialogue(stream, context, BUTTON_B, 6000ms);
        };
    }

    context.wait_for_all_requests();
    pbf_wait(context, 1500ms);

    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 3);
    pbf_move_left_joystick(context, {0, +1}, 3600ms, 100ms); // 18+
    pbf_move_left_joystick(context, {-1, 0}, 200ms, 100ms);  // 1

    {
        ShortDialogWatcher talk_to_the_grunt(COLOR_BLUE);
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {-1, 0}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{talk_to_the_grunt}});
        if (ret < 0){
            stream.log("celestic_town_talk_to_the_grunt: dialog box not detected!", COLOR_RED);
            return false;
        }
        if (ret == 0){
            stream.log("celestic_town_talk_to_the_grunt: talking to the grunt!");

            BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            int ret1 = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 200000ms);
                },
                {{black_screen}});
            if (ret1 < 0){
                stream.log("celestic_town_talk_to_the_grunt: Didn't detect start of battle with Clint!", COLOR_RED);
                return false;
            }
            stream.log("celestic_town_talk_to_the_grunt: starting the battle.");
            fight_trainer_using_moves(stream, context, stats, "celestic_town_galactic_grunt", true);
            clear_post_battle_dialogue(stream, context, BUTTON_B, 4000ms);
        };
    }

    context.wait_for_all_requests();
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 20000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("celestic_town_enter_cave: black screen not detected!", COLOR_RED);
            return false;
        }
    }

    return true;
}

static bool discover_the_paintings(VideoStream& stream, ProControllerContext& context){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("discover_the_paintings: going to the paintings.");

    pbf_move_left_joystick(context, {0, +1}, 1600ms, 100ms); // 8+
    pbf_move_left_joystick(context, {-1, 0}, 1200ms, 100ms); // 6+
    pbf_move_left_joystick(context, {0, +1}, 1400ms, 100ms); // 7+
    pbf_move_left_joystick(context, {+1, 0}, 400ms, 100ms);  // 2+
    pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms);  // 2+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 3);

    {
        ShortDialogWatcher talk_to_cynthias_grandma(COLOR_BLUE);
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {0, +1}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{talk_to_cynthias_grandma}});
        if (ret < 0){
            stream.log("discover_the_paintings_talk_to_cynthias_grandma: dialog box not detected!", COLOR_RED);
            return false;
        }
        if (ret == 0){
            stream.log("discover_the_paintings_talk_to_cynthias_grandma: talking to the grandma!");
            clear_post_battle_dialogue(stream, context, BUTTON_B, 6000ms);
        };
    }
    context.wait_for_all_requests();
    pbf_wait(context, 5000ms);

    pbf_move_left_joystick(context, {-1, 0}, 600ms, 100ms);  // 3+
    pbf_move_left_joystick(context, {0, -1}, 400ms, 100ms);  // 2+
    pbf_move_left_joystick(context, {-1, 0}, 400ms, 100ms);  // 2+
    pbf_move_left_joystick(context, {0, -1}, 1400ms, 100ms); // 7+

    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 6);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, -1}, 20000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("discover_the_paintings: black screen for exiting the building not detected!", COLOR_RED);
            return false;
        }
    }

    return true;
}

static bool talk_to_cyrus_and_fly_hearthome(VideoStream& stream, ProControllerContext& context){
    // The cave exit can start dialogue before the checkpoint can save.
    pbf_press_button(context, BUTTON_L, 80ms, 300ms);
    pbf_press_button(context, BUTTON_L, 80ms, 300ms);
    context.wait_for_all_requests();
    wait_for_dialogue(stream, context, "Cyrus", 10000ms);
    clear_post_battle_dialogue(stream, context, BUTTON_B, 6000ms);

    stream.log("[After Celestic cave] Flying to Hearthome after Cyrus's conversation.");
    if (!fly_to(stream, context, FlyPoint::HearthomeCity)){
        return false;
    }

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 20000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("talk_to_cyrus_and_fly_hearthome: black screen for entering pokemon center not detected!",
                COLOR_RED);
            return false;
        }
    }
    return true;
}

static bool heal_at_hearthome_pokemon_center(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats){
    stream.log("heal_at_hearthome_pokemon_center: healing pokemon");
    context.wait_for_all_requests();
    return heal_pokemon(stream, context, "Hearthome", &stats);
}

static bool get_into_hearthome_gym(VideoStream& stream, ProControllerContext& context){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("get_into_hearthome_gym: getting into the gym.");

    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 1);
    pbf_move_left_joystick(context, {+1, 0}, 7000ms, 100ms); // 38+
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 4);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 20000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("get_into_hearthome_gym: black screen not detected!", COLOR_RED);
            return false;
        }
    }

    return true;
}

// Start inside Hearthome Gym at its entrance; finish outside with Cynthia dialogue cleared.
static void hearthome_gym_puzzle_and_cynthia(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryStats& stats){
    VideoStream& stream = env.console;
    struct Move {
        DpadPosition dir;
        int count;
        Milliseconds delay;
    };

    // Trial: use the supplied counts without an extra facing press.
    const Move sequence1[] = {
        {DPAD_UP, 7, 0ms},
        {DPAD_UP, 1, 5000ms},
        {DPAD_RIGHT, 6, 0ms},
        {DPAD_UP, 8, 0ms},
        {DPAD_UP, 1, 3500ms},
        {DPAD_RIGHT, 2, 0ms},
        {DPAD_UP, 9, 0ms},
        {DPAD_LEFT, 1, 0ms},
        {DPAD_LEFT, 1, 5000ms},
        {DPAD_RIGHT, 2, 0ms},
        {DPAD_UP, 9, 0ms},
        {DPAD_LEFT, 2, 0ms},
        {DPAD_UP, 1, 0ms},
        {DPAD_UP, 1, 3500ms},
        {DPAD_RIGHT, 2, 0ms},
        {DPAD_UP, 9, 0ms},
        {DPAD_LEFT, 1, 0ms},
        {DPAD_LEFT, 1, 5000ms},
        {DPAD_LEFT, 6, 0ms},
        {DPAD_UP, 8, 0ms},
        {DPAD_UP, 1, 3500ms},
        {DPAD_RIGHT, 2, 0ms},
        {DPAD_UP, 9, 0ms},
        {DPAD_LEFT, 1, 0ms},
        {DPAD_LEFT, 1, 5000ms},
        {DPAD_RIGHT, 6, 0ms},
        {DPAD_UP, 8, 0ms},
        {DPAD_UP, 1, 3500ms},
        {DPAD_UP, 6, 0ms},
    };

    stream.log("[Hearthome Gym puzzle] Approaching Fantina.");
    for (const auto& move : sequence1){
        // repeat_dpad(context, dpad, move.dir, 80ms, 300ms, move.count, true);
        for (int press = 0; press < move.count; press++){
            pbf_press_dpad(context, move.dir, 80ms, 300ms);
            if (move.delay > 0ms){
                pbf_wait(context, move.delay);
                context.wait_for_all_requests();
                BlackScreenWatcher visible(COLOR_RED,
                    {0.1, 0.1, 0.8, 0.8},
                    100,
                    10,
                    BlackScreenWatcher::FinderType::GONE,
                    500ms);
                if (wait_until(stream, context, 15s, {{visible}}) != 0){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Hearthome Gym: Room/lift transition did not finish.",
                        stream);
                }
                OverworldWatcher ready;
                if (wait_until(stream, context, 15s, {{ready}}) != 0){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Hearthome Gym: Overworld not ready after the recorded room/lift wait.",
                        stream);
                }
            }
        }
    }
    context.wait_for_all_requests();
    pbf_press_button(context, BUTTON_A, 150ms, 300ms);
    fight_trainer_using_moves(stream, context, stats, "Fantina", true);
    clear_post_battle_dialogue(stream, context, BUTTON_B, 6000ms);

    const Move sequence2[] = {
        {DPAD_DOWN, 6, 0ms},
        {DPAD_DOWN, 1, 3500ms},
        {DPAD_DOWN, 8, 0ms},
        {DPAD_LEFT, 6, 0ms},
        {DPAD_DOWN, 1, 0ms},
        {DPAD_DOWN, 1, 5000ms},
        {DPAD_LEFT, 2, 0ms},
        {DPAD_DOWN, 8, 0ms},
        {DPAD_RIGHT, 2, 0ms},
        {DPAD_DOWN, 1, 0ms},
        {DPAD_DOWN, 1, 3500ms},
        {DPAD_DOWN, 8, 0ms},
        {DPAD_RIGHT, 6, 0ms},
        {DPAD_DOWN, 1, 0ms},
        {DPAD_DOWN, 1, 5000ms},
        {DPAD_LEFT, 2, 0ms},
        {DPAD_DOWN, 8, 0ms},
        {DPAD_RIGHT, 2, 0ms},
        {DPAD_DOWN, 1, 0ms},
        {DPAD_DOWN, 1, 3500ms},
        {DPAD_LEFT, 2, 0ms},
        {DPAD_DOWN, 10, 0ms},
        {DPAD_RIGHT, 1, 0ms},
        {DPAD_RIGHT, 1, 5000ms},
        {DPAD_LEFT, 2, 0ms},
        {DPAD_DOWN, 8, 0ms},
        {DPAD_RIGHT, 2, 0ms},
        {DPAD_DOWN, 1, 0ms},
        {DPAD_DOWN, 1, 3500ms},
        {DPAD_DOWN, 8, 0ms},
        {DPAD_LEFT, 6, 0ms},
        {DPAD_DOWN, 1, 0ms},
        {DPAD_DOWN, 1, 5000ms},
        {DPAD_DOWN, 8, 0ms},
    };

    stream.log("[Hearthome Gym puzzle] Returning from Fantina.");
    for (size_t move_index = 0; move_index + 1 < sizeof(sequence2) / sizeof(sequence2[0]); move_index++){
        const auto& move = sequence2[move_index];
        for (int press = 0; press < move.count; press++){
            pbf_press_dpad(context, move.dir, 80ms, 300ms);
            if (move.delay > 0ms){
                pbf_wait(context, move.delay);
                context.wait_for_all_requests();
                BlackScreenWatcher visible(COLOR_RED,
                    {0.1, 0.1, 0.8, 0.8},
                    100,
                    10,
                    BlackScreenWatcher::FinderType::GONE,
                    500ms);
                if (wait_until(stream, context, 15s, {{visible}}) != 0){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Hearthome Gym: Room/lift transition did not finish.",
                        stream);
                }
                OverworldWatcher ready;
                if (wait_until(stream, context, 15s, {{ready}}) != 0){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Hearthome Gym return: Overworld not ready after the recorded room/lift wait.",
                        stream);
                }
            }
        }
    }
    context.wait_for_all_requests();
    BlackScreenWatcher exit_black(COLOR_RED);
    if (run_until<ProControllerContext>(stream,
        context,
        [&](ProControllerContext& ctx){
                const auto& move = sequence2[sizeof(sequence2) / sizeof(sequence2[0]) - 1];
                for (int press = 0; press < move.count; press++){
                    pbf_press_dpad(ctx, move.dir, 80ms, 300ms);
                }
                pbf_wait(ctx, 5000ms);
            },
        {{exit_black}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Hearthome Gym return: Exit transition did not start.",
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
            "Hearthome Gym return: Exit transition did not finish.",
            stream);
    }
    context.wait_for_all_requests();
    wait_for_dialogue(stream, context, "Cynthia after Hearthome Gym", 30000ms);
    clear_post_battle_dialogue(stream, context, BUTTON_B, 6000ms);
    context.wait_for_all_requests();
    stream.log(
        "[Hearthome Gym puzzle] Fantina defeated; outside the Gym with Cynthia's dialogue finished.",
        COLOR_GREEN);
}

// ---------------------------------------------------------------------------
//  Segment wrappers
// ---------------------------------------------------------------------------

std::string AutoStory_Segment_05::name() const {
    return "05: Pastoria to Hearthome";
}

std::string AutoStory_Segment_05::start_text() const {
    return "Start: Outside Pastoria Gym at the doorway, facing south, with fourth-badge rewards and dialogue finished.";
}

std::string AutoStory_Segment_05::end_text() const {
    return "Outside Hearthome Gym at the post-Cynthia position, with her dialogue finished and the fifth badge acquired.";
}

void AutoStory_Segment_05::run_segment(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);
    AutoStory_Checkpoint_0501().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0502().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0503().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0504().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0505().run_checkpoint(env, context, options, stats);
    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}

// ---------------------------------------------------------------------------
//  Checkpoint wrappers
// ---------------------------------------------------------------------------

std::string AutoStory_Checkpoint_0501::name() const {
    return "05.01 - Pastoria recovery";
}

std::string AutoStory_Checkpoint_0501::start_text() const {
    return "Outside Pastoria Gym at the doorway, facing south, with fourth-badge rewards and dialogue finished.";
}

std::string AutoStory_Checkpoint_0501::end_text() const {
    return "Healed and outside Pastoria Pokemon Center at the doorway.";
}

void AutoStory_Checkpoint_0501::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0501(env, context, options, stats);
}

void checkpoint_0501(SingleSwitchProgramEnvironment& env,
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
                if (!go_to_pastoria_pokemon_center(env.console, context)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "go_to_pastoria_pokemon_center: transition not detected.",
                        env.console);
                }
            }
            {
                if (!exit_pastoria_pokemon_center(env.console, context, stats)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "exit_pastoria_pokemon_center: transition not detected.",
                        env.console);
                }
            }
            context.wait_for_all_requests();
        },
        true);
}

std::string AutoStory_Checkpoint_0502::name() const {
    return "05.02 - Galactic chase and Solaceon";
}

std::string AutoStory_Checkpoint_0502::start_text() const {
    return "Healed and outside Pastoria Pokemon Center at the doorway.";
}

std::string AutoStory_Checkpoint_0502::end_text() const {
    return "Healed and outside Solaceon Pokemon Center after the Galactic chase and Cynthia dialogue.";
}

void AutoStory_Checkpoint_0502::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0502(env, context, options, stats);
}

void checkpoint_0502(SingleSwitchProgramEnvironment& env,
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
                if (!pastoria_city_drama(env.console, context, stats)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "pastoria_city_drama: transition not detected.",
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
                if (!go_through_route_213_chasing(env.console, context, stats)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "go_through_route_213_chasing: transition not detected.",
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
            {
                if (!go_through_route_214(env.console, context, stats)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "go_through_route_214: transition not detected.",
                        env.console);
                }
            }
            {
                if (!go_inside_solaceon_pokemon_center(env.console, context)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "go_inside_solaceon_pokemon_center: transition not detected.",
                        env.console);
                }
            }
            {
                if (!heal_at_solaceon_pokemon_center(env.console, context, stats)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "heal_at_solaceon_pokemon_center: transition not detected.",
                        env.console);
                }
            }
            context.wait_for_all_requests();
        },
        true);
}

std::string AutoStory_Checkpoint_0503::name() const {
    return "05.03 - Route 210 to Celestic";
}

std::string AutoStory_Checkpoint_0503::start_text() const {
    return "Healed and outside Solaceon Pokemon Center after the Galactic chase and Cynthia dialogue.";
}

std::string AutoStory_Checkpoint_0503::end_text() const {
    return "Healed and outside Celestic Pokemon Center after the Psyducks and Cynthia follow-up.";
}

void AutoStory_Checkpoint_0503::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0503(env, context, options, stats);
}

void checkpoint_0503(SingleSwitchProgramEnvironment& env,
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
                if (!go_to_the_psyduck_route_210(env.console, context, stats)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "go_to_the_psyduck_route_210: transition not detected.",
                        env.console);
                }
            }
            {
                if (!heal_at_celestic_pokemon_center(env.console, context, stats)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "heal_at_celestic_pokemon_center: transition not detected.",
                        env.console);
                }
            }
            context.wait_for_all_requests();
        },
        true);
}

std::string AutoStory_Checkpoint_0504::name() const {
    return "05.04 - Celestic story and Hearthome";
}

std::string AutoStory_Checkpoint_0504::start_text() const {
    return "Healed and outside Celestic Pokemon Center after the Psyducks and Cynthia follow-up.";
}

std::string AutoStory_Checkpoint_0504::end_text() const {
    return "Healed and outside Hearthome Pokemon Center after the paintings and Cyrus dialogue.";
}

void AutoStory_Checkpoint_0504::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0504(env, context, options, stats);
}

void checkpoint_0504(SingleSwitchProgramEnvironment& env,
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
                if (!celestic_town_drama(env.console, context, stats)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "celestic_town_drama: transition not detected.",
                        env.console);
                }
            }
            {
                if (!discover_the_paintings(env.console, context)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "discover_the_paintings: transition not detected.",
                        env.console);
                }
            }
            {
                if (!talk_to_cyrus_and_fly_hearthome(env.console, context)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "talk_to_cyrus_and_fly_hearthome: transition not detected.",
                        env.console);
                }
            }
            {
                if (!heal_at_hearthome_pokemon_center(env.console, context, stats)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "heal_at_hearthome_pokemon_center: transition not detected.",
                        env.console);
                }
            }
            context.wait_for_all_requests();
        },
        true);
}

std::string AutoStory_Checkpoint_0505::name() const {
    return "05.05 - Hearthome Gym and Cynthia";
}

std::string AutoStory_Checkpoint_0505::start_text() const {
    return "Healed and outside Hearthome Pokemon Center after the paintings and Cyrus dialogue.";
}

std::string AutoStory_Checkpoint_0505::end_text() const {
    return "Outside Hearthome Gym at the post-Cynthia position, with her dialogue finished and the fifth badge acquired.";
}

void AutoStory_Checkpoint_0505::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0505(env, context, options, stats);
}

void checkpoint_0505(SingleSwitchProgramEnvironment& env,
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
                if (!get_into_hearthome_gym(env.console, context)){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "get_into_hearthome_gym: transition not detected.",
                        env.console);
                }
            }
            hearthome_gym_puzzle_and_cynthia(env, context, stats);
            context.wait_for_all_requests();
        },
        true);
}

}
}
}
