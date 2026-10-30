/*  BDSP AutoStory - Segment 07
 *  From after acquiring the sixth badge until after acquiring the seventh badge.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/FatalProgramException.h"
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
#include "PokemonBDSP/Inference/PokemonBDSP_TransitionWatchers.h"
#include "PokemonBDSP/Programs/AutoStory/PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP/Programs/AutoStory/Utils/PokemonBDSP_AutoStory_Battle.h"
#include "PokemonBDSP/Programs/AutoStory/Utils/PokemonBDSP_AutoStory_Segment_00_tools.h"

#include "PokemonBDSP_AutoStory_Segment_07.h"

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

    if (trainerid == "route_216_ace_maria"){
        pbf_move_left_joystick(context, {-1, 0}, 1600ms, 100ms); // 8
    }else if (trainerid == "route_215_skier_edward"){
        pbf_move_left_joystick(context, {-1, 0}, 8100ms, 100ms); // 27++ (snow)
    }else if (trainerid == "route_217_skier_lexie"){
        pbf_move_left_joystick(context, {0, +1}, 2100ms, 100ms); // 7++ (snow)
    }else if (trainerid == "route_217_blackbelt_luke"){
        pbf_move_left_joystick(context, {0, +1}, 5100ms, 100ms); // 17++ (snow)
    }
}

static bool handle_battle(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats, const std::string& trainerid){
    fight_trainer_using_moves(stream, context, stats, trainerid, true);
    return true;
}

// ---------------------------------------------------------------------------
//  Named route navigation blocks
// ---------------------------------------------------------------------------

static bool leave_gym(VideoStream& stream, ProControllerContext& context){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("leave_gym: leaving the gym...");

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, -1}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("leave_gym: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("leave_gym: exited gym", COLOR_GREEN);
    };
    return true;
}

static bool go_to_the_library(VideoStream& stream, ProControllerContext& context){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_to_the_library: talking to Clint and going to the Library...");

    {
        ShortDialogWatcher talk_to_clint(COLOR_BLUE);
        int ret = wait_until(stream, context, std::chrono::seconds(30), {{talk_to_clint}});
        if (ret < 0){
            stream.log("go_to_the_library_talk_to_clint: dialog box not detected!", COLOR_RED);
            return false;
        }else if (ret == 0){
            stream.log("go_to_the_library_talk_to_clint: dialog box detected.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 7000ms);
            context.wait_for_all_requests();
        }
    }

    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 2);
    pbf_move_left_joystick(context, {-1, 0}, 1600ms, 100ms); // 8+
    pbf_move_left_joystick(context, {0, +1}, 3800ms, 100ms); // 19+
    pbf_move_left_joystick(context, {+1, 0}, 600ms, 100ms);  // 2+
    pbf_move_left_joystick(context, {0, -1}, 1000ms, 100ms); // 4+
    dpad.last_dir = DPAD_DOWN;
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
            stream.log("go_to_the_library: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_to_the_library: entered the library", COLOR_GREEN);
    };
    return true;
}

static bool drama_at_the_library(VideoStream& stream, ProControllerContext& context){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("drama_at_the_library: going to professor Rowan...");

    pbf_move_left_joystick(context, {0, +1}, 1400ms, 100ms); // 7+

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("drama_at_the_library: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("drama_at_the_library: moved to the first floor", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    pbf_move_left_joystick(context, {0, -1}, 600ms, 100ms);  // 3+
    pbf_move_left_joystick(context, {+1, 0}, 1400ms, 100ms); // 7
    pbf_move_left_joystick(context, {0, +1}, 800ms, 100ms);  // 3+

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("drama_at_the_library: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("drama_at_the_library: moved to the second floor", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    {
        ShortDialogWatcher talk_to_clint(COLOR_BLUE);
        int ret = wait_until(stream, context, std::chrono::seconds(30), {{talk_to_clint}});
        if (ret < 0){
            stream.log("drama_at_the_library_talk_to_clint: dialog box not detected!", COLOR_RED);
            return false;
        }else if (ret == 0){
            stream.log("drama_at_the_library_talk_to_clint: dialog box detected.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 45000ms);
            context.wait_for_all_requests();
        }
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {0, +1}, 600ms, 100ms); // 2+
    pbf_move_left_joystick(context, {-1, 0}, 700ms, 100ms); // 5
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
            stream.log("drama_at_the_library: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("drama_at_the_library: moved back to the first floor", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 2);
    pbf_move_left_joystick(context, {-1, 0}, 1400ms, 100ms); // 7
    pbf_move_left_joystick(context, {0, +1}, 800ms, 100ms);  // 3+

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("drama_at_the_library: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("drama_at_the_library: moved back to the ground floor", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, -1}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("drama_at_the_library: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("drama_at_the_library: moved out of the library", COLOR_GREEN);
    };
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("drama_at_the_library: finishing the Rowan follow-up outside the library...");

    {
        ShortDialogWatcher talk_to_rowan(COLOR_BLUE);
        int ret = wait_until(stream, context, std::chrono::seconds(30), {{talk_to_rowan}});
        if (ret < 0){
            stream.log("drama_at_the_library_talk_to_rowan: dialog box not detected!", COLOR_RED);
            return false;
        }else if (ret == 0){
            stream.log("drama_at_the_library_talk_to_rowan: dialog box detected.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_B, 21000ms);
            context.wait_for_all_requests();
        }
    }

    clear_post_battle_dialogue(stream, context, BUTTON_B, 6000ms);
    context.wait_for_all_requests();
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    return true;
}

static bool fly_and_enter_mount_coronet(VideoStream& stream, ProControllerContext& context){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    if (!fly_to(stream, context, FlyPoint::CelesticTown)){
        return false;
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    stream.log("fly_and_enter_mount_coronet: going towards Mt. Coronet...");

    pbf_move_left_joystick(context, {-1, 0}, 2000ms, 100ms); // 10+
    pbf_move_left_joystick(context, {0, -1}, 800ms, 100ms);  // 3+
    pbf_move_left_joystick(context, {-1, 0}, 1800ms, 100ms); // 9+
    pbf_move_left_joystick(context, {0, +1}, 800ms, 100ms);  // 3+
    pbf_move_left_joystick(context, {-1, 0}, 600ms, 100ms);  // 2+

    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 11);
    pbf_move_left_joystick(context, {-1, 0}, 3200ms, 100ms); // 16+
    context.wait_for_all_requests();
    activate_repel(stream, context);
    context.wait_for_all_requests();

    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 3);
    pbf_move_left_joystick(context, {-1, 0}, 2400ms, 100ms); // 12+
    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 4);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("fly_and_enter_mount_coronet: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("fly_and_enter_mount_coronet: entered Mt. Coronet", COLOR_GREEN);
    };
    return true;
}

static bool go_through_mount_coronet(VideoStream& stream, ProControllerContext& context){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_through_mount_coronet: going through Mt. Coronet...");

    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 4);
    pbf_move_left_joystick(context, {+1, 0}, 400ms, 100ms); // 1+
    pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms); // 1+

    use_HM(stream, context, "Strength");

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, +1}, 10000ms, 500ms); // 10++ (strength)
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);
    pbf_move_left_joystick(context, {0, +1}, 2400ms, 100ms); // 12+

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_mount_coronet: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_mount_coronet: finished first part of Mt. Coronet", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);
    stream.log("go_through_mount_coronet: continuing through Mt. Coronet...");

    pbf_move_left_joystick(context, {-1, 0}, 1000ms, 100ms); // 4+
    pbf_move_left_joystick(context, {0, +1}, 600ms, 100ms);  // 2+

    {
        ShortDialogWatcher repel_dialog(COLOR_RED);
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 11000ms, 100ms); // 55+
            },
            {{repel_dialog}});
        if (ret == 0){
            stream.log("go_through_mount_coronet: Reactivating repel.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 3000ms);
        }else if (ret < 0){
            stream.log("go_through_mount_coronet: Didn't detect repel dialog box!", COLOR_RED);
            return false;
        }
    }
    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, +1}, 5600ms, 100ms); // 28+

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_mount_coronet: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_mount_coronet: finished second part of Mt. Coronet", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {+1, 0}, 1200ms, 100ms); // 6+
    pbf_move_left_joystick(context, {0, +1}, 1600ms, 100ms); // 8+

    {
        WhiteScreenOverWatcher white_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 10000ms, 100ms);
            },
            {{white_screen}});
        if (ret < 0){
            stream.log("go_through_mount_coronet: white screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_mount_coronet: exited Mt. Coronet", COLOR_GREEN);
    };

    return true;
}

static bool go_through_route_216(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_through_route_216: going through the snow...");

    pbf_move_left_joystick(context, {-1, 0}, 1200ms, 100ms); // 5+
    pbf_move_left_joystick(context, {0, +1}, 800ms, 100ms);  // 2+

    {
        ShortDialogWatcher repel_dialog(COLOR_RED);
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 9600ms, 100ms); // 48+
            },
            {{repel_dialog}});
        if (ret == 0){
            stream.log("go_through_route_216: Reactivating repel.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 3000ms);
        }else if (ret < 0){
            stream.log("go_through_route_216: Didn't detect repel dialog box!", COLOR_RED);
            return false;
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    {
        MarkDetector ace_maria_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 3200ms, 100ms); // 16
            },
            {{ace_maria_fight}});
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
                stream.log("go_through_route_216_ace_maria: black screen not detected!", COLOR_RED);
                return false;
            }else if (ret == 0){
                handle_battle(stream, context, stats, "route_216_ace_maria");
                context.wait_for_all_requests();
                pbf_wait(context, 3000ms);
                move_back_after_unexpected_battle(stream, context, "route_216_ace_maria");
            }
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    {
        MarkDetector skier_edward_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 7500ms, 100ms); // 25++ (snow)
            },
            {{skier_edward_fight}});
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
                stream.log("go_through_route_216_skier_edward: black screen not detected!", COLOR_RED);
                return false;
            }else if (ret == 0){
                handle_battle(stream, context, stats, "route_216_skier_edward");
                context.wait_for_all_requests();
                pbf_wait(context, 3000ms);
                move_back_after_unexpected_battle(stream, context, "route_216_skier_edward");
            }
        }
    }

    context.wait_for_all_requests();
    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 800ms, 11);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_route_216: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_route_216: entered chalet", COLOR_GREEN);
    };

    return true;
}

static bool heal_small_house_route_216(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("heal_small_house_route_216: taking a quick nap...");

    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 2);

    {
        ShortDialogWatcher sleep_in_a_bed(COLOR_RED);
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {+1, 0}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{sleep_in_a_bed}});
        if (ret == 0){
            stream.log("heal_small_house_route_216_sleep_in_a_bed: taking a nap...", COLOR_GREEN);

            const ImageFloatBox box{0.670000, 0.600000, 0.100000, 0.150000};
            SelectionArrowFinder arrow(stream.overlay(), box, COLOR_GREEN);

            int arrow_ret = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 10000ms);
                },
                {{arrow}});
            if (arrow_ret != 0){
                stream.log("heal_small_house_route_216_sleep_in_a_bed: selection arrow not found.", COLOR_RED);
                return false;
            }

            pbf_press_button(context, BUTTON_A, 80ms, 300ms);
            pbf_mash_button(context, BUTTON_B, 6000ms);

        }else if (ret < 0){
            stream.log("heal_small_house_route_216_sleep_in_a_bed: Didn't find the bed!", COLOR_RED);
            return false;
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    pbf_move_left_joystick(context, {0, -1}, 600ms, 100ms); // 2+
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 3);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, -1}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("heal_small_house_route_216: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("heal_small_house_route_216: exited chalet", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    stats.move_pp_remaining = stats.move_pp_capacity;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    return true;
}

static bool continue_through_route_216(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("continue_through_route_216: continuing through the snow...");

    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 800ms, 5);
    pbf_move_left_joystick(context, {0, +1}, 2200ms, 100ms); // 11+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 800ms, 6); // Snow

    {
        ShortDialogWatcher repel_dialog(COLOR_RED);
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 11000ms, 100ms); // 55+
            },
            {{repel_dialog}});
        if (ret == 0){
            stream.log("continue_through_route_216: Not reactivating repel.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_B, 3000ms);
        }else if (ret < 0){
            stream.log("continue_through_route_216: Didn't detect repel dialog box!", COLOR_RED);
            return false;
        }
    }

    pbf_move_left_joystick(context, {0, +1}, 2000ms, 100ms); // 10++ (Snow)
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);   // Snow
    pbf_move_left_joystick(context, {0, +1}, 1200ms, 100ms); // 4++ (Snow)
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 800ms, 1); // Snow

    {
        MarkDetector ace_dalton_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 7500ms, 100ms); // 25++ (snow)
            },
            {{ace_dalton_fight}});
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
                stream.log("continue_through_route_216_ace_dalton: black screen not detected!", COLOR_RED);
                return false;
            }else if (ret == 0){
                handle_battle(stream, context, stats, "route_217_ace_dalton");
            }
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);
    pbf_move_left_joystick(context, {0, +1}, 17000ms, 100ms); // 67++ (snow)
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 800ms, 8); // Snow

    {
        ShortDialogWatcher pick_up_TM100(COLOR_RED);
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {0, -1}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{pick_up_TM100}});
        if (ret == 0){
            stream.log("continue_through_route_216: Picking up TM100.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 10000ms);
        }else if (ret < 0){
            stream.log("continue_through_route_216: Didn't detect TM100 dialog box!", COLOR_RED);
            return false;
        }
    }

    pbf_move_left_joystick(context, {-1, 0}, 2000ms, 100ms); // 7++ (snow)

    {
        MarkDetector skier_lexie_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 6000ms, 100ms); // 20++ (snow)
            },
            {{skier_lexie_fight}});
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
                stream.log("continue_through_route_216_skier_lexie: black screen not detected!", COLOR_RED);
                return false;
            }else if (ret == 0){
                handle_battle(stream, context, stats, "route_217_skier_lexie");
                context.wait_for_all_requests();
                pbf_wait(context, 3000ms);
                move_back_after_unexpected_battle(stream, context, "route_217_skier_lexie");
            }
        }
    }

    context.wait_for_all_requests();

    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 800ms, 2);  // Snow
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 800ms, 3);     // Snow
    pbf_move_left_joystick(context, {-1, 0}, 1000ms, 100ms); // 2++ (snow)
    pbf_move_left_joystick(context, {0, +1}, 2700ms, 100ms); // 9++ (snow)
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 800ms, 2); // Snow
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 800ms, 1);    // Snow

    {
        MarkDetector blackbelt_luke_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 5400ms, 100ms); // 21++ (snow)
            },
            {{blackbelt_luke_fight}});
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
                stream.log("continue_through_route_216_blackbelt_luke: black screen not detected!", COLOR_RED);
                return false;
            }else if (ret == 0){
                handle_battle(stream, context, stats, "route_217_blackbelt_luke");
                context.wait_for_all_requests();
                pbf_wait(context, 3000ms);
                move_back_after_unexpected_battle(stream, context, "route_217_blackbelt_luke");
            }
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    activate_repel(stream, context);
    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    {
        MarkDetector ace_olivia_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
            },
            {{ace_olivia_fight}});
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
                stream.log("continue_through_route_216_ace_olivia: black screen not detected!", COLOR_RED);
                return false;
            }else if (ret == 0){
                handle_battle(stream, context, stats, "route_217_ace_olivia");
            }
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    pbf_move_left_joystick(context, {0, +1}, 1800ms, 100ms); // 9+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 4);
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 11);
    pbf_move_left_joystick(context, {+1, 0}, 7800ms, 100ms); // 26++ (snow)
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 800ms, 2);     // Snow
    pbf_move_left_joystick(context, {+1, 0}, 2400ms, 100ms); // 8++ (snow)
    pbf_move_left_joystick(context, {0, +1}, 2400ms, 100ms); // 8++ (snow)
    pbf_move_left_joystick(context, {+1, 0}, 2700ms, 100ms); // 9++ (snow)

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    if (!fly_to(stream, context, FlyPoint::VeilstoneCity)){
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
            stream.log("continue_through_route_216: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("continue_through_route_216: entered Veilstone Pokemon Center", COLOR_GREEN);
    }

    return true;
}

static bool heal_and_exit_veilstone_pokemon_center(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("heal_and_exit_veilstone_pokemon_center: healing and exiting...");
    if (!heal_pokemon(stream, context, "Veilstone city", &stats)){
        return false;
    }
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    return true;
}

static bool go_to_house_south_veilstone(VideoStream& stream, ProControllerContext& context){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_to_house_south_veilstone: heading South...");

    pbf_move_left_joystick(context, {0, -1}, 600ms, 100ms); // 2+

    {
        ShortDialogWatcher repel_dialog(COLOR_RED);
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 4000ms, 100ms); // 20+
                pbf_move_left_joystick(context, {+1, 0}, 4000ms, 100ms); // 20+
            },
            {{repel_dialog}});
        if (ret == 0){
            stream.log("go_to_house_south_veilstone: Not reactivating repel.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_B, 3000ms);
        }else if (ret < 0){
            stream.log("go_to_house_south_veilstone: Didn't detect repel dialog box!", COLOR_RED);
            return false;
        }
    }

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

static bool go_through_house_south_veilstone(VideoStream& stream, ProControllerContext& context){
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

static bool walk_through_route_214(VideoStream& stream, ProControllerContext& context){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("walk_through_route_214: going through route 214...");

    pbf_move_left_joystick(context, {+1, 0}, 600ms, 100ms);  // 2+
    pbf_move_left_joystick(context, {0, -1}, 3800ms, 100ms); // 19+
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 8);

    activate_repel(stream, context);

    pbf_move_left_joystick(context, {0, -1}, 2600ms, 100ms); // 13+

    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1, true);
    pbf_move_left_joystick(context, {0, -1}, 1200ms, 100ms); // 6+
    pbf_move_left_joystick(context, {-1, 0}, 400ms, 100ms);  // 2
    pbf_move_left_joystick(context, {0, -1}, 1000ms, 100ms); // 5+

    pbf_move_left_joystick(context, {+1, 0}, 1600ms, 100ms); // 8+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1, true);
    pbf_move_left_joystick(context, {0, -1}, 1600ms, 100ms); // 8+

    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1, true);
    pbf_move_left_joystick(context, {0, -1}, 2200ms, 100ms); // 11+

    pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms); // 4+
    pbf_move_left_joystick(context, {0, -1}, 600ms, 100ms); // 3+
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 2, true);

    pbf_move_left_joystick(context, {0, -1}, 6200ms, 100ms); // 31+
    context.wait_for_all_requests();

    {
        ShortDialogWatcher repel_dialog(COLOR_RED);
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 11000ms, 100ms); // 55+
            },
            {{repel_dialog}});
        if (ret == 0){
            stream.log("walk_through_route_214: Reactivating repel.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 3000ms);
        }else if (ret < 0){
            stream.log("walk_through_route_214: Didn't detect repel dialog box!", COLOR_RED);
            return false;
        }
    }

    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {-1, 0}, 2000ms, 100ms); // 10+
    pbf_move_left_joystick(context, {0, -1}, 2800ms, 100ms); // 14+
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 2);
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 3);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("walk_through_route_214: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("walk_through_route_214: entered Valor Lakefront", COLOR_GREEN);
    }

    return true;
}

static bool go_through_lake_valor(VideoStream& stream, ProControllerContext& context){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_through_lake_valor: going through the remnants of the lake...");

    pbf_move_left_joystick(context, {-1, 0}, 2000ms, 100ms); // 10+
    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 2);
    pbf_move_left_joystick(context, {-1, 0}, 1200ms, 100ms); // 6+
    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 1);
    pbf_move_left_joystick(context, {0, -1}, 800ms, 100ms); // 3+
    pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms); // 3+
    pbf_move_left_joystick(context, {0, -1}, 600ms, 100ms); // 2+
    pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms); // 2+
    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 3);


    {
        ShortDialogWatcher repel_dialog(COLOR_RED);
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 2400ms, 100ms); // 12+
                pbf_move_left_joystick(context, {+1, 0}, 2400ms, 100ms); // 12+
                pbf_move_left_joystick(context, {-1, 0}, 2400ms, 100ms); // 12+
                pbf_move_left_joystick(context, {+1, 0}, 2400ms, 100ms); // 12+
                pbf_move_left_joystick(context, {-1, 0}, 2400ms, 100ms); // 12+
                pbf_move_left_joystick(context, {+1, 0}, 2400ms, 100ms); // 12+
            },
            {{repel_dialog}});
        if (ret == 0){
            stream.log("go_through_lake_valor: Not reactivating repel.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_B, 3000ms);
        }else if (ret < 0){
            stream.log("go_through_lake_valor: Didn't detect repel dialog box!", COLOR_RED);
            return false;
        }
    }
    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    pbf_move_left_joystick(context, {-1, 0}, 2400ms, 100ms); // 12+

    // TODO(07.04): Record the cave approach, Galactic battles, Saturn battle,
    // reward/dialogue cleanup, and return to the outdoor cave exit.
    // return_to_twinleaf_and_heal() preserves the continuation after this gap.
    context.wait_for_all_requests();
    throw FatalProgramException(ErrorReportMode::NO_ERROR_REPORT,
        "07.04 incomplete: Lake Valor cave approach, Saturn battle, and cave exit need recorded pathing. "
        "Stopped before the missing route; later checkpoints have not run.",
        stream);
}

static bool return_to_twinleaf_and_heal(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats){
    if (!fly_to(stream, context, FlyPoint::TwinleafTown)){
        return false;
    }

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_through_lake_valor: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_lake_valor: back into own's house", COLOR_GREEN);
    }


    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("heal_at_home: sleeping at mom's...");

    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 1);

    {
        ShortDialogWatcher mom_heal_dialog(COLOR_RED);
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {0, +1}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{mom_heal_dialog}});
        if (ret == 0){
            stream.log("heal_at_home: Sleeping...", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_B, 4000ms);
        }else if (ret < 0){
            stream.log("heal_at_home: Didn't detect mom dialog box!", COLOR_RED);
            return false;
        }
    }
    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);
    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {0, -1}, 400ms, 100ms); // 1+
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, -1}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("heal_at_home: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("heal_at_home: leaving own's house", COLOR_GREEN);
    }

    context.wait_for_all_requests();
    stats.move_pp_remaining = stats.move_pp_capacity;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    return true;
}

static bool go_to_lake_verity(VideoStream& stream, ProControllerContext& context){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_to_lake_verity: heading towards Lake Verity...");

    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 5);

    pbf_move_left_joystick(context, {0, +1}, 6800ms, 100ms); // 34+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 1);
    pbf_move_left_joystick(context, {-1, 0}, 6200ms, 100ms); // 31+

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("go_to_lake_verity: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_to_lake_verity: entered Verity lakefront", COLOR_GREEN);
    }
    return true;
}

static bool commander_mars_at_verity(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);
    context.wait_for_all_requests();
    stream.log("commander_mars_at_verity: Completing the Lake Verity battles and Mars dialogue...");

    /*{
        ShortDialogWatcher rowan_talking(COLOR_BLUE);
        int ret = wait_until(stream, context, std::chrono::seconds(30), {{talk_to_clint}});
        if (ret < 0){
            stream.log("commander_mars_at_verity_rowan_talking: dialog box not detected!", COLOR_RED);
            return false;
        } else if (ret == 0) {
            stream.log("commander_mars_at_verity_rowan_talking: dialog box detected.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_B, 4000ms);
            context.wait_for_all_requests();
        }
    }*/

    pbf_move_left_joystick(context, {0, +1}, 600ms, 100ms); // 2+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 2);

    {
        MarkDetector grunt_1_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 3200ms, 100ms); // 16
            },
            {{grunt_1_fight}});
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
                stream.log("commander_mars_at_verity_grunt_1: black screen not detected!", COLOR_RED);
                return false;
            }else if (ret == 0){
                handle_battle(stream, context, stats, "verity_lakefront_grunt_1");
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
                pbf_mash_button(context, BUTTON_A, 200000ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("commander_mars_at_verity_grunt_4: black screen not detected!", COLOR_RED);
            return false;
        }else if (ret == 0){
            handle_battle(stream, context, stats, "verity_lakefront_grunt_2");
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 2);

    pbf_move_left_joystick(context, {+1, 0}, 1400ms, 100ms); // 7+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 2);
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 1);

    {
        MarkDetector grunt_3_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 3200ms, 100ms); // 16
            },
            {{grunt_3_fight}});
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
                stream.log("commander_mars_at_verity_grunt_3: black screen not detected!", COLOR_RED);
                return false;
            }else if (ret == 0){
                handle_battle(stream, context, stats, "verity_lakefront_grunt_3");
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
                pbf_mash_button(context, BUTTON_A, 200000ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("commander_mars_at_verity_grunt_4: black screen not detected!", COLOR_RED);
            return false;
        }else if (ret == 0){
            handle_battle(stream, context, stats, "verity_lakefront_grunt_4");
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 1);
    pbf_move_left_joystick(context, {+1, 0}, 800ms, 100ms); // 3+

    {
        ShortDialogWatcher commander_mars_fight(COLOR_RED);
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {0, +1}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{commander_mars_fight}});
        if (ret < 0){
            stream.log("commander_mars_at_verity: Didn't detect Commander Mars dialog box!", COLOR_RED);
            return false;
        }else if (ret == 0){
            stream.log("commander_mars_at_verity: Battling Commander Mars...", COLOR_GREEN);
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
                stream.log("commander_mars_at_verity_commander_mars: black screen not detected!", COLOR_RED);
                return false;
            }else if (ret == 0){
                handle_battle(stream, context, stats, "verity_lakefront_commander_mars");
            }
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    pbf_mash_button(context, BUTTON_B, 10000ms);
    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    clear_post_battle_dialogue(stream, context, BUTTON_B, 6000ms);
    context.wait_for_all_requests();
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    return true;
}

