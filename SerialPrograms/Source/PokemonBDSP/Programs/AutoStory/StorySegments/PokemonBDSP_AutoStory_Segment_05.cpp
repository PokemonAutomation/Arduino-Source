/*  BDSP AutoStory - Segment 05
 *  From after acquiring the fourth badge until after acquiring the fifth badge.
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
#include "PokemonBDSP_AutoStory_Segment_05.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


std::string AutoStory_Segment_05::name()       const{ return "05: Fourth Badge to Fifth Badge"; }
std::string AutoStory_Segment_05::start_text() const{ return "Start: Fourth Gym Badge acquired (Pastoria City)."; }
std::string AutoStory_Segment_05::end_text()   const{ return "End: Fifth Gym Badge acquired (Veilstone City)."; }

void AutoStory_Segment_05::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_0501().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0502().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0503().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0504().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0505().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0506().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0507().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0508().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0509().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0510().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0511().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0512().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0513().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0514().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0515().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0516().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0517().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}


std::string AutoStory_Checkpoint_0501::name()       const{ return "05.01 - go_to_pastoria_pokemon_center"; }
std::string AutoStory_Checkpoint_0501::start_text() const{ return "Fourth Gym Badge acquired (Pastoria City)."; }
std::string AutoStory_Checkpoint_0501::end_text()   const{ return "Entered Pastoria City Pokemon Center."; }
void AutoStory_Checkpoint_0501::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_0501(env, context, options, stats);
}

std::string AutoStory_Checkpoint_0502::name()       const{ return "05.02 - exit_pastoria_pokemon_center"; }
std::string AutoStory_Checkpoint_0502::start_text() const{ return "Inside Pastoria City Pokemon Center."; }
std::string AutoStory_Checkpoint_0502::end_text()   const{ return "Healed and exited Pastoria City Pokemon Center."; }
void AutoStory_Checkpoint_0502::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_0502(env, context, options, stats);
}

std::string AutoStory_Checkpoint_0503::name()       const{ return "05.03 - pastoria_city_drama"; }
std::string AutoStory_Checkpoint_0503::start_text() const{ return "Outside Pastoria City Pokemon Center."; }
std::string AutoStory_Checkpoint_0503::end_text()   const{ return "Pastoria City drama complete, exited east building."; }
void AutoStory_Checkpoint_0503::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_0503(env, context, options, stats);
}

std::string AutoStory_Checkpoint_0504::name()       const{ return "05.04 - go_through_house_east_pastoria"; }
std::string AutoStory_Checkpoint_0504::start_text() const{ return "Inside east Pastoria building."; }
std::string AutoStory_Checkpoint_0504::end_text()   const{ return "Exited east Pastoria building onto Route 213."; }
void AutoStory_Checkpoint_0504::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_0504(env, context, options, stats);
}

std::string AutoStory_Checkpoint_0505::name()       const{ return "05.05 - go_through_route_213_chasing"; }
std::string AutoStory_Checkpoint_0505::start_text() const{ return "On Route 213 chasing Galactic Grunts."; }
std::string AutoStory_Checkpoint_0505::end_text()   const{ return "Entered east Route 213 building."; }
void AutoStory_Checkpoint_0505::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_0505(env, context, options, stats);
}

std::string AutoStory_Checkpoint_0506::name()       const{ return "05.06 - go_through_building_route_213"; }
std::string AutoStory_Checkpoint_0506::start_text() const{ return "Inside building on Route 213."; }
std::string AutoStory_Checkpoint_0506::end_text()   const{ return "Exited building onto Route 214."; }
void AutoStory_Checkpoint_0506::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_0506(env, context, options, stats);
}

std::string AutoStory_Checkpoint_0507::name()       const{ return "05.07 - go_through_route_214"; }
std::string AutoStory_Checkpoint_0507::start_text() const{ return "On Route 214 chasing Galactic Grunts."; }
std::string AutoStory_Checkpoint_0507::end_text()   const{ return "Flew to Solaceon Town."; }
void AutoStory_Checkpoint_0507::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_0507(env, context, options, stats);
}

std::string AutoStory_Checkpoint_0508::name()       const{ return "05.08 - go_inside_solaceon_pokemon_center"; }
std::string AutoStory_Checkpoint_0508::start_text() const{ return "In Solaceon Town."; }
std::string AutoStory_Checkpoint_0508::end_text()   const{ return "Entered Solaceon Town Pokemon Center."; }
void AutoStory_Checkpoint_0508::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_0508(env, context, options, stats);
}

std::string AutoStory_Checkpoint_0509::name()       const{ return "05.09 - heal_at_solaceon_pokemon_center"; }
std::string AutoStory_Checkpoint_0509::start_text() const{ return "Inside Solaceon Town Pokemon Center."; }
std::string AutoStory_Checkpoint_0509::end_text()   const{ return "Healed and exited Solaceon Town Pokemon Center."; }
void AutoStory_Checkpoint_0509::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_0509(env, context, options, stats);
}

std::string AutoStory_Checkpoint_0510::name()       const{ return "05.10 - go_to_the_psyduck_route_210"; }
std::string AutoStory_Checkpoint_0510::start_text() const{ return "Outside Solaceon Town Pokemon Center."; }
std::string AutoStory_Checkpoint_0510::end_text()   const{ return "Reached Celestic Town Pokemon Center via Route 210."; }
void AutoStory_Checkpoint_0510::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_0510(env, context, options, stats);
}

std::string AutoStory_Checkpoint_0511::name()       const{ return "05.11 - heal_at_celestic_pokemon_center"; }
std::string AutoStory_Checkpoint_0511::start_text() const{ return "Inside Celestic Town Pokemon Center."; }
std::string AutoStory_Checkpoint_0511::end_text()   const{ return "Healed and exited Celestic Town Pokemon Center."; }
void AutoStory_Checkpoint_0511::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_0511(env, context, options, stats);
}

std::string AutoStory_Checkpoint_0512::name()       const{ return "05.12 - celestic_town_drama"; }
std::string AutoStory_Checkpoint_0512::start_text() const{ return "In Celestic Town."; }
std::string AutoStory_Checkpoint_0512::end_text()   const{ return "Entered Celestic Town cave."; }
void AutoStory_Checkpoint_0512::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_0512(env, context, options, stats);
}

std::string AutoStory_Checkpoint_0513::name()       const{ return "05.13 - discover_the_paintings"; }
std::string AutoStory_Checkpoint_0513::start_text() const{ return "Inside Celestic Town cave."; }
std::string AutoStory_Checkpoint_0513::end_text()   const{ return "Discovered paintings and exited cave."; }
void AutoStory_Checkpoint_0513::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_0513(env, context, options, stats);
}

std::string AutoStory_Checkpoint_0514::name()       const{ return "05.14 - talk_to_cyrus_and_fly_hearthome"; }
std::string AutoStory_Checkpoint_0514::start_text() const{ return "Outside Celestic Town cave."; }
std::string AutoStory_Checkpoint_0514::end_text()   const{ return "Entered Hearthome City Pokemon Center."; }
void AutoStory_Checkpoint_0514::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_0514(env, context, options, stats);
}

std::string AutoStory_Checkpoint_0515::name()       const{ return "05.15 - heal_at_hearthome_pokemon_center"; }
std::string AutoStory_Checkpoint_0515::start_text() const{ return "Inside Hearthome City Pokemon Center."; }
std::string AutoStory_Checkpoint_0515::end_text()   const{ return "Healed and exited Hearthome City Pokemon Center."; }
void AutoStory_Checkpoint_0515::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_0515(env, context, options, stats);
}

std::string AutoStory_Checkpoint_0516::name()       const{ return "05.16 - get_into_hearthome_gym"; }
std::string AutoStory_Checkpoint_0516::start_text() const{ return "Outside Hearthome City gym."; }
std::string AutoStory_Checkpoint_0516::end_text()   const{ return "Fifth Gym Badge acquired (Hearthome City)."; }
void AutoStory_Checkpoint_0516::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_0516(env, context, options, stats);
}

std::string AutoStory_Checkpoint_0517::name()       const{ return "05.17 - Hearthome Gym Puzzle"; }
std::string AutoStory_Checkpoint_0517::start_text() const{ return "Outside Hearthome City gym."; }
std::string AutoStory_Checkpoint_0517::end_text()   const{ return "Fifth Gym Badge acquired (Hearthome City)."; }
void AutoStory_Checkpoint_0517::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_0517(env, context, options, stats);
}

static void move_back_after_unexpected_battle(
    VideoStream& stream,
    ProControllerContext& context,
    const std::string& trainerid
){
    stream.log("Starting battle with " + trainerid, COLOR_GREEN);
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    if (trainerid == "route_213_tuber_chelsea") {
        DpadState dpad;

        pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms); // 4+
        pbf_move_left_joystick(context, {0, +1}, 1000ms, 100ms); // 4+
        dpad.last_dir = DPAD_UP;
        repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 6);
    } else if (trainerid == "route_213_tuber_jared") {
        // Just need to wait, nothing else
    } else if (trainerid == "route_213_beauty_cindy") {
        pbf_move_left_joystick(context, {0, -1}, 400ms, 100ms); // 1+
    } else if (trainerid == "route_210_ace_ernest") {
        pbf_move_left_joystick(context, {-1, 0}, 3800ms, 100ms); // 19+
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
        trainerid == "pastoria_clint" or
        trainerid == "route_213_tuber_chelsea" or
        trainerid == "route_213_tuber_jared" or
        trainerid == "route_213_beauty_cindy" or
        trainerid == "route_210_veteran_brian"
    ) {
        // Select Razor leaf
        pbf_press_dpad(context, DPAD_UP, 280ms, 200ms);

    } else if (
        trainerid == "route_214_galactic_grunt" or
        trainerid == "route_210_rancher_marco" or
        trainerid == "route_210_breeder_amber" or
        trainerid == "route_210_blackbelt_adam"
    ){
        // Select Bulldoze
        pbf_press_dpad(context, DPAD_DOWN, 280ms, 200ms);
        
    } else if (
        trainerid == "route_210_ace_alyssa" or
        trainerid == "route_210_birdkeeper_brianna" or
        trainerid == "celestic_town_galactic_grunt"
    ){
        // Select Crunch
    }
    context.wait_for_all_requests();

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
    context.wait_for_all_requests();
    pbf_mash_button(context, BUTTON_B, 2000ms);
    stream.log("handle_battle_" + trainerid + ": transition confirmed.");
    return true;
}

static bool leave_gym(
    VideoStream& stream,
    ProControllerContext& context
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("leave_gym: walking out of gym");

    pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms); // 1+
    pbf_move_left_joystick(context, {+1, 0}, 600ms, 100ms); // 2+
    pbf_move_left_joystick(context, {0, +1}, 600ms, 100ms); // 2+
    pbf_move_left_joystick(context, {+1, 0}, 2000ms, 100ms); // 10+
    pbf_move_left_joystick(context, {0, -1}, 6000ms, 100ms); // 34+
    pbf_move_left_joystick(context, {-1, 0}, 1800ms, 100ms); // 9+
    pbf_move_left_joystick(context, {0, -1}, 1000ms, 100ms); // 5+
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 3, true);
    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, -1}, 100000ms, 100ms);
        },
        {{black_screen}}
    );
    if (ret < 0){
        stream.log("leave_gym: Didn't enter building!", COLOR_RED);
        return false;
    }
    stream.log("leave_gym: transition confirmed.");
    return true;
}

static bool go_to_pastoria_pokemon_center(
    VideoStream& stream,
    ProControllerContext& context
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_to_pastoria_pokemon_center: going to pokemon center");

    pbf_move_left_joystick(context, {+1, 0}, 900ms, 100ms); // 6
    pbf_move_left_joystick(context, {0, +1}, 2400ms, 100ms); // 12+
    pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms); // 3+
    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 7, true);

    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret1 = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 20000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret1 < 0){
            stream.log("go_to_pastoria_pokemon_centert: Didn't detect transition!", COLOR_RED);
            return false;
        }
        stream.log("go_to_pastoria_pokemon_center: transition confirmed.");
    return true;
}

static bool exit_pastoria_pokemon_center(
    VideoStream& stream,
    ProControllerContext& context
){
    return heal_pokemon(stream, context, "Pastoria");
}

static bool pastoria_city_drama(
    VideoStream& stream,
    ProControllerContext& context
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("pastoria_city_drama: starting the drama");

    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 2, true);
    pbf_move_left_joystick(context, {+1, 0}, 1600ms, 100ms); // 10
    pbf_move_left_joystick(context, {0, +1}, 500ms, 100ms); // 3
    pbf_move_left_joystick(context, {+1, 0}, 1000ms, 100ms); // 7
    pbf_move_left_joystick(context, {0, +1}, 1400ms, 100ms); // 7+
    pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms); // 2+

    context.wait_for_all_requests();

    {
        ShortDialogWatcher talk_to_grunt_1(COLOR_BLUE);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, 2000ms);
            },
            {{talk_to_grunt_1}}
        );
        if (ret < 0) {
            stream.log("pastoria_city_drama_talk_to_grunt_1: dialog box not detected!", COLOR_RED);
        }
        if (ret == 0){
            stream.log("pastoria_city_drama_talk_to_grunt_1: talking to grunt!");
            pbf_mash_button(context, BUTTON_B, 8000ms);
        };
    }
    

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {+1, 0}, 4800ms, 100ms); // 24+
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 2, true);
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 1, true);

    {
        ShortDialogWatcher talk_to_grunt_2(COLOR_BLUE);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, 2000ms);
            },
            {{talk_to_grunt_2}}
        );
        if (ret < 0) {
            stream.log("pastoria_city_drama_talk_to_grunt_2: dialog box not detected!", COLOR_RED);
        }
        if (ret == 0){
            stream.log("pastoria_city_drama_talk_to_grunt_2: talking to grunt!");
            
            BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            int ret1 = run_until<ProControllerContext>(
                stream, context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_B, 200000ms);
                },
                {{black_screen}}
            );
            if (ret1 < 0){
                stream.log("pastoria_city_drama_talk_to_clint: Didn't detect start of battle with Clint!", COLOR_RED);
                return false;
            }
            stream.log("pastoria_city_drama_talk_to_clint: battling Clint.");
            handle_battle(stream, context, "pastoria_clint");
            pbf_mash_button(context, BUTTON_B, 3000ms);
        };
    }
    

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);
    
    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 1, true);

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
            stream.log("pastoria_city_drama: transition not detected!", COLOR_RED);
            return false;
        }
        stream.log("pastoria_city_drama: transition detected.");
    }
    
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
            pbf_move_left_joystick(context, {+1, 0}, 100000ms, 100ms);
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

static bool go_through_route_213_chasing(
    VideoStream& stream,
    ProControllerContext& context
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("go_through_route_213_chasing: continuing the drama");

    pbf_move_left_joystick(context, {+1, 0}, 1600ms, 100ms); // 12
    activate_repel(stream, context);
    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);
    pbf_move_left_joystick(context, {+1, 0}, 3600ms, 100ms); // 18+

    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 5, true);

    MarkDetector beauty_cindy_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, -1}, 5000ms, 100ms); // 15+
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
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {-1, 0}, 600ms, 100ms); // 4
    pbf_move_left_joystick(context, {0, -1}, 1200ms, 100ms); // 9
    pbf_move_left_joystick(context, {+1, 0}, 1600ms, 100ms); // 8+
    pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms); // 2+
    pbf_move_left_joystick(context, {+1, 0}, 600ms, 100ms); // 4
    pbf_move_left_joystick(context, {0, +1}, 600ms, 100ms); // 3+
    pbf_move_left_joystick(context, {-1, 0}, 600ms, 100ms); // 3+

    dpad.last_dir = DPAD_LEFT;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 2, true);

    context.wait_for_all_requests();

    ShortDialogWatcher talk_to_grunt(COLOR_BLUE);
    ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            ssf_press_left_joystick(context, {+1, 0}, 0ms, 5000ms);
            ssf_mash1_button(context, BUTTON_A, 5000ms);
        },
        {{talk_to_grunt}}
    );
    if (ret < 0) {
        stream.log("go_through_route_213_chasing_talk_to_grunt: dialog box not detected!", COLOR_RED);
    }
    if (ret == 0){
        stream.log("go_through_route_213_chasing_talk_to_grunt: talking to grunt!");
        pbf_mash_button(context, BUTTON_B, 6000ms);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms); // 2+
    context.wait_for_all_requests();

    ShortDialogWatcher repel_dialog(COLOR_RED);
    ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {-1, 0}, 2000ms, 100ms);
            pbf_move_left_joystick(context, {+1, 0}, 2000ms, 100ms);
            pbf_move_left_joystick(context, {-1, 0}, 2000ms, 100ms);
            pbf_move_left_joystick(context, {+1, 0}, 2000ms, 100ms);
        },
        {{repel_dialog}}
    );
    if (ret == 0){
        pbf_mash_button(context, BUTTON_B, 3000ms);
    } else if (ret < 0){
            stream.log("go_through_route_213: Didn't detect repel dialog box!", COLOR_RED);
            return false;
    }

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {-1, 0}, 2000ms, 100ms);
    
    MarkDetector tuber_jared_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
    ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {+1, 0}, 3200ms, 100ms); // 17
            pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms); // 3
            context.wait_for_all_requests();
            pbf_wait(context, 2000ms);
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

    MarkDetector tuber_chelsea_fight(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
    ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            DpadState dpad;
            pbf_move_left_joystick(context, {0, +1}, 1200ms, 100ms); // 6+
            pbf_move_left_joystick(context, {-1, 0}, 600ms, 100ms); // 3+
            dpad.last_dir = DPAD_LEFT;
            repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 6, true);
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
            stream.log("go_through_route_213_tuber_chelsea: Didn't detect battle starting!");
        } else if (ret == 0) {
            stream.log("go_through_route_213_tuber_chelsea: Starting battle.");
            handle_battle(stream, context, "route_213_tuber_chelsea");
            context.wait_for_all_requests();
            pbf_wait(context, 3000ms);
        }
    }
    
    BlackScreenOverWatcher route_213_east_building(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, +1}, 20000ms, 100ms);
        },
        {{route_213_east_building}}
    );
    if (ret < 0){
        stream.log("go_through_route_213_east_building: Didn't detect entering the building!", COLOR_RED);
        return false;
    };
    
    return true;
}

static bool go_through_building_route_213(
    VideoStream& stream,
    ProControllerContext& context
){
    stream.log("Going through building between routes 214 and 213");
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, +1}, 30000ms, 100ms);
        },
        {{black_screen}}
    );
    if (ret < 0){
        stream.log("go_through_building_route_214: Didn't exit building!", COLOR_RED);
        return false;
    }
    stream.log("go_through_building_route_213: transition confirmed.");

    return true;
}

static bool go_through_route_214(
    VideoStream& stream,
    ProControllerContext& context
){
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
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            ssf_press_left_joystick(context, {+1, 0}, 0ms, 5000ms);
            ssf_mash1_button(context, BUTTON_A, 5000ms);
        },
        {{talk_to_grunt}}
    );
    if (ret < 0) {
        stream.log("go_through_route_214_talk_to_grunt: dialog box not detected!", COLOR_RED);
    }
    if (ret == 0){
        stream.log("go_through_route_214_talk_to_grunt: talking to grunt!");
        pbf_mash_button(context, BUTTON_B, 6000ms);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {0, +1}, 2200ms, 100ms); // 11+
    pbf_move_left_joystick(context, {+1, 0}, 1200ms, 100ms); // 6+
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 2, true);

    ShortDialogWatcher talk_to_grunt_2(COLOR_BLUE);
    ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            ssf_press_left_joystick(context, {0, +1}, 0ms, 5000ms);
            ssf_mash1_button(context, BUTTON_A, 5000ms);
        },
        {{talk_to_grunt_2}}
    );
    if (ret < 0) {
        stream.log("go_through_route_214_talk_to_grunt_2: dialog box not detected!", COLOR_RED);
    }
    if (ret == 0){
        stream.log("go_through_route_214_talk_to_grunt_2: talking to grunt!");
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
            stream.log("go_through_route_214_galactic_grunt: Didn't detect battle starting!", COLOR_RED);
            return false;
        } else if (ret == 0) {
            stream.log("go_through_route_214_galactic_grunt: Starting battle.");
            handle_battle(stream, context, "route_214_galactic_grunt");
            context.wait_for_all_requests();
            pbf_wait(context, 5000ms);
        }
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 1, true);

    ShortDialogWatcher talk_to_cynthia(COLOR_BLUE);
    ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            ssf_press_left_joystick(context, {0, +1}, 0ms, 10000ms);
            ssf_mash1_button(context, BUTTON_A, 10000ms);
        },
        {{talk_to_cynthia}}
    );
    if (ret < 0) {
        stream.log("go_through_route_214_talk_to_cynthia: dialog box not detected!", COLOR_RED);
    }
    if (ret == 0){
        stream.log("go_through_route_214_talk_to_cynthia: talking to Cynthia!");
        pbf_mash_button(context, BUTTON_A, 12000ms);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 4000ms);

    fly_to(stream, context, FlyPoint::SolaceonTown);

    return true;
}

static bool go_inside_solaceon_pokemon_center(
    VideoStream& stream,
    ProControllerContext& context
){
    stream.log("go_inside_solaceon_pokemon_center: going inside");
    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, +1}, 200000ms, 100ms);
        },
        {{black_screen}}
    );
    if (ret < 0){
        stream.log("go_inside_solaceon_pokemon_center: black screen not detected!", COLOR_RED);
        return false;
    } else if (ret == 0) {
        stream.log("go_inside_solaceon_pokemon_center: black screen detected!", COLOR_GREEN);
        return true;
    }
    return true;
}

static bool heal_at_solaceon_pokemon_center(
    VideoStream& stream,
    ProControllerContext& context
){
    stream.log("heal_at_solaceon_pokemon_center: healing pokemon");
    context.wait_for_all_requests();
    heal_pokemon(stream, context, "Solaceon");
    return true;
}

static bool go_to_the_psyduck_route_210(
    VideoStream& stream,
    ProControllerContext& context
){
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
    activate_repel(stream, context);

    context.wait_for_all_requests();
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);
    pbf_move_left_joystick(context, {0, +1}, 200ms, 100ms); // 1+

    {
        MarkDetector battle_dialogue(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 20000ms, 100ms);
            },
            {{battle_dialogue}}
        );
        if (ret == 0){
            context.wait_for_all_requests();
            pbf_wait(context, 2000ms);
            stream.log("go_to_the_psyduck_route_210_rancher_marco: marker detected!", COLOR_GREEN);

            BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            ret = run_until<ProControllerContext>(
                stream, context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 200000ms);
                },
                {{black_screen}}
            );
            if (ret < 0){
                stream.log("go_to_the_psyduck_route_210_rancher_marco: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "route_210_rancher_marco");
            }
        }
    }

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0, +1}, 2400ms, 100ms); // 8+ tall grass
    pbf_move_left_joystick(context, {+1, 0}, 2000ms, 100ms); // 6+ tall grass


    
    {
        MarkDetector battle_dialogue(stream.overlay(), {0.1, 0.0, 0.8, 0.9});
        ShortDialogWatcher battle_dialogue_2(COLOR_BLUE);

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                DpadState dpad;
                pbf_move_left_joystick(context, {0, +1}, 6600ms, 100ms); // 22+ tall grass

                dpad.last_dir = DPAD_UP;
                repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 4);
                repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);
                pbf_mash_button(context, BUTTON_A, 200000ms);
            },
            {{battle_dialogue}, {battle_dialogue_2}}
        );
        if (ret >= 0){
            context.wait_for_all_requests();
            pbf_wait(context, 2000ms);
            stream.log("go_to_the_psyduck_route_210_breeder_amber: marker detected!", COLOR_GREEN);

            BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            ret = run_until<ProControllerContext>(
                stream, context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 200000ms);
                },
                {{black_screen}}
            );
            if (ret < 0){
                stream.log("go_to_the_psyduck_route_210_breeder_amber: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "route_210_breeder_amber");
            }
        }
    }

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);
    pbf_move_left_joystick(context, {0, +1}, 800ms, 100ms); // 4+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 3);

    ShortDialogWatcher talk_to_psyducks(COLOR_BLUE);
    int ret_psyducks = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            ssf_press_left_joystick(context, {0, +1}, 0ms, 5000ms);
            ssf_mash1_button(context, BUTTON_A, 5000ms);
        },
        {{talk_to_psyducks}}
    );
    if (ret_psyducks < 0) {
        stream.log("go_to_the_psyduck_route_210_talk_to_psyducks: dialog box not detected!", COLOR_RED);
    }
    if (ret_psyducks == 0){
        stream.log("go_to_the_psyduck_route_210_talk_to_psyducks: talking to Psyducks!");
        pbf_mash_button(context, BUTTON_A, 28000ms);
    };

    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 1);
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 9);
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 3);
    context.wait_for_all_requests();

    activate_repel(stream, context);

    pbf_move_left_joystick(context, {0, +1}, 8000ms, 100ms); // 29+ tall grass 
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);

    {
        ShortDialogWatcher repel_dialog(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 2600ms, 100ms);
            },
            {{repel_dialog}}
        );
        if (ret == 0){
            stream.log("go_to_the_psyduck_route_210: Reactivating repel.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_A, 3000ms);
        } else if (ret < 0){
            stream.log("go_to_the_psyduck_route_210: Didn't detect repel dialog box!", COLOR_RED);
            return false;
        }
    }

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

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, +1}, 10000ms, 100ms);
            },
            {{battle_dialogue}}
        );
        if (ret < 0) {
            stream.log("go_to_the_psyduck_route_210_ace_alyssa: dialog box not detected!", COLOR_RED);
            return false;
        } else if (ret == 0){
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
                stream.log("go_to_the_psyduck_route_210_ace_alyssa: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "route_210_ace_alyssa");
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

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 3800ms, 100ms); // 19+
            },
            {{battle_dialogue}}
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
                stream.log("go_to_the_psyduck_route_210_ace_ernest: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "route_210_ace_ernest");
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

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, -1}, 20000ms, 100ms);
            },
            {{battle_dialogue}}
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
                stream.log("go_to_the_psyduck_route_210_blackbelt_adam: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "route_210_blackbelt_adam");
            }
        }
    }

    context.wait_for_all_requests();

    {
        ShortDialogWatcher repel_dialog(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, -1}, 200ms, 100ms); // 1+
                pbf_move_left_joystick(context, {+1, 0}, 200ms, 100ms); // 1+
                pbf_move_left_joystick(context, {-1, 0}, 1000ms, 100ms);
                pbf_move_left_joystick(context, {+1, 0}, 1000ms, 100ms);
            },
            {{repel_dialog}}
        );
        if (ret == 0){
            stream.log("go_to_the_psyduck_route_210: Not reactivating repel.", COLOR_GREEN);
            pbf_mash_button(context, BUTTON_B, 3000ms);
        } else if (ret < 0){
            stream.log("go_to_the_psyduck_route_210: Didn't detect repel dialog box!", COLOR_RED);
            return false;
        }
    }

    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {+1, 0}, 1000ms, 100ms);

    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 5);

    pbf_move_left_joystick(context, {0, +1}, 1400ms, 100ms); // 7+

    {
        MarkDetector battle_dialogue(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 20000ms, 100ms);
            },
            {{battle_dialogue}}
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
                stream.log("go_to_the_psyduck_route_210_birdkeeper_brianna: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "route_210_birdkeeper_brianna");
            }
        }
    }

    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {-1, 0}, 200ms, 100ms); // 1+
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 4);

    {
        MarkDetector battle_dialogue(stream.overlay(), {0.1, 0.0, 0.8, 0.9});

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 6000ms, 100ms);
            },
            {{battle_dialogue}}
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
                stream.log("go_to_the_psyduck_route_210_veteran_brian: black screen not detected!", COLOR_RED);
                return false;
            } else if (ret == 0){
                handle_battle(stream, context, "route_210_veteran_brian");
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 20000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("go_to_the_psyduck_route_210_enter_celestic_town_pokemon_center: black screen not detected!", COLOR_RED);
            return false;
        } else if (ret == 0){
            return true;
        }

    }
    return true;
}

static bool heal_at_celestic_pokemon_center(
    VideoStream& stream,
    ProControllerContext& context
){
    stream.log("heal_at_celestic_pokemon_center: healing pokemon");
    context.wait_for_all_requests();
    heal_pokemon(stream, context, "Celestic");
    return true;
}

static bool celestic_town_drama(
    VideoStream& stream,
    ProControllerContext& context
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("celestic_town_drama: continuing the drama.");

    pbf_move_left_joystick(context, {-1, 0}, 2000ms, 100ms); // 10+

    {
        ShortDialogWatcher talk_to_cynthias_grandma(COLOR_BLUE);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {0, +1}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{talk_to_cynthias_grandma}}
        );
        if (ret < 0) {
            stream.log("celestic_town_talk_to_cynthias_grandma: dialog box not detected!", COLOR_RED);
        }
        if (ret == 0){
            stream.log("celestic_town_talk_to_cynthias_grandma: talking to Cynthia's grandma!");
            pbf_mash_button(context, BUTTON_B, 4000ms);
        };
    }

    context.wait_for_all_requests();
    pbf_wait(context, 1500ms);

    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 3);
    pbf_move_left_joystick(context, {0, +1}, 3600ms, 100ms); // 18+
    pbf_move_left_joystick(context, {-1, 0}, 200ms, 100ms); // 1

    {
        ShortDialogWatcher talk_to_the_grunt(COLOR_BLUE);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {-1, 0}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{talk_to_the_grunt}}
        );
        if (ret < 0) {
            stream.log("celestic_town_talk_to_the_grunt: dialog box not detected!", COLOR_RED);
        }
        if (ret == 0){
            stream.log("celestic_town_talk_to_the_grunt: talking to the grunt!");

            BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
            int ret1 = run_until<ProControllerContext>(
                stream, context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 200000ms);
                },
                {{black_screen}}
            );
            if (ret1 < 0){
                stream.log("celestic_town_talk_to_the_grunt: Didn't detect start of battle with Clint!", COLOR_RED);
                return false;
            }
            stream.log("celestic_town_talk_to_the_grunt: starting the battle.");
            handle_battle(stream, context, "celestic_town_galactic_grunt");
            pbf_mash_button(context, BUTTON_B, 17000ms);
        };
    }

    context.wait_for_all_requests();
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 1);
    
    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 20000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("celestic_town_enter_cave: black screen not detected!", COLOR_RED);
            return false;
        }
    }

    return true;
}


static bool discover_the_paintings(
    VideoStream& stream,
    ProControllerContext& context
){
    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("discover_the_paintings: going to the paintings.");

    pbf_move_left_joystick(context, {0, +1}, 1600ms, 100ms); // 8+
    pbf_move_left_joystick(context, {-1, 0}, 1200ms, 100ms); // 6+
    pbf_move_left_joystick(context, {0, +1}, 1400ms, 100ms); // 7+
    pbf_move_left_joystick(context, {+1, 0}, 400ms, 100ms); // 2+
    pbf_move_left_joystick(context, {0, +1}, 400ms, 100ms); // 2+
    dpad.last_dir = DPAD_UP;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 3);

    {
        ShortDialogWatcher talk_to_cynthias_grandma(COLOR_BLUE);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, {0, +1}, 0ms, 5000ms);
                ssf_mash1_button(context, BUTTON_A, 5000ms);
            },
            {{talk_to_cynthias_grandma}}
        );
        if (ret < 0) {
            stream.log("discover_the_paintings_talk_to_cynthias_grandma: dialog box not detected!", COLOR_RED);
        }
        if (ret == 0){
            stream.log("discover_the_paintings_talk_to_cynthias_grandma: talking to the grandma!");
            pbf_mash_button(context, BUTTON_B, 20000ms);
        };
    }
    context.wait_for_all_requests();
    pbf_wait(context, 5000ms);

    pbf_move_left_joystick(context, {-1, 0}, 600ms, 100ms); // 3+
    pbf_move_left_joystick(context, {0, -1}, 400ms, 100ms); // 2+
    pbf_move_left_joystick(context, {-1, 0}, 400ms, 100ms); // 2+
    pbf_move_left_joystick(context, {0, -1}, 1400ms, 100ms); // 7+

    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 6);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, -1}, 20000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("discover_the_paintings: black screen for exiting the building not detected!", COLOR_RED);
            return false;
        }
    }

    return true;
}

static bool talk_to_cyrus_and_fly_hearthome(
    VideoStream& stream,
    ProControllerContext& context
){
    pbf_press_button(context, BUTTON_L, 80ms, 300ms);
    pbf_press_button(context, BUTTON_L, 80ms, 300ms);
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("talk_to_cyrus_and_fly_hearthome: continuing the drama.");

    wait_for_dialogue(stream, context, "Cyrus", 10000ms);
    pbf_mash_button(context, BUTTON_B, 10000ms);

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    fly_to(stream, context, FlyPoint::HearthomeCity);

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);

    {
        BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 20000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("talk_to_cyrus_and_fly_hearthome: black screen for entering pokemon center not detected!", COLOR_RED);
            return false;
        }
    }
    return true;
}


static bool heal_at_hearthome_pokemon_center(
    VideoStream& stream,
    ProControllerContext& context
){
    stream.log("heal_at_hearthome_pokemon_center: healing pokemon");
    context.wait_for_all_requests();
    heal_pokemon(stream, context, "Hearthome");
    return true;
}

static bool get_into_hearthome_gym(
    VideoStream& stream,
    ProControllerContext& context
){
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
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 20000ms, 100ms);
            },
            {{black_screen}}
        );
        if (ret < 0){
            stream.log("get_into_hearthome_gym: black screen not detected!", COLOR_RED);
            return false;
        }
    }

    return true;
}

void checkpoint_0501(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_to_pastoria_pokemon_center(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_to_pastoria_pokemon_center: transition not detected.", env.console);
            }
        },
        true
    );
}

void checkpoint_0502(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!exit_pastoria_pokemon_center(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "exit_pastoria_pokemon_center: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_0503(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!pastoria_city_drama(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "pastoria_city_drama: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_0504(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_through_house_east_pastoria(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_through_house_east_pastoria: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_0505(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_through_route_213_chasing(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_through_route_213_chasing: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_0506(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_through_building_route_213(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_through_building_route_213: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_0507(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_through_route_214(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_through_route_214: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_0508(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_inside_solaceon_pokemon_center(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_inside_solaceon_pokemon_center: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_0509(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!heal_at_solaceon_pokemon_center(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "heal_at_solaceon_pokemon_center: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_0510(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!go_to_the_psyduck_route_210(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "go_to_the_psyduck_route_210: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_0511(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!heal_at_celestic_pokemon_center(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "heal_at_celestic_pokemon_center: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_0512(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!celestic_town_drama(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "celestic_town_drama: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_0513(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!discover_the_paintings(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "discover_the_paintings: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_0514(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!talk_to_cyrus_and_fly_hearthome(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "talk_to_cyrus_and_fly_hearthome: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_0515(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!heal_at_hearthome_pokemon_center(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "heal_at_hearthome_pokemon_center: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_0516(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!get_into_hearthome_gym(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "get_into_hearthome_gym: transition not detected.", env.console);
            }
        },
        false
    );
}

void checkpoint_0517(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            if (!get_into_hearthome_gym(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "get_into_hearthome_gym: transition not detected.", env.console);
            }
        },
        false
    );
}

}
}
}
