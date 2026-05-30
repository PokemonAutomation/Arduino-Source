/*  BDSP AutoStory - Segment 04
 *  From after acquiring the third badge until after acquiring the fourth badge.
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
#include "PokemonBDSP_AutoStory_Segment_04.h"

using namespace std::chrono_literals;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


std::string AutoStory_Segment_04::name()       const{ return "04: Third Badge to Fourth Badge"; }
std::string AutoStory_Segment_04::start_text() const{ return "Start: Third Gym Badge acquired (Hearthome City)."; }
std::string AutoStory_Segment_04::end_text()   const{ return "End: Fourth Gym Badge acquired (Pastoria City)."; }


void AutoStory_Segment_04::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_050().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_051().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_052().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_053().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_054().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_055().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_056().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_057().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_058().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_059().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_060().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_061().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_062().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_063().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_064().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_065().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_066().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}


std::string AutoStory_Checkpoint_050::name()       const{ return "004.01 - leave_gym"; }
std::string AutoStory_Checkpoint_050::start_text() const{ return "Inside Hearthome City gym after third badge."; }
std::string AutoStory_Checkpoint_050::end_text()   const{ return "Exited Hearthome City gym."; }
void AutoStory_Checkpoint_050::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_050(env, context, options, stats);
}

std::string AutoStory_Checkpoint_051::name()       const{ return "004.02 - go_to_veilstone_pokemon_center"; }
std::string AutoStory_Checkpoint_051::start_text() const{ return "Outside Hearthome City gym."; }
std::string AutoStory_Checkpoint_051::end_text()   const{ return "Entered Veilstone City Pokemon Center."; }
void AutoStory_Checkpoint_051::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_051(env, context, options, stats);
}

std::string AutoStory_Checkpoint_052::name()       const{ return "004.03 - heal_at_pokemon_center (Veilstone)"; }
std::string AutoStory_Checkpoint_052::start_text() const{ return "Inside Veilstone City Pokemon Center."; }
std::string AutoStory_Checkpoint_052::end_text()   const{ return "Healed and exited Veilstone City Pokemon Center."; }
void AutoStory_Checkpoint_052::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_052(env, context, options, stats);
}

std::string AutoStory_Checkpoint_053::name()       const{ return "004.04 - go_to_galactic_warehouse"; }
std::string AutoStory_Checkpoint_053::start_text() const{ return "Outside Veilstone City Pokemon Center."; }
std::string AutoStory_Checkpoint_053::end_text()   const{ return "Reached Galactic Warehouse, battle started."; }
void AutoStory_Checkpoint_053::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_053(env, context, options, stats);
}

std::string AutoStory_Checkpoint_054::name()       const{ return "004.05 - handle_battle (veilstone_galactic_grunts_1)"; }
std::string AutoStory_Checkpoint_054::start_text() const{ return "In battle against Galactic Grunt at Veilstone."; }
std::string AutoStory_Checkpoint_054::end_text()   const{ return "Defeated Galactic Grunt at Veilstone."; }
void AutoStory_Checkpoint_054::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_054(env, context, options, stats);
}

std::string AutoStory_Checkpoint_055::name()       const{ return "004.06 - enter_galactic_warehouse"; }
std::string AutoStory_Checkpoint_055::start_text() const{ return "Outside Galactic Warehouse."; }
std::string AutoStory_Checkpoint_055::end_text()   const{ return "Entered Galactic Warehouse."; }
void AutoStory_Checkpoint_055::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_055(env, context, options, stats);
}

std::string AutoStory_Checkpoint_056::name()       const{ return "004.07 - exit_galactic_warehouse"; }
std::string AutoStory_Checkpoint_056::start_text() const{ return "Inside Galactic Warehouse."; }
std::string AutoStory_Checkpoint_056::end_text()   const{ return "Exited Galactic Warehouse with TM Fly."; }
void AutoStory_Checkpoint_056::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_056(env, context, options, stats);
}

std::string AutoStory_Checkpoint_057::name()       const{ return "004.08 - go_to_route_214"; }
std::string AutoStory_Checkpoint_057::start_text() const{ return "Outside Galactic Warehouse."; }
std::string AutoStory_Checkpoint_057::end_text()   const{ return "Arrived at Route 214, battle started."; }
void AutoStory_Checkpoint_057::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_057(env, context, options, stats);
}

std::string AutoStory_Checkpoint_058::name()       const{ return "004.09 - handle_battle (route_214_psychic_abigail)"; }
std::string AutoStory_Checkpoint_058::start_text() const{ return "In battle against Psychic Abigail on Route 214."; }
std::string AutoStory_Checkpoint_058::end_text()   const{ return "Defeated Psychic Abigail on Route 214."; }
void AutoStory_Checkpoint_058::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_058(env, context, options, stats);
}

std::string AutoStory_Checkpoint_059::name()       const{ return "004.10 - walk_through_route_214"; }
std::string AutoStory_Checkpoint_059::start_text() const{ return "On Route 214 after Psychic Abigail."; }
std::string AutoStory_Checkpoint_059::end_text()   const{ return "Walked through Route 214."; }
void AutoStory_Checkpoint_059::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_059(env, context, options, stats);
}

std::string AutoStory_Checkpoint_060::name()       const{ return "004.11 - pass_by_valor_lakefront"; }
std::string AutoStory_Checkpoint_060::start_text() const{ return "End of Route 214."; }
std::string AutoStory_Checkpoint_060::end_text()   const{ return "Passed Valor Lakefront, entered building."; }
void AutoStory_Checkpoint_060::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_060(env, context, options, stats);
}

std::string AutoStory_Checkpoint_061::name()       const{ return "004.12 - go_through_building_route_213"; }
std::string AutoStory_Checkpoint_061::start_text() const{ return "Inside building between Route 214 and Route 213."; }
std::string AutoStory_Checkpoint_061::end_text()   const{ return "Exited building onto Route 213."; }
void AutoStory_Checkpoint_061::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_061(env, context, options, stats);
}

std::string AutoStory_Checkpoint_062::name()       const{ return "004.13 - go_through_route_213"; }
std::string AutoStory_Checkpoint_062::start_text() const{ return "On Route 213."; }
std::string AutoStory_Checkpoint_062::end_text()   const{ return "Walked through Route 213, entered east Pastoria building."; }
void AutoStory_Checkpoint_062::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_062(env, context, options, stats);
}

std::string AutoStory_Checkpoint_063::name()       const{ return "004.14 - go_through_house_east_pastoria"; }
std::string AutoStory_Checkpoint_063::start_text() const{ return "Inside east Pastoria building."; }
std::string AutoStory_Checkpoint_063::end_text()   const{ return "Exited east Pastoria building into Pastoria City."; }
void AutoStory_Checkpoint_063::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_063(env, context, options, stats);
}

std::string AutoStory_Checkpoint_064::name()       const{ return "004.15 - go_through_pastoria"; }
std::string AutoStory_Checkpoint_064::start_text() const{ return "In Pastoria City."; }
std::string AutoStory_Checkpoint_064::end_text()   const{ return "Entered Pastoria City Pokemon Center."; }
void AutoStory_Checkpoint_064::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_064(env, context, options, stats);
}

std::string AutoStory_Checkpoint_065::name()       const{ return "004.16 - heal_at_pokemon_center (Pastoria)"; }
std::string AutoStory_Checkpoint_065::start_text() const{ return "Inside Pastoria City Pokemon Center."; }
std::string AutoStory_Checkpoint_065::end_text()   const{ return "Healed and exited Pastoria City Pokemon Center."; }
void AutoStory_Checkpoint_065::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_065(env, context, options, stats);
}

std::string AutoStory_Checkpoint_066::name()       const{ return "004.17 - go_to_pastoria_gym"; }
std::string AutoStory_Checkpoint_066::start_text() const{ return "Outside Pastoria City Pokemon Center."; }
std::string AutoStory_Checkpoint_066::end_text()   const{ return "Fourth Gym Badge acquired (Pastoria City)."; }
void AutoStory_Checkpoint_066::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_066(env, context, options, stats);
}

static void move_back_after_unexpected_battle(
    VideoStream& stream,
    ProControllerContext& context,
    const std::string& trainerid
){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    if (trainerid == "route_213_tuber_chelsea") {

        pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms); // 4+
        pbf_move_left_joystick(context, {0, +1}, 1000ms, 100ms); // 4+
        pbf_move_left_joystick(context, {0, -1}, 800ms, 100ms); // 5
    } else if (trainerid == "route_213_tuber_jared") {
        // Just need to wait, nothing else
    } else if (trainerid == "route_213_beauty_cindy") {
        pbf_move_left_joystick(context, {0, +1}, 1400ms, 100ms); // 7+
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
        trainerid == "veilstone_galactic_grunts_1" or
        trainerid == "route_213_tuber_chelsea" or
        trainerid == "route_213_tuber_jared" or
        trainerid == "route_213_beauty_cindy"
    ) {
        // Select Razor Leaf
        pbf_press_dpad(context, DPAD_UP, 280ms, 200ms);
    } else if (
        trainerid == "route_214_collector_brady" or
        trainerid == "route_214_collector_douglas"
    ){
        // Select Bulldoze
        pbf_press_dpad(context, DPAD_DOWN, 280ms, 200ms);
    } else if (
        trainerid == "route_214_psychic_abigail" or
        trainerid == "route_214_psychic_mitchell"
    ){
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
    if (
        trainerid == "route_213_tuber_chelsea" or
        trainerid == "route_213_tuber_jared" or
        trainerid == "route_213_beauty_cindy"
    ){
        move_back_after_unexpected_battle(stream, context, trainerid);
    }
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
    stream.log("leave_gym: walking out of gym");

    pbf_move_left_joystick(context, {0, -1}, 1200ms, 100ms); // 2
    pbf_move_left_joystick(context, {+1, 0}, 300ms, 100ms); // 2

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
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, -1}, 10000ms, 0ms);
        },
        {{black_screen}}
    );
    if (ret < 0){
        stream.log("leave_gym: black screen not detected!", COLOR_RED);
        return false;
    }
    stream.log("leave_gym: transition confirmed.");
    return true;
}

static bool go_to_veilstone_pokemon_center(
    VideoStream& stream,
    ProControllerContext& context
){
    DpadState dpad;
    stream.log("Walking towards the pokemon center");
    pbf_wait(context, 4000ms);
    wait_for_dialogue(stream, context, "Dawn");
    mash_until_dialogue_ends(stream, context, BUTTON_B);
    context.wait_for_all_requests();
    pbf_wait(context, 4000ms);
    
    pbf_move_left_joystick(context, {0, -1}, 3000ms, 100ms); // 21
    pbf_move_left_joystick(context, {+1, 0}, 2000ms, 100ms); // 14
    pbf_move_left_joystick(context, {0, +1}, 600ms, 100ms); // 4
    pbf_move_left_joystick(context, {+1, 0}, 3400ms, 100ms); // 25
    pbf_move_left_joystick(context, {0, +1}, 1600ms, 100ms); // 11
    pbf_move_left_joystick(context, {-1, 0}, 1600ms, 100ms); // 12
    pbf_move_left_joystick(context, {0, +1}, 800ms, 100ms); // 5
    pbf_move_left_joystick(context, {+1, 0}, 1400ms, 100ms); // 8+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 2, true);
    context.wait_for_all_requests();

    /* Entering the Pokemon Center */
    BlackScreenOverWatcher black_screen1(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
        },
        {{black_screen1}}
    );
    if (ret < 0){
        stream.log("Didn't enter pokemon center!", COLOR_RED);
        return false;
    }

    stream.log("Entered pokemon center successfully!");

    stream.log("go_to_veilstone_pokemon_center: transition confirmed.");
    return true;

}