static bool fly_to_snowpoint_and_heal(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats){
    if (!fly_to(stream, context, FlyPoint::SnowpointCity)){
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
            stream.log("fly_to_snowpoint_and_heal: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("fly_to_snowpoint_and_heal: entered Snowpoint City Pokemon Center", COLOR_GREEN);
    }


    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);
    context.wait_for_all_requests();
    stream.log("heal_at_snowpoint_pokemon_center: Healing at Snowpoint City Pokemon center and exiting...");

    if (!heal_pokemon(stream, context, "Snowpoint City", &stats)){
        return false;
    }
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    return true;
}

static bool go_to_snowpoint_gym(VideoStream& stream, ProControllerContext& context){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);
    context.wait_for_all_requests();
    stream.log("go_to_snowpoint_gym: Heading towards Snowpoint City gym...");

    pbf_move_left_joystick(context, {-1, 0}, 1000ms, 100ms); // 5+
    pbf_move_left_joystick(context, {0, +1}, 1600ms, 100ms); // 8+

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
            stream.log("go_to_snowpoint_gym: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_to_snowpoint_gym: entered Snowpoint Gym", COLOR_GREEN);
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    // TODO(07.07): Record the puzzle, trainers, Candice battle,
    // rewards, and return route to the inside entrance for Segment 08.
    throw FatalProgramException(ErrorReportMode::NO_ERROR_REPORT,
        "07.07 incomplete: Snowpoint Gym entrance reached. Puzzle, Candice battle, "
        "rewards, and return path need recorded pathing. Segment 08 must not start.",
        stream);
}

