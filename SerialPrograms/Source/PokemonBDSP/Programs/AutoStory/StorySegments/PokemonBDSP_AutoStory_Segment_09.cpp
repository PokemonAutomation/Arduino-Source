/*  BDSP AutoStory - Segment 09
 *  From after acquiring the eighth badge until defeating the Elite Four and Champion.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/VisualDetectors/ImageMatchDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_MarkFinder.h"
#include "PokemonBDSP/Inference/PokemonBDSP_ShopDetectors.h"
#include "../PokemonBDSP_AutoStory.h"
#include "../PokemonBDSP_AutoStoryTools.h"
#include "../Utils/PokemonBDSP_AutoStory_Battle.h"
#include "PokemonBDSP_AutoStory_Segment_09.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


std::string AutoStory_Segment_09::name()       const{ return "09: Eighth Badge to Champion"; }
std::string AutoStory_Segment_09::start_text() const{ return "Start: Eighth Gym Badge acquired (Sunyshore City)."; }
std::string AutoStory_Segment_09::end_text()   const{ return "End: Elite Four and Champion defeated, credits roll."; }

void AutoStory_Segment_09::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
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
    AutoStory_Checkpoint_0910().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0911().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0912().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0913().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0914().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}

static void move_back_after_unexpected_battle(
    VideoStream& stream,
    ProControllerContext& context,
    const std::string& trainerid
){
    stream.log("Finishing battle with " + trainerid, COLOR_GREEN);
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    if (trainerid == "route_223_swimmer_miranda") {
        pbf_move_left_joystick(context, {0, +1}, 3800ms, 100ms); // 19+
    } else if (trainerid == "route_223_swimmer_crystal") {
        pbf_move_left_joystick(context, {0, +1}, 1200ms, 100ms); // 5+
    } else if (trainerid == "route_223_swimmer_gabrielle") {
        pbf_move_left_joystick(context, {-1, 0}, 600ms, 100ms); // 2+
    } else if (trainerid == "route_223_swimmer_troy") {
        pbf_move_left_joystick(context, {0, +1}, 800ms, 100ms); // 3+
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
        trainerid == "victory_road_ace_mariah" or
        trainerid == "victory_road_blackbelt_miles" or
        trainerid == "victory_road_dragontamer_andre" or
        trainerid == "victory_road_veteran_edgar"
    ) {
        // Select Bulldoze
        pbf_press_dpad(context, DPAD_DOWN, 280ms, 200ms);
    } else if (
        trainerid == "route_223_swimmer_miranda" or
        trainerid == "route_223_swimmer_oscar" or
        trainerid == "route_223_swimmer_crystal" or
        trainerid == "route_223_swimmer_gabrielle" or
        trainerid == "route_223_swimmer_troy" or
        trainerid == "victory_road_ace_sydney" or
        trainerid == "victory_road_psychic_valencia"
    ) {
        // Select Razor leaf
        pbf_press_dpad(context, DPAD_UP, 280ms, 200ms);
    } else if (
        trainerid == "victory_road_ace_omar" or
        trainerid == "victory_road_veteran_clayton" or
        trainerid == "victory_road_dragontamer_clinton"
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

static bool enter_sunyshore_pokemon_center(
    VideoStream& stream,
    ProControllerContext& context,
    AutoStoryOptions options
){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("enter_sunyshore_pokemon_center: flying to the Pokemon Center and entering...");

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
            stream.log("enter_sunyshore_pokemon_center: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("enter_sunyshore_pokemon_center: entered the Pokemon Center.", COLOR_GREEN);
    }

    return true;
}

static bool go_to_victory_road_pokemon_center(
    VideoStream& stream,
    ProControllerContext& context,
    AutoStoryOptions options
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_to_victory_road_pokemon_center: going through Victory Road's outdoor and heading to the Pokemon Center...");

    // Going to the lady who gives you HM Waterfall

    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 5);
    pbf_move_left_joystick(context, {0, +1}, 3400ms, 100ms); // 17+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 4);
    pbf_move_left_joystick(context, {0, +1}, 1200ms, 100ms); // 6+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 5);

    {
        MarkDetector jasmine_dialog(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 6800ms, 100ms); // 34+
            },
            {{jasmine_dialog}}
        );
        if (ret == 0){
            stream.log("go_to_victory_road_pokemon_center: talking to Jasmine.", COLOR_GREEN);
            context.wait_for_all_requests();
            pbf_wait(context, 2000ms);
            pbf_mash_button(context, BUTTON_B, 14000ms);
        } else if (ret < 0) {
            stream.log("go_to_victory_road_pokemon_center: Jasmine dialog box not detected!", COLOR_RED);
            return false;
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {-1, 0}, 600ms, 100ms); // 2+
    pbf_move_left_joystick(context, {0, +1}, 1200ms, 100ms); // 6+

    use_HM(stream, context, "Surf");
    context.wait_for_all_requests();
    
    activate_repel(stream, context);
    context.wait_for_all_requests();

    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 3);
    pbf_move_left_joystick(context, {-1, 0}, 1800ms, 100ms); // 9+
    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 1);

    {
        MarkDetector route_223_swimmer_miranda_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 5800ms, 100ms); // 29+
            },
            {{route_223_swimmer_miranda_fight}}
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
                stream.log("go_to_victory_road_pokemon_center, route_223_swimmer_miranda: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "route_223_swimmer_miranda");
                context.wait_for_all_requests();
                pbf_wait(context, 3000ms);
                move_back_after_unexpected_battle(stream, context, "route_223_swimmer_miranda");
            }
        }
    }

    context.wait_for_all_requests();
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 1);

    {
        MarkDetector route_223_swimmer_oscar_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 5800ms, 100ms); // 29+
            },
            {{route_223_swimmer_oscar_fight}}
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
                stream.log("go_to_victory_road_pokemon_center, route_223_swimmer_oscar: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "route_223_swimmer_oscar");
            }
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    pbf_move_left_joystick(context, {0, +1}, 1800ms, 100ms); // 9+
    pbf_move_left_joystick(context, {+1, 0}, 1800ms, 100ms); // 9+

    {
        MarkDetector route_223_swimmer_crystal_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 5600ms, 100ms); // 28+
            },
            {{route_223_swimmer_crystal_fight}}
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
                stream.log("go_to_victory_road_pokemon_center, route_223_swimmer_crystal: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "route_223_swimmer_crystal");
                context.wait_for_all_requests();
                pbf_wait(context, 3000ms);
                move_back_after_unexpected_battle(stream, context, "route_223_swimmer_crystal");
            }
        }
    }

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
            stream.log("go_to_victory_road_pokemon_center: Reactivating repel.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 3000ms);
        } else if (ret < 0){
                stream.log("go_to_victory_road_pokemon_center: Didn't detect repel dialog box (1)!", COLOR_RED);
                return false;
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    pbf_move_left_joystick(context, {-1, 0}, 2000ms, 100ms); // 10+
    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 2);
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 4);
    pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms); // 3+
    pbf_move_left_joystick(context, {0, +1}, 1600ms, 100ms); // 4++ (getting out of the water)
    pbf_move_left_joystick(context, {-1, 0}, 1000ms, 100ms); // 4+
    pbf_move_left_joystick(context, {0, -1}, 800ms, 100ms); // 3+

    context.wait_for_all_requests();
    use_HM(stream, context, "Surf");

    context.wait_for_all_requests();
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 5);
    pbf_move_left_joystick(context, {0, +1}, 3000ms, 100ms); // 10++ (getting out of the water)

    context.wait_for_all_requests();
    use_HM(stream, context, "Surf");

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms); // 1+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 5);

    {
        MarkDetector route_223_swimmer_gabrielle_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                DpadState dpad;
                dpad.last_dir = DPAD_RIGHT;
                repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 4);
                pbf_move_left_joystick(context, {-1, 0}, 600ms, 100ms); // 2+
                context.wait_for_all_requests();
                pbf_wait(context, 2000ms);
                context.wait_for_all_requests();
            },
            {{route_223_swimmer_gabrielle_fight}}
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
                stream.log("go_to_victory_road_pokemon_center, route_223_swimmer_gabrielle: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "route_223_swimmer_gabrielle");
                context.wait_for_all_requests();
                pbf_wait(context, 3000ms);
                move_back_after_unexpected_battle(stream, context, "route_223_swimmer_gabrielle");
            }
        }
    }

    context.wait_for_all_requests();

    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 3);
    pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms); // 3+
    pbf_move_left_joystick(context, {0, +1}, 1400ms, 100ms); // 7+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 5);

    {
        MarkDetector route_223_swimmer_troy_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 2400ms, 100ms); // 12+
                context.wait_for_all_requests();
                pbf_wait(context, 1000ms);
                context.wait_for_all_requests();
            },
            {{route_223_swimmer_troy_fight}}
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
                stream.log("go_to_victory_road_pokemon_center, route_223_swimmer_troy: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "route_223_swimmer_troy");
                context.wait_for_all_requests();
                pbf_wait(context, 3000ms);
                move_back_after_unexpected_battle(stream, context, "route_223_swimmer_troy");
            }
        }
    }

    context.wait_for_all_requests();

    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 5);
    pbf_move_left_joystick(context, {0, +1}, 3000ms, 100ms); // 15+

    context.wait_for_all_requests();

    use_HM(stream, context, "Waterfall");

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);
    pbf_move_left_joystick(context, {0, +1}, 2000ms, 100ms); // 5++ (getting out of the water)

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
            stream.log("go_to_victory_road_pokemon_center: Not reactivating repel.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_B, 3000ms);
        } else if (ret < 0){
                stream.log("go_to_victory_road_pokemon_center: Didn't detect repel dialog box (1)!", COLOR_RED);
                return false;
        }
    }

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {-1, 0}, 2000ms, 100ms); // 10+

    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 1);
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
            stream.log("go_to_victory_road_pokemon_center: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_to_victory_road_pokemon_center: entered the Pokemon Center.", COLOR_GREEN);
    }

    return true;
}

static bool heal_and_exit(
    VideoStream& stream,
    ProControllerContext& context,
    const std::string& label
){
    return heal_pokemon(stream, context, label);
}

static bool enter_victory_road_cave(
    VideoStream& stream,
    ProControllerContext& context,
    AutoStoryOptions options
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("enter_victory_road_cave: entering the cave...");

    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 9);

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
            stream.log("enter_victory_road_cave: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("enter_victory_road_cave: entered the Victory Road cave.", COLOR_GREEN);
    }

    return true;
}

static bool go_through_victory_road_cave(
    VideoStream& stream,
    ProControllerContext& context,
    AutoStoryOptions options
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_through_victory_road_cave: going through Victory Road's cave and heading to the Elite 4...");

    activate_repel(stream, context);
    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {0, +1}, 3200ms, 100ms); // 16+
    pbf_move_left_joystick(context, {+1, 0}, 3800ms, 100ms); // 19+
    pbf_move_left_joystick(context, {0, +1}, 3000ms, 100ms); // 15+
    pbf_move_left_joystick(context, {+1, 0}, 600ms, 100ms); // 2+
    pbf_move_left_joystick(context, {0, -1}, 400ms, 100ms); // 1+

    use_HM(stream, context, "Rock Climb");

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    pbf_move_left_joystick(context, {0, -1}, 400ms, 100ms); // 1+
    pbf_move_left_joystick(context, {-1, 0}, 2200ms, 100ms); // 11+
    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 2);
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 12);
    pbf_move_left_joystick(context, {0, +1}, 2600ms, 100ms); // 13+

    {
        MarkDetector victory_road_ace_mariah_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 5800ms, 100ms); // 29+
            },
            {{victory_road_ace_mariah_fight}}
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
                stream.log("go_through_victory_road_cave, victory_road_ace_mariah: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "victory_road_ace_mariah");
            }
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);
    pbf_move_left_joystick(context, {-1, 0}, 400ms, 100ms); // 1+

    {
        ShortDialogWatcher repel_dialog(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 2000ms, 100ms); // 10+
                pbf_move_left_joystick(context, {0, -1}, 2000ms, 100ms); // 10+
            },
            {{repel_dialog}}
        );
        if (ret == 0){
            stream.log("go_through_victory_road_cave: Reactivating repel.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 3000ms);
        } else if (ret < 0){
                stream.log("go_through_victory_road_cave: Didn't detect repel dialog box (1)!", COLOR_RED);
                return false;
        }
    }

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, -1}, 2000ms, 100ms); // 10+
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 7);

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
            stream.log("go_through_victory_road_cave: black screen not detected (1)!", COLOR_RED);
            return false;
        }
        stream.log("go_through_victory_road_cave: entered the second part of the cave.", COLOR_GREEN);
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {-1, 0}, 1000ms, 100ms); // 4+
    pbf_move_left_joystick(context, {0, -1}, 600ms, 100ms); // 2+

    context.wait_for_all_requests();
    use_HM(stream, context, "Rock Smash");

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, -1}, 800ms, 100ms); // 3+
    pbf_move_left_joystick(context, {+1, 0}, 400ms, 100ms); // 1+
    pbf_move_left_joystick(context, {0, -1}, 1200ms, 100ms); // 6+

    context.wait_for_all_requests();
    use_HM(stream, context, "Strength");

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, -1}, 1500ms, 100ms); // 1++ (move boulders)
    pbf_move_left_joystick(context, {-1, 0}, 1500ms, 100ms); // 1++ (move boulders)
    pbf_move_left_joystick(context, {0, -1}, 4000ms, 100ms); // 3++ (move boulders)

    {
        MarkDetector victory_road_ace_omar_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 5800ms, 100ms); // 29+
            },
            {{victory_road_ace_omar_fight}}
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
                stream.log("go_through_victory_road_cave, victory_road_ace_omar: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "victory_road_ace_omar");
            }
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {0, -1}, 1000ms, 100ms); // 1++ (move boulders)
    pbf_move_left_joystick(context, {-1, 0}, 600ms, 100ms); // 2+
    pbf_move_left_joystick(context, {0, -1}, 800ms, 100ms); // 3+
    pbf_move_left_joystick(context, {+1, 0}, 600ms, 100ms); // 2+
    pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms); // 1+

    context.wait_for_all_requests();
    use_HM(stream, context, "Rock Smash");

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, +1}, 4000ms, 100ms); // 2++ (move boulders)
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 1);
    pbf_move_left_joystick(context, {+1, 0}, 1500ms, 100ms); // 4++ (move after boulder moving)
    pbf_move_left_joystick(context, {0, -1}, 600ms, 100ms); // 2+

    context.wait_for_all_requests();
    use_HM(stream, context, "Rock Smash");

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, -1}, 600ms, 100ms); // 2+
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 2);

    {
        MarkDetector victory_road_ace_sydney_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 5800ms, 100ms); // 29+
            },
            {{victory_road_ace_sydney_fight}}
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
                stream.log("go_through_victory_road_cave, victory_road_ace_sydney: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "victory_road_ace_sydney");
            }
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {0, +1}, 600ms, 100ms); // 2+
    pbf_move_left_joystick(context, {+1, 0}, 2000ms, 100ms); // 10+

    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 1);
    pbf_move_left_joystick(context, {+1, 0}, 1500ms, 100ms); // 2++ (move boulders)

    {
        MarkDetector victory_road_veteran_clayton_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 5800ms, 100ms); // 29+
            },
            {{victory_road_veteran_clayton_fight}}
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
                stream.log("go_through_victory_road_cave, victory_road_veteran_clayton: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "victory_road_veteran_clayton");
            }
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {-1, 0}, 400ms, 100ms); // 1+

    {
        ShortDialogWatcher repel_dialog(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 2000ms, 100ms); // 10+
                pbf_move_left_joystick(context, {0, -1}, 2000ms, 100ms); // 10+
            },
            {{repel_dialog}}
        );
        if (ret == 0){
            stream.log("go_through_victory_road_cave: Reactivating repel.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 3000ms);
        } else if (ret < 0){
                stream.log("go_through_victory_road_cave: Didn't detect repel dialog box (1)!", COLOR_RED);
                return false;
        }
    }

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, +1}, 1800ms, 100ms); // 9+

    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 1);

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
    pbf_move_left_joystick(context, {-1, 0}, 1200ms, 100ms); // 6+
    pbf_move_left_joystick(context, {0, +1}, 2400ms, 100ms); // 12+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 1);

    {
        MarkDetector victory_road_blackbelt_miles_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 5800ms, 100ms); // 29+
            },
            {{victory_road_blackbelt_miles_fight}}
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
                stream.log("go_through_victory_road_cave, victory_road_blackbelt_miles: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "victory_road_blackbelt_miles");
            }
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

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
            stream.log("go_through_victory_road_cave: black screen not detected (3)!", COLOR_RED);
            return false;
        }
        stream.log("go_through_victory_road_cave: entered the fourth part of the cave.", COLOR_GREEN);
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 1);
    pbf_move_left_joystick(context, {+1, 0}, 2400ms, 100ms); // 12+
    pbf_move_left_joystick(context, {0, +1}, 1000ms, 100ms); // 4+

    context.wait_for_all_requests();
    use_HM(stream, context, "Surf");

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, +1}, 1000ms, 100ms); // 4+
    pbf_move_left_joystick(context, {+1, 0}, 800ms, 100ms); // 3+

    {
        MarkDetector victory_road_psychic_valencia_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 5800ms, 100ms); // 29+
            },
            {{victory_road_psychic_valencia_fight}}
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
                stream.log("go_through_victory_road_cave, victory_road_psychic_valencia: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "victory_road_psychic_valencia");
            }
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms); // 1+

    context.wait_for_all_requests();
    use_HM(stream, context, "Surf");

    context.wait_for_all_requests();

    {
        ShortDialogWatcher repel_dialog(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 2000ms, 100ms); // 6+
                pbf_move_left_joystick(context, {-1, 0}, 2000ms, 100ms); // 6+
                pbf_move_left_joystick(context, {+1, 0}, 2000ms, 100ms); // 6+
                pbf_move_left_joystick(context, {-1, 0}, 2000ms, 100ms); // 6+
            },
            {{repel_dialog}}
        );
        if (ret == 0){
            stream.log("go_through_victory_road_cave: Reactivating repel.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 3000ms);
        } else if (ret < 0){
                stream.log("go_through_victory_road_cave: Didn't detect repel dialog box (2)!", COLOR_RED);
                return false;
        }
    }

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

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 5800ms, 100ms); // 29+
            },
            {{victory_road_dragontamer_andre_fight}}
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
                stream.log("go_through_victory_road_cave, victory_road_dragontamer_andre: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "victory_road_dragontamer_andre");
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
    pbf_move_left_joystick(context, {+1, 0}, 600ms, 100ms); // 2+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 2);

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
            stream.log("go_through_victory_road_cave: black screen not detected (4)!", COLOR_RED);
            return false;
        }
        stream.log("go_through_victory_road_cave: entered the fifth part of the cave.", COLOR_GREEN);
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms); // 1+

    {
        ShortDialogWatcher repel_dialog(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 3000ms, 100ms); // 15+
                pbf_move_left_joystick(context, {+1, 0}, 2000ms, 100ms); // 10+
            },
            {{repel_dialog}}
        );
        if (ret == 0){
            stream.log("go_through_victory_road_cave: Reactivating repel.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 3000ms);
        } else if (ret < 0){
                stream.log("go_through_victory_road_cave: Didn't detect repel dialog box (2)!", COLOR_RED);
                return false;
        }
    }

    pbf_move_left_joystick(context, {-1, 0}, 3000ms, 100ms); // 15+
    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 3);

    {
        MarkDetector victory_road_veteran_edgar_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, -1}, 5800ms, 100ms); // 29+
            },
            {{victory_road_veteran_edgar_fight}}
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
                stream.log("go_through_victory_road_cave, victory_road_veteran_edgar: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "victory_road_veteran_edgar");
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

    {
        MarkDetector victory_road_dragontamer_clinton_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 5800ms, 100ms); // 29+
            },
            {{victory_road_dragontamer_clinton_fight}}
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
                stream.log("go_through_victory_road_cave, victory_road_dragontamer_clinton: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "victory_road_dragontamer_clinton");
            }
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {0, +1}, 5600ms, 100ms); // 28+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 1);

    {
        WhiteScreenOverWatcher white_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 10000ms, 100ms);
            },
            {{white_screen}}
        );
        if (ret < 0){
            stream.log("go_through_victory_road_cave: white screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_victory_road_cave: exited Victory Road's cave.", COLOR_GREEN);
    }

    return true;
}

static bool enter_elite_4_pokemon_center(
    VideoStream& stream,
    ProControllerContext& context,
    AutoStoryOptions options
){
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
            stream.log("enter_elite_4_pokemon_center: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("enter_elite_4_pokemon_center: entered the Elite 4 building.", COLOR_GREEN);
    }

    return true;
}

static bool ready_in_front_of_elite_4(
    VideoStream& stream,
    ProControllerContext& context,
    AutoStoryOptions options
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 6000ms);
    context.wait_for_all_requests();
    stream.log("ready_in_front_of_elite_4: stocked up and healed up, ready for the Elite 4...");

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
            stream.log("ready_in_front_of_elite_4: Not reactivating repel.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_B, 3000ms);
        } else if (ret < 0){
                stream.log("ready_in_front_of_elite_4: Didn't detect repel dialog box (1)!", COLOR_RED);
                return false;
        }
    }

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {-1, 0}, 2000ms, 100ms); // 10+

    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 1);

    pbf_move_left_joystick(context, {0, +1}, 1800ms, 100ms); // 7+
    context.wait_for_all_requests();

    pbf_mash_button(context, BUTTON_A, 5000ms);
    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);
    wait_for_dialogue(stream, context, "Elite 4 Pokemon Center - Pokemon healing");
    mash_until_dialogue_ends(stream, context, BUTTON_B);
    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    pbf_move_left_joystick(context, {+1, 0}, 3200ms, 100ms); // 16+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 3);
    pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms); // 1+


    buy_items(stream, context,
        {
            {ShopItem::FullRestore, BUY_MAX}
        },
        options.language
    );

    return true;
}

static bool get_into_the_elite_4_entrance(
    VideoStream& stream,
    ProControllerContext& context,
    AutoStoryOptions options
){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("get_into_the_elite_4_entrance: entered the Elite 4...");

    return true;
}

static bool deal_with_aaron_elite_4(
    VideoStream& stream,
    ProControllerContext& context,
    AutoStoryOptions options
){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("deal_with_aaron_elite_4: battling Aaron of the Elite 4 and passing the gate...");

    return true;
}

static bool deal_with_bertha_elite_4(
    VideoStream& stream,
    ProControllerContext& context,
    AutoStoryOptions options
){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("deal_with_bertha_elite_4: battling Bertha of the Elite 4 and passing the gate...");

    return true;
}

static bool deal_with_flint_elite_4(
    VideoStream& stream,
    ProControllerContext& context,
    AutoStoryOptions options
){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("deal_with_flint_elite_4: battling Flint of the Elite 4 and passing the gate...");

    return true;
}

static bool deal_with_lucian_elite_4(
    VideoStream& stream,
    ProControllerContext& context,
    AutoStoryOptions options
){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("deal_with_lucian_elite_4: battling Lucian of the Elite 4 and passing the gate...");

    return true;
}

static bool deal_with_cynthia_and_finish(
    VideoStream& stream,
    ProControllerContext& context,
    AutoStoryOptions options
){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("deal_with_cynthia_and_finish: battling Cynthia and finishing the game...");

    return true;
}

void checkpoint_0901(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!enter_sunyshore_pokemon_center(env.console, context, options)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "enter_sunyshore_pokemon_center: transition not detected.", env.console);
            }
        },
        true
    );
}

void checkpoint_0902(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!heal_and_exit(env.console, context, "Sunyshore City")){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "heal_and_exit_1: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_0903(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_to_victory_road_pokemon_center(env.console, context, options)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_to_victory_road_pokemon_center: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_0904(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!heal_and_exit(env.console, context, "Victory Road")){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "heal_and_exit_2: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_0905(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!enter_victory_road_cave(env.console, context, options)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "enter_victory_road_cave: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_0906(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_through_victory_road_cave(env.console, context, options)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_through_victory_road_cave: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_0907(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!enter_elite_4_pokemon_center(env.console, context, options)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "enter_elite_4_pokemon_center: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_0908(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!ready_in_front_of_elite_4(env.console, context, options)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "ready_in_front_of_elite_4: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_0909(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!get_into_the_elite_4_entrance(env.console, context, options)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "get_into_the_elite_4_entrance: transition not detected.", env.console);
            }
        },
        true
    );
}

void checkpoint_0910(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!deal_with_aaron_elite_4(env.console, context, options)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "deal_with_aaron_elite_4: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_0911(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!deal_with_bertha_elite_4(env.console, context, options)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "deal_with_bertha_elite_4: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_0912(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!deal_with_flint_elite_4(env.console, context, options)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "deal_with_flint_elite_4: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_0913(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!deal_with_lucian_elite_4(env.console, context, options)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "deal_with_lucian_elite_4: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_0914(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!deal_with_cynthia_and_finish(env.console, context, options)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "deal_with_cynthia_and_finish: transition not detected.", env.console);
            }
        },
        false
    );
}

std::string AutoStory_Checkpoint_0901::name()       const{ return "09.01 - Leave the gym and fly to the Sunyshore Pokemon Center."; }
std::string AutoStory_Checkpoint_0901::start_text() const{ return "Eighth Gym Badge acquired (Sunyshore City)."; }
std::string AutoStory_Checkpoint_0901::end_text()   const{ return "Enter the Sunyshore Pokemon Center."; }
void AutoStory_Checkpoint_0901::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_0901(env, context, options, stats);
}

std::string AutoStory_Checkpoint_0902::name()       const{ return "09.02 - Heal the Pokemon at the Sunyshore Pokemon Center."; }
std::string AutoStory_Checkpoint_0902::start_text() const{ return "Entered the Sunyshore Pokemon Center."; }
std::string AutoStory_Checkpoint_0902::end_text()   const{ return "Exit the Sunyshore Pokemon Center."; }
void AutoStory_Checkpoint_0902::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_0902(env, context, options, stats);
}


std::string AutoStory_Checkpoint_0903::name()       const{ return "09.03 - Start at the Sunyshore Pokemon Center and go to the Victory Road Pokemon Center."; }
std::string AutoStory_Checkpoint_0903::start_text() const{ return "Exited the Sunyshore Pokemon Center."; }
std::string AutoStory_Checkpoint_0903::end_text()   const{ return "Enter the Victory Road Pokemon Center."; }
void AutoStory_Checkpoint_0903::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_0903(env, context, options, stats);
}

std::string AutoStory_Checkpoint_0904::name()       const{ return "09.04 - Heal at the Victory Road Pokemon Center."; }
std::string AutoStory_Checkpoint_0904::start_text() const{ return "Entered Victory Road Pokemon Center."; }
std::string AutoStory_Checkpoint_0904::end_text()   const{ return "Exit Victory Road Pokemon Center."; }
void AutoStory_Checkpoint_0904::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_0904(env, context, options, stats);
}

std::string AutoStory_Checkpoint_0905::name()       const{ return "09.05 - Enter the Victory Road cave."; }
std::string AutoStory_Checkpoint_0905::start_text() const{ return "Exited Victory Road Pokemon Center."; }
std::string AutoStory_Checkpoint_0905::end_text()   const{ return "Enter the Victory Road cave."; }
void AutoStory_Checkpoint_0905::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_0905(env, context, options, stats);
}

std::string AutoStory_Checkpoint_0906::name()       const{ return "09.06 - Go through the Victory Road cave."; }
std::string AutoStory_Checkpoint_0906::start_text() const{ return "Entered Victory Road cave."; }
std::string AutoStory_Checkpoint_0906::end_text()   const{ return "Exit Victory Road cave."; }
void AutoStory_Checkpoint_0906::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_0906(env, context, options, stats);
}

std::string AutoStory_Checkpoint_0907::name()       const{ return "09.07 - Go through the end of Victory Road into the Elite 4 building"; }
std::string AutoStory_Checkpoint_0907::start_text() const{ return "Exited Victory Road cave."; }
std::string AutoStory_Checkpoint_0907::end_text()   const{ return "Enter the Elite 4 building."; }
void AutoStory_Checkpoint_0907::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_0907(env, context, options, stats);
}

std::string AutoStory_Checkpoint_0908::name()       const{ return "09.08 - Enter the Elite 4 building and get ready to enter."; }
std::string AutoStory_Checkpoint_0908::start_text() const{ return "Entered the Elite 4 building."; }
std::string AutoStory_Checkpoint_0908::end_text()   const{ return "Ready in front of the Elite 4."; }
void AutoStory_Checkpoint_0908::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_0908(env, context, options, stats);
}

std::string AutoStory_Checkpoint_0909::name()       const{ return "09.09 - Enter the first room of the Elite 4."; }
std::string AutoStory_Checkpoint_0909::start_text() const{ return "Readied in front of the Elite 4."; }
std::string AutoStory_Checkpoint_0909::end_text()   const{ return "Enter the Elite 4's first room."; }
void AutoStory_Checkpoint_0909::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_0909(env, context, options, stats);
}

std::string AutoStory_Checkpoint_0910::name()       const{ return "09.10 - Deal with Aaron and go to the second room."; }
std::string AutoStory_Checkpoint_0910::start_text() const{ return "Entered the Elite 4's first room."; }
std::string AutoStory_Checkpoint_0910::end_text()   const{ return "Deal with Aaron and enter the second room."; }
void AutoStory_Checkpoint_0910::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_0910(env, context, options, stats);
}

std::string AutoStory_Checkpoint_0911::name()       const{ return "09.11 - Deal with Bertha and go to the third room."; }
std::string AutoStory_Checkpoint_0911::start_text() const{ return "Dealt with Aaron and entered the second room."; }
std::string AutoStory_Checkpoint_0911::end_text()   const{ return "Deal with Bertha and enter the third room."; }
void AutoStory_Checkpoint_0911::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_0911(env, context, options, stats);
}

std::string AutoStory_Checkpoint_0912::name()       const{ return "09.12 - Deal with Flint and go to the fourth room."; }
std::string AutoStory_Checkpoint_0912::start_text() const{ return "Dealt with Bertha and entered the third room."; }
std::string AutoStory_Checkpoint_0912::end_text()   const{ return "Deal with Flint and enter the fourth room."; }
void AutoStory_Checkpoint_0912::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_0912(env, context, options, stats);
}

std::string AutoStory_Checkpoint_0913::name()       const{ return "09.13 - Deal with Lucian and go to the last room."; }
std::string AutoStory_Checkpoint_0913::start_text() const{ return "Dealt with Flint and entered the fourth room."; }
std::string AutoStory_Checkpoint_0913::end_text()   const{ return "Deal with Lucian and enter the last room."; }
void AutoStory_Checkpoint_0913::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_0913(env, context, options, stats);
}

std::string AutoStory_Checkpoint_0914::name()       const{ return "09.14 - Deal with Cynthia and finish the game."; }
std::string AutoStory_Checkpoint_0914::start_text() const{ return "Dealt with Lucian and entered the last room."; }
std::string AutoStory_Checkpoint_0914::end_text()   const{ return "Deal with Cynthia and finish the game."; }
void AutoStory_Checkpoint_0914::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_0914(env, context, options, stats);
}


}
}
}
