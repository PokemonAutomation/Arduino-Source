/*  BDSP AutoStory - Segment 07
 *  From after acquiring the sixth badge until after acquiring the seventh badge.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string>
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
#include "PokemonBDSP/Inference/PokemonBDSP_TransitionWatchers.h"
#include "PokemonBDSP/Programs/AutoStory/Detect/PokemonBDSP_AutoStory_OverworldDetector.h"
#include "PokemonBDSP/Programs/AutoStory/PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP/Programs/AutoStory/Utils/PokemonBDSP_AutoStory_Shopping.h"
#include "PokemonBDSP/Programs/AutoStory/Utils/PokemonBDSP_AutoStory_Battle.h"
#include "PokemonBDSP/Programs/AutoStory/Utils/PokemonBDSP_AutoStory_Segment_00_tools.h"

#include "PokemonBDSP_AutoStory_Segment_02.h"
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

    clear_dialog(stream, context, BUTTON_B, 6000ms);
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
                fight_trainer_using_moves(stream, context, stats, "route_216_ace_maria", true);
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
                fight_trainer_using_moves(stream, context, stats, "route_216_skier_edward", true);
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
    context.wait_for_all_requests();
    stats.move_pp_remaining = stats.move_pp_capacity;
    stream.log("heal_small_house_route_216: healed; staying inside for the next checkpoint save.", COLOR_GREEN);
    return true;
}

static bool leave_small_house_route_216(VideoStream& stream, ProControllerContext& context){
    DpadState dpad;
    context.wait_for_all_requests();
    stream.log("leave_small_house_route_216: leaving after the indoor checkpoint save.");

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
            stream.log("leave_small_house_route_216: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("leave_small_house_route_216: exited chalet", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();

    // Stay on the clear strip in front of the house, away from the grass.
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);
    context.wait_for_all_requests();
    force_repel_expiry(stream, context, RepelRenewalChoice::YES, false, true);

    // Fourteen westward presses reached the tree stop in the successful run.
    // Expiry can end facing either direction; the obstruction absorbs the offset.
    dpad.last_dir = DPAD_NONE;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 800ms, 14);
    context.wait_for_all_requests();
    stream.log("leave_small_house_route_216: Repel renewed; westward alignment hold finished.", COLOR_GREEN);
    return true;
}

static bool continue_through_route_216(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("continue_through_route_216: continuing through the snow...");

    // Offset six walking steps east from the post-renewal westward stop.
    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 800ms, 6);
    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, +1}, 2200ms, 100ms); // 11+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 800ms, 6); // Snow
    context.wait_for_all_requests();

    // Repel is fresh from the rest-house exit; do not wait for expiration here.
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
                fight_trainer_using_moves(stream, context, stats, "route_217_ace_dalton", true);
            }
        }else{
            stream.log("continue_through_route_216: Dalton approach was not detected; cannot use him as the alignment stop.", COLOR_RED);
            return false;
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);
    context.wait_for_all_requests();
    force_repel_expiry(stream, context, RepelRenewalChoice::YES, true, true);

    // The north/south wiggle can stop partway through a move. Dalton stands
    // one tile east after battle: approach from above and use him as the stop.
    // The extra northward clearance is absorbed by the southward collision hold.
    dpad.last_dir = DPAD_NONE;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 800ms, 4);
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 800ms, 1);
    pbf_move_left_joystick(context, {0, -1}, 8000ms, 100ms);
    context.wait_for_all_requests();
    dpad.last_dir = DPAD_DOWN;
    // From Dalton's north side, restore the original tile immediately west of him.
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 800ms, 1);
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 800ms, 1);
    context.wait_for_all_requests();
    stream.log("continue_through_route_216: Repel renewed; Dalton alignment stop and return offset finished.");

    // The return offset restores the original start of this recorded northward leg.
    pbf_move_left_joystick(context, {0, +1}, 17000ms, 100ms); // 67++ (snow)
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 800ms, 8); // Snow
    context.wait_for_all_requests();

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

    context.wait_for_all_requests();
    force_repel_expiry(stream, context, RepelRenewalChoice::NO, false, true);
    // Absorb the east/west expiry offset with the extended westward approach.
    pbf_move_left_joystick(context, {-1, 0}, 2500ms, 100ms);

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
                fight_trainer_using_moves(stream, context, stats, "route_217_skier_lexie", true);
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
                fight_trainer_using_moves(stream, context, stats, "route_217_blackbelt_luke", true);
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
                fight_trainer_using_moves(stream, context, stats, "route_217_ace_olivia", true);
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

    if (!fly_to(stream, context, FlyPoint::SnowpointCity)){
        return false;
    }
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();

    return true;
}

// Start: Snowpoint Fly landing point. End: outside the Mart after restocking.
static bool restock_super_repels_snowpoint(VideoStream& stream,
    ProControllerContext& context,
    int quantity,
    Language language){
    stream.log("Snowpoint Mart: following the recorded route to buy " + std::to_string(quantity) + " Super Repels.");
    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 944ms, 0ms);
    pbf_wait(context, 54ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 1606ms, 0ms);
    pbf_wait(context, 131ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 2391ms, 0ms);
    pbf_wait(context, 3328ms);
    pbf_press_button(context, BUTTON_B, 229ms, 0ms);
    pbf_wait(context, 2255ms);
    pbf_press_button(context, BUTTON_B, 195ms, 0ms);
    pbf_wait(context, 519ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1726ms, 0ms);
    pbf_wait(context, 129ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 1126ms, 0ms);
    pbf_wait(context, 82ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1264ms, 0ms);
    pbf_wait(context, 236ms);
    context.wait_for_all_requests();

    {
        BlackScreenOverWatcher entrance(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        const int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0.000000, 1.000000}, 768ms, 0ms);
                pbf_move_left_joystick(context, {1.000000, 1.000000}, 1460ms, 0ms);
                pbf_wait(context, 3577ms);
            },
            {{entrance}});
        if (ret != 0){
            stream.log("Snowpoint Mart: entrance transition did not complete.", COLOR_RED);
            return false;
        }
    }

    pbf_move_left_joystick(context, {0.000000, 1.000000}, 845ms, 0ms);
    pbf_wait(context, 503ms);
    context.wait_for_all_requests();
    if (!buy_items(stream, context, {{ShopItem::SuperRepel, quantity}}, language, DPAD_UP)){
        stream.log("Snowpoint Mart: Could not buy " + std::to_string(quantity) + " Super Repels.", COLOR_RED);
        return false;
    }
    context.wait_for_all_requests();

    {
        BlackScreenOverWatcher exit(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        const int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0.000000, -1.000000}, 1101ms, 0ms);
                // Observe the doorway fade after the recorded movement ends.
                pbf_wait(context, 5000ms);
            },
            {{exit}});
        if (ret != 0){
            stream.log("Snowpoint Mart: exit transition did not complete; cannot continue.", COLOR_RED);
            return false;
        }
    }
    context.wait_for_all_requests();
    stream.log("Snowpoint Mart: bought " + std::to_string(quantity) + " Super Repels and exited.", COLOR_GREEN);
    return true;
}

// Start: outside Snowpoint Mart after shopping. End: healed outside Veilstone Center.
static bool heal_and_exit_veilstone_pokemon_center(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats){
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
        stream.log("heal_and_exit_veilstone_pokemon_center: entered Veilstone Pokemon Center", COLOR_GREEN);
    }

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
    // Super Repels were first purchased in Snowpoint. Clear their NEW flags
    // before the first bag lookup in this checkpoint.
    if (!clear_new_item_flags(stream, context, 8)){
        stream.log("07.04: Could not clear new-item flags before using Super Repels.", COLOR_RED);
        return false;
    }
    stream.log("go_to_house_south_veilstone: heading South...");

    pbf_move_left_joystick(context, {0, -1}, 600ms, 100ms); // 2+

    // Repel already expired in Snowpoint before shopping. Go directly to the
    // westward alignment instead of requiring another expiry in Veilstone.
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

    if (!activate_repel(stream, context, 8, 1)){
        stream.log("walk_through_route_214: Could not activate a Super Repel.", COLOR_RED);
        return false;
    }

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

    // Keep the recorded southward stop against the trees at 41:00.
    // Skip the expiry wiggle and shorten the following westward alignment
    // from four seconds to three before continuing along the existing route.
    pbf_move_left_joystick(context, {0, -1}, 6200ms, 100ms); // 31+
    pbf_move_left_joystick(context, {-1, 0}, 3000ms, 100ms);

    // Carry the original Super Repel into the lake. Do not deliberately renew
    // it here just to expire the replacement again at the lake stair landing.
    pbf_move_left_joystick(context, {0, -1}, 2800ms, 100ms); // 14+
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 2);
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 3);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                // Recorded entry completes in under three seconds, including the fade.
                pbf_move_left_joystick(context, {-1, 0}, 4000ms, 100ms);
            },
            {{black_screen}});
        if (ret < 0){
            stream.log("walk_through_route_214: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("walk_through_route_214: entered Lake Valor", COLOR_GREEN);
    }

    return true;
}

static bool go_through_lake_valor(VideoStream& stream,
    ProControllerContext& context,
    AutoStoryStats& stats){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_through_lake_valor: going through the remnants of the lake...");

    // Clear the map boundary before forcing expiry. Wiggling at the spawn
    // repeatedly crossed the Lake Valor / Valor Lakefront transition.
    pbf_move_left_joystick(context, {-1, 0}, 2000ms, 100ms); // 10+
    context.wait_for_all_requests();
    force_repel_expiry(stream, context, RepelRenewalChoice::NO, false, true);
    // The footage shows this west hold reaching the cliff beside the stairs.
    // Reuse that stop after expiry to remove the horizontal wiggle offset.
    pbf_move_left_joystick(context, {-1, 0}, 2000ms, 100ms);
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


    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    pbf_move_left_joystick(context, {-1, 0}, 2400ms, 100ms); // 12+

    context.wait_for_all_requests();
    stream.log("Lake Valor: following the recorded route from the stair landing to Saturn.");
    // Recorded taps include facing presses. Do not add another turn press.
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 1627ms, 0ms);
    pbf_wait(context, 390ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 434ms, 0ms);
    pbf_wait(context, 165ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 570ms, 0ms);
    pbf_wait(context, 919ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 279ms, 0ms);
    pbf_wait(context, 322ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1024ms, 0ms);
    pbf_wait(context, 1871ms);
    pbf_press_dpad(context, DPAD_RIGHT, 160ms, 0ms);
    pbf_wait(context, 145ms);
    pbf_press_dpad(context, DPAD_RIGHT, 79ms, 0ms);
    pbf_wait(context, 454ms);
    pbf_press_dpad(context, DPAD_RIGHT, 106ms, 0ms);
    pbf_wait(context, 441ms);
    pbf_press_dpad(context, DPAD_DOWN, 199ms, 0ms);
    pbf_wait(context, 190ms);
    pbf_press_dpad(context, DPAD_DOWN, 155ms, 0ms);
    pbf_wait(context, 256ms);
    pbf_press_dpad(context, DPAD_DOWN, 199ms, 0ms);
    pbf_wait(context, 682ms);
    pbf_press_dpad(context, DPAD_DOWN, 144ms, 0ms);
    pbf_wait(context, 564ms);
    pbf_press_dpad(context, DPAD_LEFT, 187ms, 0ms);
    pbf_wait(context, 639ms);
    pbf_press_dpad(context, DPAD_DOWN, 184ms, 0ms);
    pbf_wait(context, 267ms);
    pbf_press_dpad(context, DPAD_DOWN, 199ms, 0ms);
    pbf_wait(context, 483ms);
    pbf_press_dpad(context, DPAD_DOWN, 184ms, 0ms);
    pbf_wait(context, 388ms);
    pbf_press_dpad(context, DPAD_RIGHT, 145ms, 0ms);
    pbf_wait(context, 336ms);
    pbf_press_dpad(context, DPAD_RIGHT, 176ms, 0ms);
    pbf_wait(context, 305ms);
    pbf_press_dpad(context, DPAD_RIGHT, 143ms, 0ms);
    pbf_wait(context, 388ms);
    pbf_press_dpad(context, DPAD_RIGHT, 131ms, 0ms);
    pbf_wait(context, 376ms);
    pbf_press_dpad(context, DPAD_DOWN, 212ms, 0ms);
    pbf_wait(context, 1321ms);
    pbf_press_dpad(context, DPAD_RIGHT, 176ms, 0ms);
    pbf_wait(context, 1104ms);
    context.wait_for_all_requests();
    {
        BlackScreenWatcher battle_start;
        const int result = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& ctx){
                // The recorded fight relied on the Grunt turning and walking
                // over. Approach and interact instead of waiting at a distance.
                ssf_press_left_joystick(ctx, {-1, 0}, 0ms, 2000ms);
                ssf_mash1_button(ctx, BUTTON_A, 30000ms);
            },
            {{battle_start}});
        if (result != 0){
            stream.log("Lake Valor Grunt: Battle did not start.", COLOR_RED);
            return false;
        }
    }
    fight_trainer_using_moves(stream, context, stats, "lake_valor_grunt", true);
    context.wait_for_all_requests();

    // A direct interaction and a sight-line encounter can end at different
    // horizontal positions. Step below the Grunt before using the west cliff
    // so the NPC cannot block the wall stop. Restore the recorded column five
    // walking steps east; one south step here leaves seven of the recorded eight.
    // TODO(live verification): verify this wall return and restored column.
    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 1);
    pbf_move_left_joystick(context, {-1, 0}, 2000ms, 100ms);
    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 5);
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 7);
    pbf_press_dpad(context, DPAD_RIGHT, 211ms, 0ms);
    pbf_wait(context, 404ms);
    pbf_press_dpad(context, DPAD_RIGHT, 196ms, 0ms);
    pbf_wait(context, 429ms);
    pbf_press_dpad(context, DPAD_RIGHT, 187ms, 0ms);
    pbf_wait(context, 347ms);
    pbf_press_dpad(context, DPAD_RIGHT, 168ms, 0ms);
    pbf_wait(context, 341ms);
    pbf_press_dpad(context, DPAD_RIGHT, 168ms, 0ms);
    pbf_wait(context, 324ms);
    pbf_press_dpad(context, DPAD_RIGHT, 143ms, 0ms);
    pbf_wait(context, 632ms);
    pbf_press_dpad(context, DPAD_RIGHT, 179ms, 0ms);
    pbf_wait(context, 390ms);
    pbf_press_dpad(context, DPAD_RIGHT, 198ms, 0ms);
    pbf_wait(context, 300ms);
    pbf_press_dpad(context, DPAD_RIGHT, 194ms, 0ms);
    pbf_wait(context, 441ms);
    pbf_press_dpad(context, DPAD_RIGHT, 175ms, 0ms);
    pbf_wait(context, 425ms);
    pbf_press_dpad(context, DPAD_RIGHT, 199ms, 0ms);
    pbf_wait(context, 401ms);
    pbf_press_dpad(context, DPAD_RIGHT, 109ms, 0ms);
    pbf_wait(context, 624ms);
    pbf_press_dpad(context, DPAD_UP, 202ms, 0ms);
    pbf_wait(context, 520ms);
    pbf_press_dpad(context, DPAD_UP, 171ms, 0ms);
    pbf_wait(context, 362ms);
    pbf_press_dpad(context, DPAD_UP, 189ms, 0ms);
    pbf_wait(context, 423ms);
    pbf_press_dpad(context, DPAD_UP, 198ms, 0ms);
    pbf_wait(context, 350ms);
    pbf_press_dpad(context, DPAD_UP, 186ms, 0ms);
    pbf_wait(context, 465ms);
    pbf_press_dpad(context, DPAD_LEFT, 160ms, 0ms);
    pbf_wait(context, 628ms);
    context.wait_for_all_requests();
    {
        BlackScreenWatcher black;
        WhiteScreenWatcher white;
        const int result = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& ctx){
                pbf_press_dpad(ctx, DPAD_UP, 147ms, 0ms);
                pbf_wait(ctx, 414ms);
                pbf_press_dpad(ctx, DPAD_UP, 186ms, 0ms);
                pbf_wait(ctx, 454ms);
                pbf_press_dpad(ctx, DPAD_UP, 187ms, 0ms);
                pbf_wait(ctx, 323ms);
                pbf_press_dpad(ctx, DPAD_UP, 193ms, 0ms);
                pbf_wait(ctx, 1741ms);
                // Observe a delayed fade without extending the recorded movement.
                pbf_wait(ctx, 5000ms);
            },
            {{black}, {white}});
        if (result < 0){
            stream.log("Valor Cavern: Entrance transition did not start.", COLOR_RED);
            return false;
        }
        BlackScreenWatcher black_gone(COLOR_RED,
            {0.1, 0.1, 0.8, 0.8}, 100, 10, BlackScreenWatcher::FinderType::GONE, 500ms);
        WhiteScreenWatcher white_gone(COLOR_RED,
            {0.1, 0.1, 0.8, 0.8}, 500, 10, WhiteScreenWatcher::FinderType::GONE, 500ms);
        const int finished = result == 1 ? wait_until(stream, context, 15s, {{white_gone}})
            : wait_until(stream, context, 15s, {{black_gone}});
        if (finished != 0){
            stream.log("Valor Cavern: Entrance transition did not finish.", COLOR_RED);
            return false;
        }
        OverworldWatcher overworld;
        if (wait_until(stream, context, 10s, {{overworld}}) != 0){
            stream.log("Valor Cavern: Indoor overworld not ready.", COLOR_RED);
            return false;
        }
    }

    pbf_press_dpad(context, DPAD_UP, 180ms, 0ms);
    pbf_wait(context, 443ms);
    pbf_press_dpad(context, DPAD_UP, 156ms, 0ms);
    pbf_wait(context, 321ms);
    pbf_press_dpad(context, DPAD_UP, 159ms, 0ms);
    pbf_wait(context, 376ms);
    pbf_press_dpad(context, DPAD_UP, 158ms, 0ms);
    pbf_wait(context, 273ms);
    pbf_press_dpad(context, DPAD_UP, 167ms, 0ms);
    pbf_wait(context, 366ms);
    pbf_press_dpad(context, DPAD_UP, 169ms, 0ms);
    pbf_wait(context, 298ms);
    pbf_press_dpad(context, DPAD_UP, 180ms, 0ms);
    pbf_wait(context, 321ms);
    pbf_press_dpad(context, DPAD_UP, 176ms, 0ms);
    pbf_wait(context, 412ms);
    pbf_press_dpad(context, DPAD_UP, 172ms, 0ms);
    pbf_wait(context, 1096ms);
    context.wait_for_all_requests();
    {
        BlackScreenWatcher battle_start;
        const int result = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& ctx){
                pbf_mash_button(ctx, BUTTON_A, 30000ms);
            },
            {{battle_start}});
        if (result != 0){
            stream.log("Saturn: Battle did not start.", COLOR_RED);
            return false;
        }
    }
    fight_trainer_using_moves(stream, context, stats, "lake_valor_commander_saturn", true);
    clear_dialog(stream, context, BUTTON_B, 6000ms);
    context.wait_for_all_requests();

    {
        BlackScreenWatcher black;
        WhiteScreenWatcher white;
        const int result = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {0.000000, -1.000000}, 1834ms, 0ms);
                pbf_wait(ctx, 5000ms);
            },
            {{black}, {white}});
        if (result < 0){
            stream.log("Valor Cavern: Exit transition did not start.", COLOR_RED);
            return false;
        }
        // The recording exits through a white fade. Stop movement at its onset
        // so no remaining south hold carries past the exterior doorway spawn.
        BlackScreenWatcher black_gone(COLOR_RED,
            {0.1, 0.1, 0.8, 0.8}, 100, 10, BlackScreenWatcher::FinderType::GONE, 500ms);
        WhiteScreenWatcher white_gone(COLOR_RED,
            {0.1, 0.1, 0.8, 0.8}, 500, 10, WhiteScreenWatcher::FinderType::GONE, 500ms);
        const int finished = result == 1 ? wait_until(stream, context, 15s, {{white_gone}})
            : wait_until(stream, context, 15s, {{black_gone}});
        if (finished != 0){
            stream.log("Valor Cavern: Exit transition did not finish.", COLOR_RED);
            return false;
        }
        OverworldWatcher overworld;
        if (wait_until(stream, context, 10s, {{overworld}}) != 0){
            stream.log("Valor Cavern: Outdoor overworld not ready.", COLOR_RED);
            return false;
        }
    }
    context.wait_for_all_requests();
    stream.log("Lake Valor: Saturn defeated; outside Valor Cavern doorway, facing south, Repel inactive.", COLOR_GREEN);
    return true;
}

static bool return_to_twinleaf_and_heal(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats){
    if (!fly_to(stream, context, FlyPoint::TwinleafTown)){
        return false;
    }
    context.wait_for_all_requests();
    OverworldWatcher landed;
    if (wait_until(stream, context, 10s, {{landed}}) != 0){
        stream.log("Twinleaf home: Fly landing overworld not ready.", COLOR_RED);
        return false;
    }

    // Fly lands directly in front of the home doorway. The old west hold
    // walked into the trees; enter north and stop movement when the fade starts.
    {
        BlackScreenWatcher black;
        WhiteScreenWatcher white;
        const int result = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {0, +1}, 10000ms, 100ms);
            },
            {{black}, {white}});
        if (result < 0){
            stream.log("Twinleaf home: Entrance transition did not start.", COLOR_RED);
            return false;
        }
        BlackScreenWatcher black_gone(COLOR_RED,
            {0.1, 0.1, 0.8, 0.8}, 100, 10, BlackScreenWatcher::FinderType::GONE, 500ms);
        WhiteScreenWatcher white_gone(COLOR_RED,
            {0.1, 0.1, 0.8, 0.8}, 500, 10, WhiteScreenWatcher::FinderType::GONE, 500ms);
        const int finished = result == 1 ? wait_until(stream, context, 15s, {{white_gone}})
            : wait_until(stream, context, 15s, {{black_gone}});
        if (finished != 0){
            stream.log("Twinleaf home: Entrance transition did not finish.", COLOR_RED);
            return false;
        }
        OverworldWatcher inside;
        if (wait_until(stream, context, 10s, {{inside}}) != 0){
            stream.log("Twinleaf home: Indoor overworld not ready.", COLOR_RED);
            return false;
        }
    }

    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("heal_at_home: sleeping at mom's...");

    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 1);
    context.wait_for_all_requests();

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
            stream.log("Twinleaf home: Mom dialogue detected; completing healing.", COLOR_GREEN);
        }else if (ret < 0){
            stream.log("heal_at_home: Didn't detect mom dialog box!", COLOR_RED);
            return false;
        }
    }
    // Finish dialogue and any healing fade before starting the exit route.
    clear_dialog(stream, context, BUTTON_B, 6000ms);
    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {0, -1}, 400ms, 100ms); // 1+
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);

    context.wait_for_all_requests();

    {
        BlackScreenWatcher black;
        WhiteScreenWatcher white;
        const int result = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {0, -1}, 10000ms, 100ms);
            },
            {{black}, {white}});
        if (result < 0){
            stream.log("Twinleaf home: Exit transition did not start.", COLOR_RED);
            return false;
        }
        BlackScreenWatcher black_gone(COLOR_RED,
            {0.1, 0.1, 0.8, 0.8}, 100, 10, BlackScreenWatcher::FinderType::GONE, 500ms);
        WhiteScreenWatcher white_gone(COLOR_RED,
            {0.1, 0.1, 0.8, 0.8}, 500, 10, WhiteScreenWatcher::FinderType::GONE, 500ms);
        const int finished = result == 1 ? wait_until(stream, context, 15s, {{white_gone}})
            : wait_until(stream, context, 15s, {{black_gone}});
        if (finished != 0){
            stream.log("Twinleaf home: Exit transition did not finish.", COLOR_RED);
            return false;
        }
        OverworldWatcher outside;
        if (wait_until(stream, context, 10s, {{outside}}) != 0){
            stream.log("Twinleaf home: Outdoor overworld not ready.", COLOR_RED);
            return false;
        }
    }

    context.wait_for_all_requests();
    stats.move_pp_remaining = stats.move_pp_capacity;
    stream.log("Twinleaf home: Healing route complete; outside the doorway, facing south, Repel inactive.", COLOR_GREEN);
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

    // Arrival starts Rowan's conversation before the trainer approach. Moving
    // while it is open loses the route inputs and leaves no battle to detect.
    wait_for_dialog(stream, context, "Lake Verity Rowan arrival", 15s);
    clear_dialog(stream, context, BUTTON_B, 6000ms);

    pbf_move_left_joystick(context, {0, +1}, 600ms, 100ms); // 2+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 2);

    {
        context.wait_for_all_requests();
        MarkDetector grunt_1_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 3200ms, 100ms); // 16
            },
            {{grunt_1_fight}});
        if (ret < 0){
            stream.log("Lake Verity Grunt 1: Approach did not start an encounter.", COLOR_RED);
            return false;
        }
        if (ret == 0){
            context.wait_for_all_requests();
            pbf_wait(context, 2000ms);
            context.wait_for_all_requests();

            BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            ret = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 30000ms);
                },
                {{black_screen}});
            if (ret < 0){
                stream.log("commander_mars_at_verity_grunt_1: black screen not detected!", COLOR_RED);
                return false;
            }else if (ret == 0){
                fight_trainer_using_moves(stream, context, stats, "verity_lakefront_grunt_1", false);
            }
        }
    }

    // The next Grunt starts automatically after the battle exit. Hand off
    // immediately; overworld cleanup would consume the next battle intro.
    stream.log("Lake Verity: handling the consecutive Grunt 2 battle.");
    fight_trainer_using_moves(stream, context, stats, "verity_lakefront_grunt_2", true);

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();

    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 2);

    pbf_move_left_joystick(context, {+1, 0}, 1400ms, 100ms); // 7+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 2);
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 1);

    {
        context.wait_for_all_requests();
        MarkDetector grunt_3_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 3200ms, 100ms); // 16
            },
            {{grunt_3_fight}});
        if (ret < 0){
            stream.log("Lake Verity Grunt 3: Approach did not start an encounter.", COLOR_RED);
            return false;
        }
        if (ret == 0){
            context.wait_for_all_requests();
            pbf_wait(context, 2000ms);
            context.wait_for_all_requests();

            BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            ret = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 30000ms);
                },
                {{black_screen}});
            if (ret < 0){
                stream.log("commander_mars_at_verity_grunt_3: black screen not detected!", COLOR_RED);
                return false;
            }else if (ret == 0){
                fight_trainer_using_moves(stream, context, stats, "verity_lakefront_grunt_3", false);
            }
        }
    }

    // The next Grunt starts automatically after the battle exit. Hand off
    // immediately; overworld cleanup would consume the next battle intro.
    stream.log("Lake Verity: handling the consecutive Grunt 4 battle.");
    fight_trainer_using_moves(stream, context, stats, "verity_lakefront_grunt_4", true);

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();

    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 1);
    pbf_move_left_joystick(context, {+1, 0}, 800ms, 100ms); // 3+

    {
        context.wait_for_all_requests();
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
            context.wait_for_all_requests();

            BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            ret = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 30000ms);
                },
                {{black_screen}});
            if (ret < 0){
                stream.log("commander_mars_at_verity_commander_mars: black screen not detected!", COLOR_RED);
                return false;
            }else if (ret == 0){
                fight_trainer_using_moves(stream, context, stats, "verity_lakefront_commander_mars", true);
            }
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    pbf_mash_button(context, BUTTON_B, 10000ms);
    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    clear_dialog(stream, context, BUTTON_B, 6000ms);
    context.wait_for_all_requests();
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    // Finish at the Fly landing point so the next checkpoint can save and
    // restart the Mart route without depending on the Mars battle position.
    if (!fly_to(stream, context, FlyPoint::SnowpointCity)){
        return false;
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    OverworldWatcher snowpoint_landing;
    if (wait_until(stream, context, 15s, {{snowpoint_landing}}) != 0){
        stream.log("Lake Verity return: Snowpoint landing was not ready.", COLOR_RED);
        return false;
    }
    stream.log("Lake Verity return: ready outside Snowpoint Pokemon Center.", COLOR_GREEN);
    return true;
}

static bool restock_and_heal_snowpoint(VideoStream& stream,
    ProControllerContext& context,
    AutoStoryStats& stats,
    Language language){
    if (!restock_super_repels_snowpoint(stream, context, 10, language)){
        return false;
    }
    // The recorded Mart route ends outside the shop. Fly back to the known
    // Center landing point before reusing its entry and healing sequence.
    if (!fly_to(stream, context, FlyPoint::SnowpointCity)){
        return false;
    }
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
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
            stream.log("restock_and_heal_snowpoint: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("restock_and_heal_snowpoint: entered Snowpoint City Pokemon Center", COLOR_GREEN);
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
    // The first-entry camera sweep lasts about seven seconds after the fade.
    // Wait for it to return to the entrance before sending puzzle movements.
    pbf_wait(context, 10000ms);
    context.wait_for_all_requests();
    return true;
}

// Start: inside Snowpoint Gym on the entrance tile, facing north.
// End: outside the Gym doorway, facing south, after Candice's rewards.
static bool snowpoint_gym_puzzle_and_candice(VideoStream& stream,
    ProControllerContext& context,
    AutoStoryStats& stats){
    struct Move {
        DpadPosition dir;
        int count;
        Milliseconds hold;
        Milliseconds delay;
    };

    DpadState dpad;
    dpad.last_dir = DPAD_UP;
    // Use explicit 80ms facing presses. When the recording also moved on a
    // direction change, release before a separate 80ms movement press.
    // These turns are already in the tables: repeat_dpad must not add another.
    // Retain slide/stair settling time; trim recorded idle pauses only where
    // the replay shows arrival. Omitted inputs retain time in the preceding pause.
    const std::vector<Move> sequence1 = {
        {DPAD_UP, 1, 109ms, 424ms},
        {DPAD_UP, 1, 144ms, 554ms},
        {DPAD_UP, 1, 145ms, 1024ms},
        {DPAD_LEFT, 1, 80ms, 300ms},
        {DPAD_LEFT, 1, 113ms, 1037ms},
        {DPAD_LEFT, 1, 126ms, 2500ms},
        {DPAD_UP, 1, 80ms, 300ms},
        {DPAD_UP, 1, 80ms, 922ms},
        {DPAD_UP, 1, 114ms, 1370ms},
        {DPAD_UP, 1, 144ms, 1440ms},
        {DPAD_RIGHT, 1, 80ms, 300ms},
        {DPAD_RIGHT, 1, 128ms, 2800ms},
        {DPAD_DOWN, 1, 80ms, 868ms},
        {DPAD_DOWN, 1, 113ms, 871ms},
        {DPAD_LEFT, 1, 80ms, 300ms},
        {DPAD_LEFT, 1, 107ms, 2500ms},
        {DPAD_LEFT, 1, 144ms, 1170ms},
        {DPAD_DOWN, 1, 80ms, 300ms},
        {DPAD_DOWN, 1, 80ms, 1340ms},
        {DPAD_LEFT, 1, 80ms, 300ms},
        {DPAD_LEFT, 1, 80ms, 398ms},
        {DPAD_LEFT, 1, 154ms, 1322ms},
        {DPAD_UP, 1, 80ms, 300ms},
        {DPAD_UP, 1, 120ms, 1947ms},
        {DPAD_UP, 1, 117ms, 2200ms},
        {DPAD_RIGHT, 1, 80ms, 300ms},
        {DPAD_RIGHT, 1, 96ms, 2500ms},
        {DPAD_RIGHT, 1, 142ms, 2274ms},
        {DPAD_UP, 1, 80ms, 300ms},
        {DPAD_UP, 1, 80ms, 1171ms},
        {DPAD_RIGHT, 1, 80ms, 300ms},
        {DPAD_RIGHT, 1, 80ms, 446ms},
        {DPAD_RIGHT, 1, 126ms, 1752ms},
        {DPAD_UP, 1, 80ms, 300ms},
        {DPAD_UP, 1, 80ms, 1209ms},
        {DPAD_LEFT, 1, 80ms, 300ms},
        {DPAD_LEFT, 1, 141ms, 1289ms},
        {DPAD_LEFT, 1, 110ms, 2271ms},
        {DPAD_DOWN, 1, 80ms, 300ms},
        {DPAD_DOWN, 1, 112ms, 2800ms},
        {DPAD_LEFT, 1, 80ms, 300ms},
        {DPAD_LEFT, 1, 125ms, 2000ms},
        {DPAD_LEFT, 1, 128ms, 1640ms},
        {DPAD_UP, 1, 80ms, 300ms},
        {DPAD_UP, 1, 80ms, 392ms},
        {DPAD_RIGHT, 1, 80ms, 300ms},
        {DPAD_RIGHT, 1, 80ms, 2500ms},
        {DPAD_DOWN, 1, 80ms, 300ms},
        // Let the south slide finish before turning west; an early turn is swallowed.
        {DPAD_DOWN, 1, 80ms, 2000ms},
        {DPAD_LEFT, 1, 80ms, 300ms},
        {DPAD_LEFT, 1, 80ms, 1825ms},
        {DPAD_LEFT, 1, 126ms, 1556ms},
        {DPAD_DOWN, 1, 80ms, 300ms},
        {DPAD_DOWN, 1, 80ms, 1889ms},
        {DPAD_LEFT, 1, 80ms, 300ms},
        {DPAD_LEFT, 1, 124ms, 810ms},
        {DPAD_LEFT, 1, 124ms, 2157ms},
        {DPAD_UP, 1, 80ms, 300ms},
        {DPAD_UP, 1, 80ms, 1938ms},
        {DPAD_LEFT, 1, 80ms, 300ms},
        {DPAD_LEFT, 1, 161ms, 721ms},
        {DPAD_UP, 1, 80ms, 300ms},
        {DPAD_UP, 1, 80ms, 1706ms},
        {DPAD_RIGHT, 1, 80ms, 300ms},
        {DPAD_RIGHT, 1, 128ms, 3238ms},
    };

    const std::vector<Move> sequence2 = {
        {DPAD_RIGHT, 1, 80ms, 300ms},
        {DPAD_RIGHT, 1, 80ms, 7056ms},
        {DPAD_RIGHT, 1, 129ms, 3690ms},
    };

    const std::vector<Move> sequence3 = {
        {DPAD_DOWN, 1, 80ms, 300ms},
        {DPAD_DOWN, 1, 80ms, 2125ms},
        {DPAD_LEFT, 1, 80ms, 300ms},
        {DPAD_LEFT, 1, 125ms, 1788ms},
        {DPAD_LEFT, 1, 129ms, 1473ms},
        {DPAD_DOWN, 1, 80ms, 300ms},
        {DPAD_DOWN, 1, 80ms, 1256ms},
        {DPAD_LEFT, 1, 80ms, 300ms},
        {DPAD_LEFT, 1, 80ms, 807ms},
        {DPAD_LEFT, 1, 158ms, 659ms},
        {DPAD_DOWN, 1, 80ms, 300ms},
        {DPAD_DOWN, 1, 143ms, 1288ms},
        {DPAD_RIGHT, 1, 80ms, 300ms},
        {DPAD_RIGHT, 1, 112ms, 589ms},
        {DPAD_RIGHT, 1, 126ms, 1290ms},
        {DPAD_UP, 1, 80ms, 300ms},
        {DPAD_UP, 1, 80ms, 1305ms},
        {DPAD_UP, 1, 164ms, 753ms},
        {DPAD_UP, 1, 179ms, 2500ms},
        {DPAD_LEFT, 1, 80ms, 300ms},
        {DPAD_LEFT, 1, 145ms, 1419ms},
        {DPAD_DOWN, 1, 80ms, 300ms},
        {DPAD_DOWN, 1, 80ms, 1340ms},
        {DPAD_RIGHT, 1, 80ms, 300ms},
        {DPAD_RIGHT, 1, 80ms, 2500ms},
        {DPAD_LEFT, 1, 80ms, 300ms},
        // Finish the west slide before the south-facing input; one second
        // swallowed that turn in the 2026-11-08 replay. Restore the verified wait.
        {DPAD_LEFT, 1, 176ms, 2538ms},
        {DPAD_DOWN, 1, 80ms, 300ms},
        {DPAD_DOWN, 1, 80ms, 3390ms},
        {DPAD_LEFT, 1, 80ms, 300ms},
        {DPAD_LEFT, 1, 144ms, 1041ms},
        {DPAD_LEFT, 1, 144ms, 354ms},
        {DPAD_LEFT, 1, 143ms, 892ms},
        {DPAD_RIGHT, 1, 80ms, 300ms},
        {DPAD_RIGHT, 1, 80ms, 521ms},
        {DPAD_DOWN, 1, 80ms, 300ms},
        {DPAD_DOWN, 1, 80ms, 874ms},
        {DPAD_DOWN, 1, 161ms, 1172ms},
        {DPAD_RIGHT, 1, 80ms, 300ms},
        {DPAD_RIGHT, 1, 145ms, 1557ms},
        {DPAD_RIGHT, 1, 161ms, 455ms},
        {DPAD_RIGHT, 1, 159ms, 625ms},
        {DPAD_UP, 1, 80ms, 300ms},
        {DPAD_UP, 1, 80ms, 321ms},
        {DPAD_UP, 1, 164ms, 504ms},
        {DPAD_UP, 1, 177ms, 1500ms},
        {DPAD_UP, 1, 211ms, 886ms},
        {DPAD_UP, 1, 246ms, 1057ms},
        {DPAD_UP, 1, 191ms, 973ms},
        {DPAD_UP, 1, 210ms, 1124ms},
        {DPAD_UP, 1, 194ms, 990ms},
        {DPAD_UP, 1, 176ms, 1074ms},
        {DPAD_UP, 1, 160ms, 1155ms},
        {DPAD_UP, 1, 178ms, 872ms},
    };

    const std::vector<Move> exit_sequence = {
        {DPAD_DOWN, 1, 80ms, 300ms},
        {DPAD_DOWN, 1, 80ms, 558ms},
        {DPAD_DOWN, 1, 158ms, 708ms},
        {DPAD_DOWN, 1, 177ms, 656ms},
        {DPAD_DOWN, 1, 163ms, 722ms},
        {DPAD_DOWN, 1, 191ms, 742ms},
        {DPAD_DOWN, 1, 211ms, 804ms},
        {DPAD_DOWN, 1, 161ms, 673ms},
        {DPAD_DOWN, 1, 176ms, 957ms},
        {DPAD_DOWN, 1, 177ms, 440ms},
        {DPAD_DOWN, 1, 126ms, 558ms},
        {DPAD_DOWN, 1, 175ms, 0ms},
    };

    const std::vector<Move>* sequences[] = {
        &sequence1,
        &sequence2,
        &sequence3,
    };
    const std::string labels[] = {
        "Snowpoint Gym Ace Trainer Brenna",
        "Snowpoint Gym Ace Trainer Isaiah",
        "Candice",
    };

    for (size_t stage = 0; stage < 3; stage++){
        const bool gym_leader = stage == 2;
        const std::string& label = labels[stage];
        stream.log("[Snowpoint Gym puzzle] Approaching " + label + ".");
        const auto& sequence = *sequences[stage];
        // Ice tiles can resemble the dialogue detector's white margins.
        // Traverse the puzzle before arming it for the final trainer approach.
        for (size_t i = 0; i + 1 < sequence.size(); i++){
            const auto& move = sequence[i];
            repeat_dpad(context, dpad, move.dir, move.hold, 0ms, move.count, false);
            pbf_wait(context, move.delay);
            context.wait_for_all_requests();
        }
        const auto& final_move = sequence.back();
        if (gym_leader){
            repeat_dpad(context, dpad, final_move.dir, final_move.hold, 0ms, final_move.count, false);
            pbf_wait(context, final_move.delay);
            context.wait_for_all_requests();
        }else{
            ShortDialogWatcher trainer_dialogue(COLOR_RED);
            const int dialogue_result = run_until<ProControllerContext>(stream,
                context,
                [&](ProControllerContext& ctx){
                    repeat_dpad(ctx, dpad, final_move.dir, final_move.hold, 0ms, final_move.count, false);
                    pbf_wait(ctx, final_move.delay);
                },
                {{trainer_dialogue}});
            context.wait_for_all_requests();
            if (dialogue_result < 0){
                wait_for_dialog(stream, context, label, 15s);
            }
        }
        pbf_press_button(context, BUTTON_A, 150ms, 300ms);
        fight_trainer_using_moves(stream, context, stats, label, gym_leader);
        clear_dialog(stream, context, BUTTON_B, gym_leader ? 6000ms : 3000ms);
    }

    // The recorded right-stick input produced no route displacement. Return
    // straight south through the cleared puzzle; stop input at the exit fade.
    context.wait_for_all_requests();
    BlackScreenWatcher exit_black;
    const int exit_result = run_until<ProControllerContext>(stream,
        context,
        [&](ProControllerContext& ctx){
            for (const auto& move : exit_sequence){
                repeat_dpad(ctx, dpad, move.dir, move.hold, 0ms, move.count, false);
                pbf_wait(ctx, move.delay);
            }
            pbf_wait(ctx, 5000ms);
        },
        {{exit_black}});
    if (exit_result != 0){
        stream.log("Snowpoint Gym: Exit transition did not start.", COLOR_RED);
        return false;
    }
    BlackScreenWatcher exit_gone(COLOR_RED,
        {0.1, 0.1, 0.8, 0.8}, 100, 10, BlackScreenWatcher::FinderType::GONE, 500ms);
    if (wait_until(stream, context, 15s, {{exit_gone}}) != 0){
        stream.log("Snowpoint Gym: Exit transition did not finish.", COLOR_RED);
        return false;
    }
    OverworldWatcher outside;
    if (wait_until(stream, context, 10s, {{outside}}) != 0){
        stream.log("Snowpoint Gym: Outdoor overworld not ready.", COLOR_RED);
        return false;
    }
    context.wait_for_all_requests();
    stream.log("Snowpoint Gym complete: rewards finished; outside the doorway, facing south.", COLOR_GREEN);
    return true;
}


// ---------------------------------------------------------------------------
//  Segment wrappers
// ---------------------------------------------------------------------------

std::string AutoStory_Segment_07::name() const {
    return "07: Canalave to Snowpoint";
}

std::string AutoStory_Segment_07::start_text() const {
    return AutoStory_Checkpoint_0701().start_text();
}

std::string AutoStory_Segment_07::end_text() const {
    return AutoStory_Checkpoint_0707().end_text();
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
    return "Inside Canalave Gym at the entrance, facing east, after defeating Byron and acquiring the Mine Badge, with reward dialogue finished.";
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
    return "Inside the Route 216 rest house beside the bed after healing, with dialogue finished.";
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
    return "07.03 - Route 217, Snowpoint Mart and Veilstone";
}

std::string AutoStory_Checkpoint_0703::start_text() const {
    return "Inside the Route 216 rest house beside the bed after healing, with dialogue finished.";
}

std::string AutoStory_Checkpoint_0703::end_text() const {
    return "Bought 21 Super Repels in Snowpoint; healed and outside Veilstone Pokemon Center after the Rock Climb pickup, Repel inactive.";
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
            if (!leave_small_house_route_216(env.console, context)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "07.03: rest house exit did not complete.",
                    env.console);
            }
            if (!continue_through_route_216(env.console, context, stats)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "07.03: continue_through_route_216 did not complete.",
                    env.console);
            }
            if (!restock_super_repels_snowpoint(env.console, context, 21, options.language)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "07.03: Snowpoint Mart restock did not complete.",
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
    return "07.04 - Lake Valor and Saturn";
}

std::string AutoStory_Checkpoint_0704::start_text() const {
    return "Healed and outside Veilstone Pokemon Center after the first Snowpoint visit and Rock Climb pickup, Repel inactive.";
}

std::string AutoStory_Checkpoint_0704::end_text() const {
    return "Saturn defeated; healed and outside the Twinleaf home, facing south, Repel inactive.";
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
            if (!go_through_lake_valor(env.console, context, stats)){
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
    return "07.05 - Lake Verity, Mars and Snowpoint";
}

std::string AutoStory_Checkpoint_0705::start_text() const {
    return "Saturn defeated; healed and outside the Twinleaf home, facing south, Repel inactive.";
}

std::string AutoStory_Checkpoint_0705::end_text() const {
    return "Mars defeated and follow-up dialogue finished; outside Snowpoint Pokemon Center at the Fly landing point, facing south, Repel inactive.";
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
    return "07.06 - Snowpoint restock and healing";
}

std::string AutoStory_Checkpoint_0706::start_text() const {
    return "Mars defeated and follow-up dialogue finished; outside Snowpoint Pokemon Center at the Fly landing point, facing south, Repel inactive.";
}

std::string AutoStory_Checkpoint_0706::end_text() const {
    return "Bought 10 additional Super Repels; healed and outside Snowpoint Pokemon Center at the doorway, Repel inactive.";
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
            if (!restock_and_heal_snowpoint(env.console, context, stats, options.language)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "07.06: restock_and_heal_snowpoint did not complete.",
                    env.console);
            }
        },
        true);
}

std::string AutoStory_Checkpoint_0707::name() const {
    return "07.07 - Snowpoint Gym and Candice";
}

std::string AutoStory_Checkpoint_0707::start_text() const {
    return "Bought 10 additional Super Repels; healed and outside Snowpoint Pokemon Center at the doorway, Repel inactive.";
}

std::string AutoStory_Checkpoint_0707::end_text() const {
    return "Outside Snowpoint Gym at the doorway, facing south, with seventh-badge rewards and dialogue finished, Repel inactive.";
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
            if (!snowpoint_gym_puzzle_and_candice(env.console, context, stats)){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "07.07: Snowpoint Gym puzzle, Candice battle, or exit did not complete.",
                    env.console);
            }
        },
        true);
}

}
}
}