// ---------------------------------------------------------------------------
//  Segment wrappers
// ---------------------------------------------------------------------------

std::string AutoStory_Segment_07::name() const {
    return "07: Canalave to Snowpoint";
}

std::string AutoStory_Segment_07::start_text() const {
    return "Start: Inside Canalave Gym at the entrance, after Byron, with sixth-badge rewards and dialogue finished.";
}

std::string AutoStory_Segment_07::end_text() const {
    return "Target: Seventh badge and inside Snowpoint Gym at the entrance, with rewards finished. Lake Valor and Snowpoint Gym pathing are pending recording.";
}

void AutoStory_Segment_07::run_segment(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_0701().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0702().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0703().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0704().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0705().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0706().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0707().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}

// ---------------------------------------------------------------------------
//  Checkpoint wrappers
// ---------------------------------------------------------------------------

std::string AutoStory_Checkpoint_0701::name() const {
    return "07.01 - Canalave library events";
}

std::string AutoStory_Checkpoint_0701::start_text() const {
    return "Inside Canalave Gym at the entrance, after Byron, with sixth-badge rewards and dialogue finished.";
}

std::string AutoStory_Checkpoint_0701::end_text() const {
    return "Outside Canalave library after the meeting and Rowan follow-up, with dialogue finished.";
}