static bool heal_at_pokemon_center(
    VideoStream& stream,
    ProControllerContext& context
){
    return heal_pokemon(stream, context, "Veilstone");
}

static bool go_to_galactic_warehouse(
    VideoStream& stream,
    ProControllerContext& context
){
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {0, -1}, 800ms, 100ms); // 3+
    pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms); // 6
    pbf_move_left_joystick(context, {0, -1}, 1400ms, 100ms); // 6+
    pbf_move_left_joystick(context, {-1, 0}, 1000ms, 100ms); // 7
    pbf_move_left_joystick(context, {0, +1}, 2400ms, 100ms); // 9+
    pbf_move_left_joystick(context, {-1, 0}, 2800ms, 100ms); // 12+
    pbf_move_left_joystick(context, {0, +1}, 4000ms, 100ms); // 15+
    pbf_move_left_joystick(context, {+1, 0}, 1400ms, 100ms); // 5+
    pbf_press_dpad(context, DPAD_DOWN, 400ms, 200ms);

    context.wait_for_all_requests();
    BlackScreenOverWatcher battle_black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_A, 12000ms);
        },
        {{battle_black_screen}}
    );
    if (ret < 0){
        stream.log("go_to_galactic_warehouse: battle black screen not detected (2)", COLOR_RED);
        return false;
    }
    stream.log("go_to_galactic_warehouse: battle transition confirmed.");
    return true;
}

