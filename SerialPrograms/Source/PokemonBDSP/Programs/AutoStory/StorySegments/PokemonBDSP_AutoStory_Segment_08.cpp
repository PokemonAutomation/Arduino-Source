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
#include "../PokemonBDSP_AutoStoryTools.h"
#include "../Utils/PokemonBDSP_AutoStory_Battle.h"
#include "../PokemonBDSP_AutoStoryTools.h"
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

    pbf_move_left_joystick(context, {0, -1}, 3600ms, 100ms); // 23
    pbf_move_left_joystick(context, {-1, 0}, 3800ms, 100ms); // 21
    pbf_move_left_joystick(context, {0, +1}, 1800ms, 100ms); // 9+
    pbf_move_left_joystick(context, {-1, 0}, 2600ms, 100ms); // 13+
    pbf_move_left_joystick(context, {0, +1}, 2600ms, 100ms); // 13+
    pbf_move_left_joystick(context, {+1, 0}, 2600ms, 100ms); // 12+
    dpad.last_dir = DPAD_RIGHT;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 2);

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
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "heal_and_exit: transition not detected.", env.console);
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
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "heal_and_exit: transition not detected.", env.console);
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