void AutoStory_Checkpoint_0701::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0701(env, context, options, stats);
}

void checkpoint_0701(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            if (!leave_gym(env.console, context)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "07.01: leave_gym did not complete.",
                    env.console);
            }
            if (!go_to_the_library(env.console, context)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "07.01: go_to_the_library did not complete.",
                    env.console);
            }
            if (!drama_at_the_library(env.console, context)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "07.01: drama_at_the_library did not complete.",
                    env.console);
            }
        },
        true);
}

std::string AutoStory_Checkpoint_0702::name() const {
    return "07.02 - Canalave to Route 216";
}

std::string AutoStory_Checkpoint_0702::start_text() const {
    return "Outside Canalave library after the meeting and Rowan follow-up, with dialogue finished.";
}

std::string AutoStory_Checkpoint_0702::end_text() const {
    return "Healed and outside the Route 216 rest house at the doorway.";
}

void AutoStory_Checkpoint_0702::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0702(env, context, options, stats);
}

void checkpoint_0702(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            if (!fly_and_enter_mount_coronet(env.console, context)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "07.02: fly_and_enter_mount_coronet did not complete.",
                    env.console);
            }
            if (!go_through_mount_coronet(env.console, context)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "07.02: go_through_mount_coronet did not complete.",
                    env.console);
            }
            if (!go_through_route_216(env.console, context, stats)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "07.02: go_through_route_216 did not complete.",
                    env.console);
            }
            if (!heal_small_house_route_216(env.console, context, stats)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "07.02: heal_small_house_route_216 did not complete.",
                    env.console);
            }
        },
        true);
}