static bool enter_galactic_warehouse(
    VideoStream& stream,
    ProControllerContext& context
){
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
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, +1}, 10000ms, 100ms);
        },
        {{black_screen_1}}
    );
    if (ret < 0){
        stream.log("Entering Galactic warehouse black screen not detected!", COLOR_RED);
        return false;
    }
    stream.log("enter_galactic_warehouse: transition confirmed.");
    return true;
}

static bool exit_galactic_warehouse(
    VideoStream& stream,
    ProControllerContext& context
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {+1, 0}, 1600ms, 100ms); // 7+
    pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms); // 2+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 2, true);
    pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms); // 2+
    context.wait_for_all_requests();
    // Get fly

    ShortDialogWatcher getting_fly_TM(COLOR_BLUE);
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_A, 2000ms);
        },
        {{getting_fly_TM}}
    );
    if (ret < 0) {
        stream.log("enter_galactic_warehouse_getting_fly_TM: dialog box not detected!", COLOR_RED);
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
    ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, -1}, 10000ms, 100ms);
        },
        {{black_screen_2}}
    );
    if (ret < 0){
        stream.log("enter_galactic_warehouse: black screen not detected!", COLOR_RED);
        return false;
    }
    stream.log("exit_galactic_warehouse: transition confirmed.");
    return true;
}

