/*  BDSP AutoStory - Segment 07
 *  From after acquiring the sixth badge until after acquiring the seventh badge.
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
#include "../PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP_AutoStory_Segment_07.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


std::string AutoStory_Segment_07::name()       const{ return "07: Sixth Badge to Seventh Badge"; }
std::string AutoStory_Segment_07::start_text() const{ return "Start: Sixth Gym Badge acquired (Canalave City)."; }
std::string AutoStory_Segment_07::end_text()   const{ return "End: Seventh Gym Badge acquired (Snowpoint City)."; }

void AutoStory_Segment_07::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_093().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_094().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_095().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_096().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_097().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_098().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_099().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_100().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_101().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_102().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_103().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_104().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_105().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_106().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_107().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_108().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_109().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_110().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}


std::string AutoStory_Checkpoint_093::name()       const{ return "007.01 - leave_gym"; }
std::string AutoStory_Checkpoint_093::start_text() const{ return "Inside Canalave City gym after sixth badge."; }
std::string AutoStory_Checkpoint_093::end_text()   const{ return "Exited Canalave City gym."; }
void AutoStory_Checkpoint_093::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_093(env, context, options, stats);
}

std::string AutoStory_Checkpoint_094::name()       const{ return "007.02 - go_to_the_library"; }
std::string AutoStory_Checkpoint_094::start_text() const{ return "Outside Canalave City gym."; }
std::string AutoStory_Checkpoint_094::end_text()   const{ return "Entered Canalave City library."; }
void AutoStory_Checkpoint_094::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_094(env, context, options, stats);
}

std::string AutoStory_Checkpoint_095::name()       const{ return "007.03 - drama_at_the_library"; }
std::string AutoStory_Checkpoint_095::start_text() const{ return "Inside Canalave City library."; }
std::string AutoStory_Checkpoint_095::end_text()   const{ return "Library drama complete, exited library."; }
void AutoStory_Checkpoint_095::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_095(env, context, options, stats);
}

std::string AutoStory_Checkpoint_096::name()       const{ return "007.04 - talk_fly_and_enter_mount_coronet"; }
std::string AutoStory_Checkpoint_096::start_text() const{ return "Outside Canalave City library."; }
std::string AutoStory_Checkpoint_096::end_text()   const{ return "Entered Mt. Coronet."; }
void AutoStory_Checkpoint_096::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_096(env, context, options, stats);
}

std::string AutoStory_Checkpoint_097::name()       const{ return "007.05 - go_through_mount_coronet"; }
std::string AutoStory_Checkpoint_097::start_text() const{ return "Inside Mt. Coronet."; }
std::string AutoStory_Checkpoint_097::end_text()   const{ return "Exited Mt. Coronet."; }
void AutoStory_Checkpoint_097::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_097(env, context, options, stats);
}

std::string AutoStory_Checkpoint_098::name()       const{ return "007.06 - go_through_route_216"; }
std::string AutoStory_Checkpoint_098::start_text() const{ return "On Route 216."; }
std::string AutoStory_Checkpoint_098::end_text()   const{ return "Entered small chalet on Route 216."; }
void AutoStory_Checkpoint_098::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_098(env, context, options, stats);
}

std::string AutoStory_Checkpoint_099::name()       const{ return "007.07 - heal_small_house_route_216"; }
std::string AutoStory_Checkpoint_099::start_text() const{ return "Inside small chalet on Route 216."; }
std::string AutoStory_Checkpoint_099::end_text()   const{ return "Rested in chalet and exited."; }
void AutoStory_Checkpoint_099::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_099(env, context, options, stats);
}

std::string AutoStory_Checkpoint_100::name()       const{ return "007.08 - continue_through_route_216"; }
std::string AutoStory_Checkpoint_100::start_text() const{ return "Outside chalet on Route 216."; }
std::string AutoStory_Checkpoint_100::end_text()   const{ return "Entered Veilstone City Pokemon Center."; }
void AutoStory_Checkpoint_100::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_100(env, context, options, stats);
}

std::string AutoStory_Checkpoint_101::name()       const{ return "007.09 - heal_and_exit_veilstone_pokemon_center"; }
std::string AutoStory_Checkpoint_101::start_text() const{ return "Inside Veilstone City Pokemon Center."; }
std::string AutoStory_Checkpoint_101::end_text()   const{ return "Healed and exited Veilstone City Pokemon Center."; }
void AutoStory_Checkpoint_101::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_101(env, context, options, stats);
}

std::string AutoStory_Checkpoint_102::name()       const{ return "007.10 - go_to_house_south_veilstone"; }
std::string AutoStory_Checkpoint_102::start_text() const{ return "Outside Veilstone City Pokemon Center."; }
std::string AutoStory_Checkpoint_102::end_text()   const{ return "Entered house south of Veilstone City."; }
void AutoStory_Checkpoint_102::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_102(env, context, options, stats);
}

std::string AutoStory_Checkpoint_103::name()       const{ return "007.11 - go_through_house_south_veilstone"; }
std::string AutoStory_Checkpoint_103::start_text() const{ return "Inside house south of Veilstone City."; }
std::string AutoStory_Checkpoint_103::end_text()   const{ return "Exited house onto Route 214."; }
void AutoStory_Checkpoint_103::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_103(env, context, options, stats);
}

std::string AutoStory_Checkpoint_104::name()       const{ return "007.12 - walk_through_route_214"; }
std::string AutoStory_Checkpoint_104::start_text() const{ return "On Route 214 heading south."; }
std::string AutoStory_Checkpoint_104::end_text()   const{ return "Entered Valor Lakefront."; }
void AutoStory_Checkpoint_104::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_104(env, context, options, stats);
}

std::string AutoStory_Checkpoint_105::name()       const{ return "007.13 - go_through_lake_valor"; }
std::string AutoStory_Checkpoint_105::start_text() const{ return "At Valor Lakefront."; }
std::string AutoStory_Checkpoint_105::end_text()   const{ return "Flew to Twinleaf Town and entered own house."; }
void AutoStory_Checkpoint_105::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_105(env, context, options, stats);
}

std::string AutoStory_Checkpoint_106::name()       const{ return "007.14 - heal_at_home"; }
std::string AutoStory_Checkpoint_106::start_text() const{ return "Inside own house in Twinleaf Town."; }
std::string AutoStory_Checkpoint_106::end_text()   const{ return "Healed at home and exited."; }
void AutoStory_Checkpoint_106::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_106(env, context, options, stats);
}

std::string AutoStory_Checkpoint_107::name()       const{ return "007.15 - go_to_lake_verity"; }
std::string AutoStory_Checkpoint_107::start_text() const{ return "Outside own house in Twinleaf Town."; }
std::string AutoStory_Checkpoint_107::end_text()   const{ return "Entered Verity Lakefront."; }
void AutoStory_Checkpoint_107::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_107(env, context, options, stats);
}

std::string AutoStory_Checkpoint_108::name()       const{ return "007.16 - commander_mars_and_snowpoint"; }
std::string AutoStory_Checkpoint_108::start_text() const{ return "At Lake Verity."; }
std::string AutoStory_Checkpoint_108::end_text()   const{ return "Defeated Commander Mars and entered Snowpoint City Pokemon Center."; }
void AutoStory_Checkpoint_108::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_108(env, context, options, stats);
}

std::string AutoStory_Checkpoint_109::name()       const{ return "007.17 - heal_at_snowpoint_pokemon_center"; }
std::string AutoStory_Checkpoint_109::start_text() const{ return "Inside Snowpoint City Pokemon Center."; }
std::string AutoStory_Checkpoint_109::end_text()   const{ return "Healed and exited Snowpoint City Pokemon Center."; }
void AutoStory_Checkpoint_109::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_109(env, context, options, stats);
}

std::string AutoStory_Checkpoint_110::name()       const{ return "007.18 - go_to_snowpoint_gym"; }
std::string AutoStory_Checkpoint_110::start_text() const{ return "Outside Snowpoint City Pokemon Center."; }
std::string AutoStory_Checkpoint_110::end_text()   const{ return "Seventh Gym Badge acquired (Snowpoint City)."; }
void AutoStory_Checkpoint_110::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_110(env, context, options, stats);
}

static void move_back_after_unexpected_battle(
    VideoStream& stream,
    ProControllerContext& context,
    const std::string& trainerid
){
    stream.log("Finishing battle with " + trainerid, COLOR_GREEN);
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    if (trainerid == "route_216_ace_maria") {
        pbf_move_left_joystick(context, {-1, 0}, 1600ms, 100ms); // 8
    } else if (trainerid == "route_215_skier_edward") {
        pbf_move_left_joystick(context, {-1, 0}, 8100ms, 100ms); // 27++ (snow)
    } else if (trainerid == "route_217_skier_lexie") {
        pbf_move_left_joystick(context, {0, +1}, 2100ms, 100ms); // 7++ (snow)
    } else if (trainerid == "route_217_blackbelt_luke") {
        pbf_move_left_joystick(context, {0, +1}, 5100ms, 100ms); // 17++ (snow)
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
        trainerid == "route_216_ace_maria" or
        trainerid == "route_216_skier_edward" or
        trainerid == "route_217_skier_lexie" or
        trainerid == "route_217_blackbelt_luke" or
        trainerid == "route_217_ace_olivia" or
        trainerid == "verity_lakefront_grunt_1" or
        trainerid == "verity_lakefront_grunt_3"
    ) {
        // Select Bulldoze
        pbf_press_dpad(context, DPAD_DOWN, 280ms, 200ms);
    } else if (
        trainerid == "route_217_ace_dalton"
    ) {
        // Select Razor leaf
        pbf_press_dpad(context, DPAD_UP, 280ms, 200ms);
    } else if (
        trainerid == "verity_lakefront_grunt_2" or
        trainerid == "verity_lakefront_grunt_4" or
        trainerid == "verity_lakefront_commander_mars"
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

static bool go_to_the_library(
    VideoStream& stream,
    ProControllerContext& context
){
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
        } else if (ret == 0) {
            stream.log("go_to_the_library_talk_to_clint: dialog box detected.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 7000ms);
            context.wait_for_all_requests();
        }
    }

    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 2);
    pbf_move_left_joystick(context, {-1, 0}, 1600ms, 100ms); // 8+
    pbf_move_left_joystick(context, {0, +1}, 3800ms, 100ms); // 19+
    pbf_move_left_joystick(context, {+1, 0}, 600ms, 100ms); // 2+
    pbf_move_left_joystick(context, {0, -1}, 1000ms, 100ms); // 4+
    dpad.last_dir = DPAD_DOWN;
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
            stream.log("go_to_the_library: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_to_the_library: entered the library", COLOR_GREEN);
    };
    return true;
}

static bool drama_at_the_library(
    VideoStream& stream,
    ProControllerContext& context
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("drama_at_the_library: going to professor Rowan...");

    pbf_move_left_joystick(context, {0, +1}, 1400ms, 100ms); // 7+

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
            stream.log("drama_at_the_library: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("drama_at_the_library: moved to the first floor", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    pbf_move_left_joystick(context, {0, -1}, 600ms, 100ms); // 3+
    pbf_move_left_joystick(context, {+1, 0}, 1400ms, 100ms); // 7
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
        } else if (ret == 0) {
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
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
            stream.log("drama_at_the_library: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("drama_at_the_library: moved back to the ground floor", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

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
            stream.log("drama_at_the_library: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("drama_at_the_library: moved out of the library", COLOR_GREEN);
    };
    return true;
}

static bool talk_fly_and_enter_mount_coronet(
    VideoStream& stream,
    ProControllerContext& context
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("talk_fly_and_enter_mount_coronet: talking, flying and becoming a speleologist...");

    {
        ShortDialogWatcher talk_to_rowan(COLOR_BLUE);
        int ret = wait_until(stream, context, std::chrono::seconds(30), {{talk_to_rowan}});
        if (ret < 0){
            stream.log("talk_fly_and_enter_mount_coronet_talk_to_rowan: dialog box not detected!", COLOR_RED);
            return false;
        } else if (ret == 0) {
            stream.log("talk_fly_and_enter_mount_coronet_talk_to_rowan: dialog box detected.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_B, 21000ms);
            context.wait_for_all_requests();
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    fly_to(stream, context, FlyPoint::CelesticTown);

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    stream.log("talk_fly_and_enter_mount_coronet: going towards Mt. Coronet...");

    pbf_move_left_joystick(context, {-1, 0}, 2000ms, 100ms); // 10+
    pbf_move_left_joystick(context, {0, -1}, 800ms, 100ms); // 3+
    pbf_move_left_joystick(context, {-1, 0}, 1800ms, 100ms); // 9+
    pbf_move_left_joystick(context, {0, +1}, 800ms, 100ms); // 3+
    pbf_move_left_joystick(context, {-1, 0}, 600ms, 100ms); // 2+

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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("talk_fly_and_enter_mount_coronet: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("talk_fly_and_enter_mount_coronet: entered Mt. Coronet", COLOR_GREEN);
    };
    return true;
}

static bool go_through_mount_coronet(
    VideoStream& stream,
    ProControllerContext& context
){
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

    use_strength(stream, context);

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, +1}, 10000ms, 500ms); // 10++ (strength)
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);
    pbf_move_left_joystick(context, {0, +1}, 2400ms, 100ms); // 12+

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
            stream.log("go_through_mount_coronet: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_mount_coronet: finished first part of Mt. Coronet", COLOR_GREEN);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);
    stream.log("go_through_mount_coronet: continuing through Mt. Coronet...");

    pbf_move_left_joystick(context, {-1, 0}, 1000ms, 100ms); // 4+
    pbf_move_left_joystick(context, {0, +1}, 600ms, 100ms); // 2+

    {
        ShortDialogWatcher repel_dialog(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 11000ms, 100ms); // 55+
            },
            {{repel_dialog}}
        );
        if (ret == 0){
            stream.log("go_through_mount_coronet: Reactivating repel.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 3000ms);
        } else if (ret < 0){
                stream.log("go_through_mount_coronet: Didn't detect repel dialog box!", COLOR_RED);
                return false;
        }
    }
    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, +1}, 5600ms, 100ms); // 28+

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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 10000ms, 100ms);
            },
            {{white_screen}}
        );
        if (ret < 0){
            stream.log("go_through_mount_coronet: white screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_mount_coronet: exited Mt. Coronet", COLOR_GREEN);
    };

    return true;
}

static bool go_through_route_216(
    VideoStream& stream,
    ProControllerContext& context
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_through_route_216: going through the snow...");

    pbf_move_left_joystick(context, {-1, 0}, 1200ms, 100ms); // 5+
    pbf_move_left_joystick(context, {0, +1}, 800ms, 100ms); // 2+

    {
        ShortDialogWatcher repel_dialog(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 9600ms, 100ms); // 48+
            },
            {{repel_dialog}}
        );
        if (ret == 0){
            stream.log("go_through_route_216: Reactivating repel.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 3000ms);
        } else if (ret < 0){
                stream.log("go_through_route_216: Didn't detect repel dialog box!", COLOR_RED);
                return false;
        }
    }
    
    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    {
        MarkDetector ace_maria_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 3200ms, 100ms); // 16
            },
            {{ace_maria_fight}}
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
                stream.log("go_through_route_216_ace_maria: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "route_216_ace_maria");
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

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 7500ms, 100ms); // 25++ (snow)
            },
            {{skier_edward_fight}}
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
                stream.log("go_through_route_216_skier_edward: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "route_216_skier_edward");
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("go_through_route_216: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_route_216: entered chalet", COLOR_GREEN);
    };

    return true;
}

static bool heal_small_house_route_216(
    VideoStream& stream,
    ProControllerContext& context
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("heal_small_house_route_216: taking a quick nap...");

    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 2);

    {
        ShortDialogWatcher sleep_in_a_bed(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {+1, 0}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{sleep_in_a_bed}}
        );
        if (ret == 0){
            stream.log("heal_small_house_route_216_sleep_in_a_bed: taking a nap...", COLOR_GREEN);

            const ImageFloatBox box{0.670000, 0.600000, 0.100000, 0.150000};
            SelectionArrowFinder arrow(stream.overlay(), box, COLOR_GREEN);

            int ret = run_until<ProControllerContext>(
                stream, context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 10000ms);
                },
                {{arrow}}
            );
            if (ret != 0){
                stream.log("heal_small_house_route_216_sleep_in_a_bed: selection arrow not found.", COLOR_RED);
                return false;
            }

            pbf_press_button(context, BUTTON_A, 80ms, 300ms);
            pbf_mash_button(context, BUTTON_B, 6000ms);

        } else if (ret < 0){
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, -1}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("heal_small_house_route_216: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("heal_small_house_route_216: exited chalet", COLOR_GREEN);
    };

    return true;
}

static bool continue_through_route_216(
    VideoStream& stream,
    ProControllerContext& context
){
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 11000ms, 100ms); // 55+
            },
            {{repel_dialog}}
        );
        if (ret == 0){
            stream.log("continue_through_route_216: Not reactivating repel.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_B, 3000ms);
        } else if (ret < 0){
                stream.log("continue_through_route_216: Didn't detect repel dialog box!", COLOR_RED);
                return false;
        }
    }

    pbf_move_left_joystick(context, {0, +1}, 2000ms, 100ms); // 10++ (Snow)
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1); // Snow
    pbf_move_left_joystick(context, {0, +1}, 1200ms, 100ms); // 4++ (Snow)
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 800ms, 1); // Snow

    {
        MarkDetector ace_dalton_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 7500ms, 100ms); // 25++ (snow)
            },
            {{ace_dalton_fight}}
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
                stream.log("continue_through_route_216_ace_dalton: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "route_217_ace_dalton");
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {0, -1}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{pick_up_TM100}}
        );
        if (ret == 0){
            stream.log("continue_through_route_216: Picking up TM100.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 10000ms);
        } else if (ret < 0){
                stream.log("continue_through_route_216: Didn't detect TM100 dialog box!", COLOR_RED);
                return false;
        }
    }

    pbf_move_left_joystick(context, {-1, 0}, 2000ms, 100ms); // 7++ (snow)

    {
        MarkDetector skier_lexie_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 6000ms, 100ms); // 20++ (snow)
            },
            {{skier_lexie_fight}}
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
                stream.log("continue_through_route_216_skier_lexie: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "route_217_skier_lexie");
                context.wait_for_all_requests();
                pbf_wait(context, 3000ms);
                move_back_after_unexpected_battle(stream, context, "route_217_skier_lexie");
            }
        }
    }

    context.wait_for_all_requests();
    
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 800ms, 2); // Snow
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 800ms, 3); // Snow
    pbf_move_left_joystick(context, {-1, 0}, 1000ms, 100ms); // 2++ (snow)
    pbf_move_left_joystick(context, {0, +1}, 2700ms, 100ms); // 9++ (snow)
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 800ms, 2); // Snow
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 800ms, 1); // Snow

    {
        MarkDetector blackbelt_luke_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 5400ms, 100ms); // 21++ (snow)
            },
            {{blackbelt_luke_fight}}
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
                stream.log("continue_through_route_216_blackbelt_luke: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "route_217_blackbelt_luke");
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

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
            },
            {{ace_olivia_fight}}
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
                stream.log("continue_through_route_216_ace_olivia: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "route_217_ace_olivia");
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
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 800ms, 2); // Snow
    pbf_move_left_joystick(context, {+1, 0}, 2400ms, 100ms); // 8++ (snow)
    pbf_move_left_joystick(context, {0, +1}, 2400ms, 100ms); // 8++ (snow)
    pbf_move_left_joystick(context, {+1, 0}, 2700ms, 100ms); // 9++ (snow)

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

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
            stream.log("continue_through_route_216: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("continue_through_route_216: entered Veilstone Pokemon Center", COLOR_GREEN);
    }

    return true;
}

static bool heal_and_exit_veilstone_pokemon_center(
    VideoStream& stream,
    ProControllerContext& context
){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("heal_and_exit_veilstone_pokemon_center: healing and exiting...");
    heal_pokemon(stream, context, "Veilstone city");
    return true;
}


static bool go_to_house_south_veilstone(
    VideoStream& stream,
    ProControllerContext& context
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_to_house_south_veilstone: heading South...");

    pbf_move_left_joystick(context, {0, -1}, 600ms, 100ms); // 2+

    {
        ShortDialogWatcher repel_dialog(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 4000ms, 100ms); // 20+
                pbf_move_left_joystick(context, {+1, 0}, 4000ms, 100ms); // 20+
            },
            {{repel_dialog}}
        );
        if (ret == 0){
            stream.log("go_to_house_south_veilstone: Not reactivating repel.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_B, 3000ms);
        } else if (ret < 0){
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
    ProControllerContext& context
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

static bool walk_through_route_214(
    VideoStream& stream,
    ProControllerContext& context
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("walk_through_route_214: going through route 214...");

    pbf_move_left_joystick(context, {+1, 0}, 600ms, 100ms); // 2+
    pbf_move_left_joystick(context, {0, -1}, 3800ms, 100ms); // 19+
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 8);

    activate_repel(stream, context);

    pbf_move_left_joystick(context, {0, -1}, 2600ms, 100ms); // 13+

    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1, true);
    pbf_move_left_joystick(context, {0, -1}, 1200ms, 100ms); // 6+
    pbf_move_left_joystick(context, {-1, 0}, 400ms, 100ms); // 2
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 11000ms, 100ms); // 55+
            },
            {{repel_dialog}}
        );
        if (ret == 0){
            stream.log("walk_through_route_214: Reactivating repel.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 3000ms);
        } else if (ret < 0){
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("walk_through_route_214: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("walk_through_route_214: entered Valor Lakefront", COLOR_GREEN);
    }

    return true;
}

static bool go_through_lake_valor(
    VideoStream& stream,
    ProControllerContext& context
){
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

    // @GumGum: uncomment this when you've tested your part and made sure that it works. This is for the activate repel running.

    {
        ShortDialogWatcher repel_dialog(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 2400ms, 100ms); // 12+
                pbf_move_left_joystick(context, {+1, 0}, 2400ms, 100ms); // 12+
                pbf_move_left_joystick(context, {-1, 0}, 2400ms, 100ms); // 12+
                pbf_move_left_joystick(context, {+1, 0}, 2400ms, 100ms); // 12+
                pbf_move_left_joystick(context, {-1, 0}, 2400ms, 100ms); // 12+
                pbf_move_left_joystick(context, {+1, 0}, 2400ms, 100ms); // 12+
            },
            {{repel_dialog}}
        );
        if (ret == 0){
            stream.log("go_through_lake_valor: Not reactivating repel.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_B, 3000ms);
        } else if (ret < 0){
                stream.log("go_through_lake_valor: Didn't detect repel dialog box!", COLOR_RED);
                return false;
        }
    }
    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    pbf_move_left_joystick(context, {-1, 0}, 2400ms, 100ms); // 12+

    // @GumGum: Here comes the rest of the code



    // And this comes after having defeated the commander and having exited the cave

    fly_to(stream, context, FlyPoint::TwinleafTown);

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
            stream.log("go_through_lake_valor: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_through_lake_valor: back into own's house", COLOR_GREEN);
    }

    return true;
}

static bool heal_at_home(
    VideoStream& stream,
    ProControllerContext& context
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("heal_at_home: sleeping at mom's...");

    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 1);

    {
        ShortDialogWatcher mom_heal_dialog(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {0, +1}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{mom_heal_dialog}}
        );
        if (ret == 0){
            stream.log("heal_at_home: Sleeping...", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_B, 4000ms);
        } else if (ret < 0){
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, -1}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("heal_at_home: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("heal_at_home: leaving own's house", COLOR_GREEN);
    }

    return true;
}

static bool go_to_lake_verity(
    VideoStream& stream,
    ProControllerContext& context
){
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("go_to_lake_verity: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("go_to_lake_verity: entered Verity lakefront", COLOR_GREEN);
    }
    return true;
}

static bool commander_mars_and_snowpoint(
    VideoStream& stream,
    ProControllerContext& context
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);
    context.wait_for_all_requests();
    stream.log("commander_mars_and_snowpoint: Dealing with Commander Mars first and then heading to Snowpoint City...");

    /*{
        ShortDialogWatcher rowan_talking(COLOR_BLUE);
        int ret = wait_until(stream, context, std::chrono::seconds(30), {{talk_to_clint}});
        if (ret < 0){
            stream.log("commander_mars_and_snowpoint_rowan_talking: dialog box not detected!", COLOR_RED);
            return false;
        } else if (ret == 0) {
            stream.log("commander_mars_and_snowpoint_rowan_talking: dialog box detected.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_B, 4000ms);
            context.wait_for_all_requests();
        }
    }*/

    pbf_move_left_joystick(context, {0, +1}, 600ms, 100ms); // 2+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 2);

    {
        MarkDetector grunt_1_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 3200ms, 100ms); // 16
            },
            {{grunt_1_fight}}
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
                stream.log("commander_mars_and_snowpoint_grunt_1: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "verity_lakefront_grunt_1");
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
                pbf_mash_button(context, BUTTON_A, 200000ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("commander_mars_and_snowpoint_grunt_4: black screen not detected!", COLOR_RED);
            return false;
        } else if (ret == 0){
            handle_battle(stream, context, "verity_lakefront_grunt_2");
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

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 3200ms, 100ms); // 16
            },
            {{grunt_3_fight}}
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
                stream.log("commander_mars_and_snowpoint_grunt_3: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "verity_lakefront_grunt_3");
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
                pbf_mash_button(context, BUTTON_A, 200000ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("commander_mars_and_snowpoint_grunt_4: black screen not detected!", COLOR_RED);
            return false;
        } else if (ret == 0){
            handle_battle(stream, context, "verity_lakefront_grunt_4");
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 1);
    pbf_move_left_joystick(context, {+1, 0}, 800ms, 100ms); // 3+

    {
        ShortDialogWatcher commander_mars_fight(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {0, +1}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{commander_mars_fight}}
        );
        if (ret < 0){
            stream.log("commander_mars_and_snowpoint: Didn't detect Commander Mars dialog box!", COLOR_RED);
            return false;
        } else if (ret == 0){
            stream.log("commander_mars_and_snowpoint: Battling Commander Mars...", COLOR_GREEN);
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
                stream.log("commander_mars_and_snowpoint_commander_mars: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "verity_lakefront_commander_mars");
            }
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    pbf_mash_button(context, BUTTON_B, 10000ms);
    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    fly_to(stream, context, FlyPoint::SnowpointCity);

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
            stream.log("commander_mars_and_snowpoint: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("commander_mars_and_snowpoint: entered Snowpoint City Pokemon Center", COLOR_GREEN);
    }

    return true;
}

static bool heal_at_snowpoint_pokemon_center(
    VideoStream& stream,
    ProControllerContext& context
){
    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);
    context.wait_for_all_requests();
    stream.log("heal_at_snowpoint_pokemon_center: Healing at Snowpoint City Pokemon center and exiting...");

    heal_pokemon(stream, context, "Snowpoint City");

    return true;
}