std::string AutoStory_Checkpoint_0703::name() const {
    return "07.03 - Route 217 and Snowpoint";
}

std::string AutoStory_Checkpoint_0703::start_text() const {
    return "Healed and outside the Route 216 rest house at the doorway.";
}

std::string AutoStory_Checkpoint_0703::end_text() const {
    return "Healed and outside Veilstone Pokemon Center after the first Snowpoint visit and Rock Climb pickup.";
}

void AutoStory_Checkpoint_0703::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0703(env, context, options, stats);
}

void checkpoint_0703(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            if (!continue_through_route_216(env.console, context, stats)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "07.03: continue_through_route_216 did not complete.",
                    env.console);
            }
            if (!heal_and_exit_veilstone_pokemon_center(env.console, context, stats)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "07.03: heal_and_exit_veilstone_pokemon_center did not complete.",
                    env.console);
            }
        },
        true);
}

std::string AutoStory_Checkpoint_0704::name() const {
    return "07.04 - Lake Valor (path pending)";
}

std::string AutoStory_Checkpoint_0704::start_text() const {
    return "Healed and outside Veilstone Pokemon Center after the first Snowpoint visit and Rock Climb pickup.";
}

std::string AutoStory_Checkpoint_0704::end_text() const {
    return "Target: Saturn defeated; healed and outside the Twinleaf home. Currently stops at the unrecorded Lake Valor cave approach.";
}