static bool go_to_route_214(
    VideoStream& stream,
    ProControllerContext& context
){
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
    stream.log("Entered house");
    pbf_wait(context, 1500ms);

    context.wait_for_all_requests();
    // Add a black screen detector here when entering the connecting house
    BlackScreenOverWatcher black_screen_2(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, -1}, 10000ms, 100ms);
        },
        {{black_screen_2}}
    );
    if (ret < 0){
        stream.log("go_to_route_214_exit_house: black screen not detected!", COLOR_RED);
        return false;
    }
    stream.log("Exited house");
    pbf_wait(context, 1500ms);
    
    pbf_move_left_joystick(context, {0, -1}, 6000ms, 100ms);

    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_A, 200000ms);
        },
        {{black_screen}}
    );
    if (ret < 0){
        stream.log("go_to_route_214: black screen not detected!", COLOR_RED);
        return false;
    }
    stream.log("go_to_route_214: transition confirmed.");
    return true;
}

static bool walk_through_route_214(
    VideoStream& stream,
    ProControllerContext& context
){
    context.wait_for_all_requests();
    stream.log("walk_through_route_214: Continuing on the first part of route 214");
    pbf_wait(context, 3000ms);
    DpadState dpad;

    pbf_move_left_joystick(context, {0, -1}, 600ms, 100ms); // 4
    pbf_move_left_joystick(context, {+1, 0}, 3000ms, 100ms); // 13+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 3, true);

    activate_repel(stream, context);

    {
        MarkDetector battle_dialogue_1(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, -1}, 30000ms, 100ms); // 13+
            },
            {{battle_dialogue_1}}
        );
        if (ret == 0){
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
                stream.log("walk_through_route_214_collector_brady: black screen not detected!", COLOR_RED);
                return false;
            };
            handle_battle(stream, context, "route_214_collector_brady");
        }
    }
    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1, true);
    pbf_move_left_joystick(context, {0, -1}, 1200ms, 100ms); // 6+
    pbf_move_left_joystick(context, {-1, 0}, 400ms, 100ms); // 2
    pbf_move_left_joystick(context, {0, -1}, 1000ms, 100ms); // 5+

    pbf_move_left_joystick(context, {+1, 0}, 1600ms, 100ms); // 8+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1, true);
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 6, true);

    {
        MarkDetector battle_dialogue_2(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_wait(context, 30000ms);
            },
            {{battle_dialogue_2}}
        );
        if (ret == 0){
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
                stream.log("walk_through_route_214_psychic_mitchell: black screen not detected!", COLOR_RED);
                return false;
            };
            handle_battle(stream, context, "route_214_psychic_mitchell");
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