static bool go_to_snowpoint_gym(
    VideoStream& stream,
    ProControllerContext& context
){
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("commander_mars_and_snowpoint: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("commander_mars_and_snowpoint: entered Snowpoint City Pokemon Center", COLOR_GREEN);
    }

    return true;
}




void checkpoint_093(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!leave_gym(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "leave_gym: transition not detected.", env.console);
            }
        }
    );
}

void checkpoint_094(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_to_the_library(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_to_the_library: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_095(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!drama_at_the_library(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "drama_at_the_library: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_096(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!talk_fly_and_enter_mount_coronet(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "talk_fly_and_enter_mount_coronet: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_097(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_through_mount_coronet(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_through_mount_coronet: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_098(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_through_route_216(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_through_route_216: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_099(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!heal_small_house_route_216(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "heal_small_house_route_216: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_100(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!continue_through_route_216(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "continue_through_route_216: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_101(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!heal_and_exit_veilstone_pokemon_center(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "heal_and_exit_veilstone_pokemon_center: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_102(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_to_house_south_veilstone(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_to_house_south_veilstone: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_103(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_through_house_south_veilstone(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_through_house_south_veilstone: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_104(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!walk_through_route_214(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "walk_through_route_214: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_105(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_through_lake_valor(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_through_lake_valor: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_106(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!heal_at_home(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "heal_at_home: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_107(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_to_lake_verity(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_to_lake_verity: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_108(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!commander_mars_and_snowpoint(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "commander_mars_and_snowpoint: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_109(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!heal_at_snowpoint_pokemon_center(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "heal_at_snowpoint_pokemon_center: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_110(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_to_snowpoint_gym(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_to_snowpoint_gym: transition not detected.", env.console);
            }
        },
        false
    );
}


}
}
}
