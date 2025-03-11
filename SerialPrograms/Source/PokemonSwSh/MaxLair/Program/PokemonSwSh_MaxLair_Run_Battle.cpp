/*  Max Lair Battle
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh/Resources/PokemonSwSh_MaxLairDatabase.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.h"
#include "PokemonSwSh/Programs/PokemonSwSh_BasicCatcher.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_BattleMenu.h"
#include "PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI.h"
//#include "PokemonSwSh_MaxLair_Run_CaughtScreen.h"
#include "PokemonSwSh_MaxLair_Run_Battle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


bool read_battle_menu(
    ProgramEnvironment& env,
    VideoStream& stream, ProControllerContext& context, size_t player_index,
    OcrFailureWatchdog& ocr_watchdog,
    GlobalState& state,
    const ConsoleSpecificOptions& settings,
    bool currently_dmaxed, bool cheer_only
){
    PlayerState& player = state.players[player_index];

    VideoOverlaySet boxes(stream.overlay());
    BattleMenuReader reader(stream.overlay(), settings.language, ocr_watchdog);
    BattleMoveArrowFinder arrow_finder(stream.overlay());
    arrow_finder.make_overlays(boxes);


    //  Read raid mon.
    do{
        std::set<std::string> mon = reader.read_opponent(stream.logger(), context, stream.video());
        if (mon.size() == 1){
            state.opponent = std::move(mon);
            break;
        }
        if (mon.size() > 1){
            stream.log("Ambiguous Read Result: " + set_to_str(mon), COLOR_PURPLE);
            if (state.opponent.size() == 1 && mon.find(*state.opponent.begin()) != mon.end()){
                stream.log("Using previous known value to disambiguate: " + set_to_str(mon), COLOR_PURPLE);
                break;
            }
        }
        if (state.opponent.size() == 1){
            stream.log("Failed to read opponent from battle. Using previously known value: " + set_to_str(state.opponent), COLOR_ORANGE);
            break;
        }

        stream.log("Unable to read opponent from battle. Attempting to read from summary.", COLOR_ORANGE);
        pbf_press_button(context, BUTTON_Y, 10, TICKS_PER_SECOND);
        pbf_press_dpad(context, DPAD_UP, 10, 50);
        pbf_press_button(context, BUTTON_A, 10, 2 * TICKS_PER_SECOND);
        context.wait_for_all_requests();
        mon = reader.read_opponent_in_summary(stream.logger(), stream.video().snapshot());
        pbf_mash_button(context, BUTTON_B, 3 * TICKS_PER_SECOND);
        state.opponent = std::move(mon);

        if (state.wins == 3 && !state.boss.empty()){
            if (!state.opponent.empty() && *state.opponent.begin() != state.boss){
                stream.log("Inconsistent Boss: Expected " + state.boss + ", Read: " + *state.opponent.begin(), COLOR_RED);
            }
            state.opponent = {state.boss};
            break;
        }
    }while (false);
    context.wait_for_all_requests();

    const std::string& opponent = state.opponent.empty()
        ? ""
        : *state.opponent.begin();

    if (state.wins != 3 && is_boss(opponent)){
        stream.log("Boss found before 3 wins. Something is seriously out-of-sync.", COLOR_RED);
        dump_image(stream.logger(), MODULE_NAME, stream.video(), "BossBeforeEnd");
//        send_program_telemetry(
//            env.logger(), true, COLOR_RED, MODULE_NAME,
//            "Error",
//            {{"Message:", "Boss found before 3 wins."}},
//            ""
//        );
        return false;
    }

    //  Infer the boss if we don't know it.
    if (state.boss.empty() && state.wins == 3){
        state.boss = opponent;
    }

    //  Read misc.
    VideoSnapshot screen = stream.video().snapshot();
    state.opponent_hp = reader.read_opponent_hp(stream.logger(), screen);
    if (cheer_only){
        player.dmax_turns_left = 0;
        player.health = Health{0, 1};
        player.can_dmax = false;
        pbf_press_button(context, BUTTON_A, 10, TICKS_PER_SECOND);
        context.wait_for_all_requests();
        return true;
    }

    if (currently_dmaxed){
        player.dmax_turns_left--;
        if (player.dmax_turns_left <= 0){
            stream.log("State Inconsistency: dmax_turns_left <= 0 && currently_dmaxed == true", COLOR_RED);
            player.dmax_turns_left = 1;
        }
    }else{
        std::string name = reader.read_own_mon(stream.logger(), screen);
        if (!name.empty()){
            state.players[player_index].pokemon = std::move(name);
        }
        if (player.dmax_turns_left > 1){
            stream.log("State Inconsistency: dmax_turns_left > 0 && currently_dmaxed == false", COLOR_RED);
            state.move_slot = 0;
        }
        if (player.dmax_turns_left == 1){
            stream.log("End of Dmax.");
            state.move_slot = 0;
        }
        player.dmax_turns_left = 0;
    }

    Health health[4];
    reader.read_hp(stream.logger(), screen, health, player_index);
    if (health[0].hp >= 0) state.players[0].health = health[0];
    if (health[1].hp >= 0) state.players[1].health = health[1];
    if (health[2].hp >= 0) state.players[2].health = health[2];
    if (health[3].hp >= 0) state.players[3].health = health[3];


    for (size_t attempts = 0; attempts < 5; attempts++){
        //  Enter move selection to read PP.
        pbf_press_button(context, BUTTON_A, 10, TICKS_PER_SECOND);
        context.wait_for_all_requests();


        //  Clear move blocked status.
        player.move_blocked[0] = false;
        player.move_blocked[1] = false;
        player.move_blocked[2] = false;
        player.move_blocked[3] = false;

        screen = stream.video().snapshot();

        int8_t pp[4] = {-1, -1, -1, -1};
        reader.read_own_pp(stream.logger(), screen, pp);
        player.pp[0] = pp[0];
        player.pp[1] = pp[1];
        player.pp[2] = pp[2];
        player.pp[3] = pp[3];

        player.can_dmax = reader.can_dmax(screen);

        //  Read move slot.
    //    int8_t move_slot = arrow_finder.get_slot();
        int8_t move_slot = arrow_finder.detect(screen);
        if (move_slot < 0){
            stream.log("Unable to detect move slot.", COLOR_RED);
//            dump_image(stream.logger(), MODULE_NAME, "MoveSlot", screen);
//            pbf_press_button(context, BUTTON_A, 10, TICKS_PER_SECOND);
//            pbf_press_dpad(context, DPAD_RIGHT, 2 * TICKS_PER_SECOND, 0);
//            pbf_press_dpad(context, DPAD_UP, 2 * TICKS_PER_SECOND, 0);
//            move_slot = 0;
            pbf_mash_button(context, BUTTON_B, 1 * TICKS_PER_SECOND);
            continue;
        }else{
            stream.log("Current Move Slot: " + std::to_string(move_slot), COLOR_BLUE);
        }
        if (move_slot != state.move_slot){
            stream.log(
                "Move Slot Mismatch: Expected = " + std::to_string(state.move_slot) + ", Actual = " + std::to_string(move_slot),
                COLOR_RED
            );
        }
        state.move_slot = move_slot;

        return true;
    }

    dump_image(stream.logger(), MODULE_NAME, "MoveSlot", screen);
    return true;
}


StateMachineAction run_move_select(
    ProgramEnvironment& env, size_t console_index,
    VideoStream& stream, ProControllerContext& context,
    OcrFailureWatchdog& ocr_watchdog,
    GlobalStateTracker& state_tracker,
    const ConsoleSpecificOptions& settings,
    bool currently_dmaxed, bool cheer_only
){
    GlobalState& state = state_tracker[console_index];
    size_t player_index = state.find_player_index(console_index);
    PlayerState& player = state.players[player_index];


    if (!read_battle_menu(
        env, stream, context, player_index,
        ocr_watchdog,
        state, settings,
        currently_dmaxed, cheer_only
    )){
        return StateMachineAction::RESET_RECOVER;
    }


    GlobalState inferred = state_tracker.synchronize(stream.logger(), console_index);

    bool all_moves_blocked = false;

    while (true){
        stream.log("Selecting move...");

        if (cheer_only){
            stream.log("Choosing move Cheer. (you are dead)", COLOR_PURPLE);
//            pbf_mash_button(context, BUTTON_A, 2 * TICKS_PER_SECOND);
//            context.wait_for_all_requests();
            break;
        }

        std::pair<uint8_t, bool> move = select_move(
            stream.logger(),
            inferred,
            player_index
        );
        stream.log("Choosing move " + std::to_string((int)move.first) + (move.second ? " (dmax)." : "."), COLOR_PURPLE);

        if (player.can_dmax && move.second){
            pbf_press_dpad(context, DPAD_LEFT, 10, 50);
            pbf_press_button(context, BUTTON_A, 10, TICKS_PER_SECOND);
            player.dmax_turns_left = 3;
        }
        while (state.move_slot != move.first){
            pbf_press_dpad(context, DPAD_DOWN, 10, 50);
            state.move_slot++;
            state.move_slot %= 4;
        }

        //  Enter the move.
        if (all_moves_blocked){
            //  If we had trouble selecting a move, then we're probably stuck in a self-target loop.
            //  Force target the opponent.
            stream.log("Force targeting opponent due to inability to select a move after multiple attempts...", COLOR_RED);
            pbf_press_button(context, BUTTON_A, 20, 2 * TICKS_PER_SECOND);
            pbf_press_dpad(context, DPAD_UP, 2 * TICKS_PER_SECOND, 0);
        }
        pbf_mash_button(context, BUTTON_A, 2 * TICKS_PER_SECOND);
        context.wait_for_all_requests();

//        inference.stop();

        //  Back out and look for battle menu. This indicates that the move wasn't selectable.
        BattleMenuDetector detector;
        int result = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_B, 5 * TICKS_PER_SECOND);
            },
            {{detector}},
            INFERENCE_RATE
        );

        //  No battle menu detected, we're good.
        if (result < 0){
            player.move_blocked[state.move_slot] = false;
            break;
        }

        //  Battle menu detected. It means the move wasn't selectable.

        stream.log("Move not selectable.", COLOR_RED);
        player.move_blocked[state.move_slot] = true;

        bool no_moves = true;
        for (size_t c = 0; c < 4; c++){
            no_moves &= player.move_blocked[c];
        }
        if (no_moves){
            stream.log("All moves reported as blocked. This is impossible. Clearing state.", COLOR_RED);
            for (size_t c = 0; c < 4; c++){
                player.move_blocked[c] = false;
            }
            all_moves_blocked = true;
        }

        state_tracker.push_update(console_index);

        //  Reset position.
        pbf_press_button(context, BUTTON_A, 10, TICKS_PER_SECOND);
        pbf_press_dpad(context, DPAD_RIGHT, 2 * TICKS_PER_SECOND, 0);
        pbf_press_dpad(context, DPAD_UP, 2 * TICKS_PER_SECOND, 0);
        state.move_slot = 0;

        inferred = state_tracker.infer_actual_state(console_index);
//        inferred.players[player_index].move_blocked[state.move_slot] = true;

    }

//    inference.stop();
    return StateMachineAction::KEEP_GOING;
}



StateMachineAction throw_balls(
    AdventureRuntime& runtime,
    ProgramEnvironment& env, size_t console_index,
    VideoStream& stream, ProControllerContext& context,
    Language language,
    OcrFailureWatchdog& ocr_watchdog,
    GlobalStateTracker& state_tracker,
    const EndBattleDecider& decider
){
    GlobalState& state = state_tracker[console_index];
    state.clear_battle_state();

    state.wins++;
    state.players[0].health.value.dead = 0;
    state.players[1].health.value.dead = 0;
    state.players[2].health.value.dead = 0;
    state.players[3].health.value.dead = 0;


    GlobalState inferred = state_tracker.synchronize(stream.logger(), console_index);


    std::string ball;

    bool boss = inferred.wins == 4;
    if (boss){
        ball = decider.boss_ball(console_index, inferred.boss);
    }else{
        ball = decider.normal_ball(console_index);
    }

    BattleBallReader reader(stream, language);
    pbf_press_button(context, BUTTON_A, 50, 75);
    context.wait_for_all_requests();

    int16_t balls = move_to_ball(reader, stream, context, ball);
    if (balls != 0){
        pbf_mash_button(context, BUTTON_A, 1000ms);
    }else{
        OperationFailedException::fire(
            ErrorReport::NO_ERROR_REPORT,
            "Unable to find appropriate ball. Did you run out?",
            stream
        );
    }

    ReadableQuantity999& stat = boss
        ? runtime.consoles[console_index].boss_balls
        : runtime.consoles[console_index].normal_balls;

    stat.update_with_ocr(balls, boss ? -1 : 1);
    stat.quantity = (uint16_t)std::max((int)stat.quantity - 1, 0);

    return StateMachineAction::KEEP_GOING;
}




}
}
}
}