static bool pass_by_valor_lakefront(
    VideoStream& stream,
    ProControllerContext& context
){
    context.wait_for_all_requests();
    stream.log("Finishing going through route 214");
    pbf_wait(context, 1000ms);
    DpadState dpad;

    {
        ShortDialogWatcher repel_ran_out(COLOR_BLUE);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                DpadState dpad;
                dpad.last_dir = DPAD_DOWN;
                for (int c = 0; c < 20; c++){
                    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 1, true);
                    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 1, true);
                }
            },
            {{repel_ran_out}}
        );
        if (ret == 0){
            pbf_mash_button(context, BUTTON_A, 2000ms);
        };
    }
    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, -1}, 1000ms, 100ms); // 20+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 6, true);
    
    pbf_move_left_joystick(context, {0, -1}, 8000ms, 100ms); // 41+
    pbf_move_left_joystick(context, {-1, 0}, 600ms, 100ms); // 4
    pbf_move_left_joystick(context, {0, -1}, 800ms, 100ms); // 5
    pbf_move_left_joystick(context, {-1, 0}, 2200ms, 100ms); // 10+
    pbf_move_left_joystick(context, {0, -1}, 800ms, 100ms); // 4+
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 4, true);
    context.wait_for_all_requests();

    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, -1}, 30000ms, 100ms);
        },
        {{black_screen}}
    );
    if (ret < 0){
        stream.log("walk_through_route_214: Didn't enter building!", COLOR_RED);
        return false;
    }

    return true;
}

static bool go_through_building_route_213(
    VideoStream& stream,
    ProControllerContext& context
){
    stream.log("Going through building between routes 214 and 213");
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    ShortDialogWatcher repel_ran_out(COLOR_CYAN);
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, -1}, 30000ms, 100ms);
        },
        {{repel_ran_out}}
    );
    if (ret == 0){
        stream.log("go_through_building_route_214: Not reapplying repel.", COLOR_RED);
        pbf_mash_button(context, BUTTON_B, 2000ms);
    }

    context.wait_for_all_requests();

    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, -1}, 30000ms, 100ms);
        },
        {{black_screen}}
    );
    if (ret < 0){
        stream.log("go_through_building_route_214: Didn't exit building!", COLOR_RED);
        return false;
    }
    stream.log("go_through_building_route_214: transition confirmed.");

    return true;
}