void AutoStory_Checkpoint_0704::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0704(env, context, options, stats);
}

void checkpoint_0704(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            if (!go_to_house_south_veilstone(env.console, context)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "07.04: go_to_house_south_veilstone did not complete.",
                    env.console);
            }
            if (!go_through_house_south_veilstone(env.console, context)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "07.04: go_through_house_south_veilstone did not complete.",
                    env.console);
            }
            if (!walk_through_route_214(env.console, context)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "07.04: walk_through_route_214 did not complete.",
                    env.console);
            }
            if (!go_through_lake_valor(env.console, context)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "07.04: go_through_lake_valor did not complete.",
                    env.console);
            }
            if (!return_to_twinleaf_and_heal(env.console, context, stats)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "07.04: return_to_twinleaf_and_heal did not complete.",
                    env.console);
            }
        },
        true);
}

std::string AutoStory_Checkpoint_0705::name() const {
    return "07.05 - Lake Verity and Mars";
}

std::string AutoStory_Checkpoint_0705::start_text() const {
    return "Outside the Twinleaf home, healed, after completing Lake Valor and Saturn.";
}

std::string AutoStory_Checkpoint_0705::end_text() const {
    return "At Lake Verity after Mars, with follow-up dialogue finished and outdoor control restored.";
}

void AutoStory_Checkpoint_0705::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0705(env, context, options, stats);
}

void checkpoint_0705(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            if (!go_to_lake_verity(env.console, context)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "07.05: go_to_lake_verity did not complete.",
                    env.console);
            }
            if (!commander_mars_at_verity(env.console, context, stats)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "07.05: commander_mars_at_verity did not complete.",
                    env.console);
            }
        },
        true);
}

std::string AutoStory_Checkpoint_0706::name() const {
    return "07.06 - Return to Snowpoint";
}

std::string AutoStory_Checkpoint_0706::start_text() const {
    return "At Lake Verity after Mars, with follow-up dialogue finished and outdoor control restored.";
}

std::string AutoStory_Checkpoint_0706::end_text() const {
    return "Healed and outside Snowpoint Pokemon Center at the doorway.";
}

void AutoStory_Checkpoint_0706::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0706(env, context, options, stats);
}

void checkpoint_0706(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            if (!fly_to_snowpoint_and_heal(env.console, context, stats)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "07.06: fly_to_snowpoint_and_heal did not complete.",
                    env.console);
            }
        },
        true);
}

std::string AutoStory_Checkpoint_0707::name() const {
    return "07.07 - Snowpoint Gym (path pending)";
}

std::string AutoStory_Checkpoint_0707::start_text() const {
    return "Healed and outside Snowpoint Pokemon Center at the doorway.";
}

std::string AutoStory_Checkpoint_0707::end_text() const {
    return "Target: Seventh badge, rewards finished, inside Snowpoint Gym at the entrance. Currently stops at the unrecorded Gym puzzle.";
}

void AutoStory_Checkpoint_0707::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0707(env, context, options, stats);
}

void checkpoint_0707(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            if (!go_to_snowpoint_gym(env.console, context)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "07.07: go_to_snowpoint_gym did not complete.",
                    env.console);
            }
        },
        true);
}

}
}
}