static bool go_through_route_213(
    VideoStream& stream,
    ProControllerContext& context
){
    context.wait_for_all_requests();
    stream.log("Going through route 213");
    pbf_wait(context, 2000ms);
    DpadState dpad;

    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 4, true);
    
    MarkDetector tuber_chelsea_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {-1, 0}, 1000ms, 100ms); // 4+
            pbf_move_left_joystick(context, {0, -1}, 800ms, 100ms); // 5
            context.wait_for_all_requests();
            pbf_wait(context, 2000ms);
        },
        {{tuber_chelsea_fight}}
    );
    if (ret == 0){
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
            stream.log("go_through_route_213_tuber_chelsea: Didn't detect battle starting!", COLOR_RED);
            return false;
        } else if (ret == 0) {
            stream.log("go_through_route_213_tuber_chelsea: Starting battle.");
            handle_battle(stream, context, "route_213_tuber_chelsea");
            context.wait_for_all_requests();
            pbf_wait(context, 3000ms);
        }
    }
    context.wait_for_all_requests();
    
    MarkDetector tuber_jared_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
    ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, -1}, 1000ms, 100ms); // 4+
            pbf_move_left_joystick(context, {-1, 0}, 600ms, 100ms); // 3
            context.wait_for_all_requests();
            pbf_wait(context, 1000ms);
        },
        {{tuber_jared_fight}}
    );
    if (ret == 0){
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
            stream.log("go_through_route_213_tuber_jared: Didn't detect battle starting!", COLOR_RED);
            return false;
        } else if (ret == 0) {
            stream.log("go_through_route_213_tuber_jared: Starting battle.");
            handle_battle(stream, context, "route_213_tuber_jared");
            context.wait_for_all_requests();
            pbf_wait(context, 3000ms);
        }
    }

    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {-1, 0}, 3600ms, 100ms); // 20+
    pbf_move_left_joystick(context, {0, -1}, 400ms, 100ms); // 3
    pbf_move_left_joystick(context, {-1, 0}, 2000ms, 100ms); // 13+

    MarkDetector beauty_cindy_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
    ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, +1}, 5000ms, 100ms); // 15+
        },
        {{beauty_cindy_fight}}
    );
    if (ret == 0){
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, 200000ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("go_through_route_213_beauty_cindy: Didn't detect battle starting!", COLOR_RED);
            return false;
        } else if (ret == 0) {
            context.wait_for_all_requests();
            stream.log("go_through_route_213_beauty_cindy: Starting battle.");
            handle_battle(stream, context, "route_213_beauty_cindy");
            context.wait_for_all_requests();
            pbf_wait(context, 3000ms);
        }
    }

    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {+1, 0}, 800ms, 100ms); // 5
    pbf_move_left_joystick(context, {0, +1}, 1000ms, 100ms); // 8

    activate_repel(stream, context);

    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {-1, 0}, 100000ms, 100ms);
        },
        {{black_screen}}
    );
    if (ret < 0){
        stream.log("go_through_route_213: Didn't enter building!", COLOR_RED);
        return false;
    }
    stream.log("go_to_route_213: transition confirmed.");

    return true;
}

static bool go_through_house_east_pastoria(
    VideoStream& stream,
    ProControllerContext& context
){
    stream.log("Going through the building at the entrance of Pastoria city");
    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {-1, 0}, 100000ms, 100ms);
        },
        {{black_screen}}
    );
    if (ret < 0){
        stream.log("go_through_house_east_pastoria: Didn't exit building!", COLOR_RED);
        return false;
    }
    stream.log("go_through_house_east_pastoria: transition confirmed.");

    return true;
}

static bool go_through_pastoria(
    VideoStream& stream,
    ProControllerContext& context
){
    stream.log("Going through Pastoria city and to the Pokemon center");
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms); // 2+
    pbf_move_left_joystick(context, {-1, 0}, 4200ms, 100ms); // 23+
    pbf_move_left_joystick(context, {0, -1}, 500ms, 100ms); // 3
    pbf_move_left_joystick(context, {-1, 0}, 600ms, 100ms); // 4
    pbf_move_left_joystick(context, {0, -1}, 600ms, 100ms); // 4

    ShortDialogWatcher repel_dialog(COLOR_RED);
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {-1, 0}, 3400ms, 100ms); // 19
            pbf_move_left_joystick(context, {+1, 0}, 3400ms, 100ms); // 19
            pbf_move_left_joystick(context, {-1, 0}, 3400ms, 100ms); // 19
            pbf_move_left_joystick(context, {+1, 0}, 3400ms, 100ms); // 19
        },
        {{repel_dialog}}
    );
    if (ret == 0){
        pbf_mash_button(context, BUTTON_B, 3000ms);
    } else if (ret < 0){
            stream.log("go_through_pastoria: Didn't detect repel dialog box!", COLOR_RED);
            return false;
    }
    
    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {-1, 0}, 3400ms, 100ms); // 19

    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 8, true);

    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, +1}, 100000ms, 100ms);
        },
        {{black_screen}}
    );
    if (ret < 0){
        stream.log("go_through_pastoria: Didn't enter Pokemon center!", COLOR_RED);
        return false;
    }
    stream.log("go_through_pastoria: transition confirmed.");

    return true;
}

static bool go_to_pastoria_gym(
    VideoStream& stream,
    ProControllerContext& context
){
    stream.log("Going to Pastoria gym");
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {0, -1}, 800ms, 100ms); // 4+
    pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms); // 5
    pbf_move_left_joystick(context, {0, -1}, 1200ms, 100ms); // 8
    pbf_move_left_joystick(context, {-1, 0}, 2000ms, 100ms); // 10+
    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 3, true);

    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, +1}, 100000ms, 100ms);
        },
        {{black_screen}}
    );
    if (ret < 0){
        stream.log("go_to_pastoria_gym: Didn't enter building!", COLOR_RED);
        return false;
    }
    stream.log("go_to_pastoria_gym: transition confirmed.");
    return true;
}

void checkpoint_050(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!leave_gym(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "leave_gym: transition not detected.", env.console);
            }
        }
    );
}

void checkpoint_051(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_to_veilstone_pokemon_center(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_to_veilstone_pokemon_center: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_052(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!heal_at_pokemon_center(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "heal_at_pokemon_center: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_053(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_to_galactic_warehouse(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_to_galactic_warehouse: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_054(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!handle_battle(env.console, context, "veilstone_galactic_grunts_1")){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "handle_battle_veilstone_galactic_grunts_1: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_055(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!enter_galactic_warehouse(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "enter_galactic_warehouse: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_056(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!exit_galactic_warehouse(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "exit_galactic_warehouse: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_057(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_to_route_214(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_to_route_214: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_058(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!handle_battle(env.console, context, "route_214_psychic_abigail")){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "handle_battle_route_214_psychic_abigail: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_059(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!walk_through_route_214(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "walk_through_route_214: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_060(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!pass_by_valor_lakefront(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "pass_by_valor_lakefront: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_061(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_through_building_route_213(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_through_building_route_213: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_062(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_through_route_213(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_through_route_213: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_063(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_through_house_east_pastoria(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_through_house_east_pastoria: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_064(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_through_pastoria(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_through_pastoria: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_065(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!heal_at_pokemon_center(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "heal_at_pokemon_center: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_066(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_to_pastoria_gym(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_to_pastoria_gym: transition not detected.", env.console);
            }
        },
        false
    );
}


}
}
}