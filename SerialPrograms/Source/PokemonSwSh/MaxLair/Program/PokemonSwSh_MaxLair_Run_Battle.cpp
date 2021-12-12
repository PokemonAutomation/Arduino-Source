/*  Max Lair Battle
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Exception.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Inference/VisualInferenceSession.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_PushButtons.h"
#include "PokemonSwSh/Resources/PokemonSwSh_MaxLairDatabase.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.h"
#include "PokemonSwSh/Inference/Dens/PokemonSwSh_RaidCatchDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_BasicCatcher.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonReader.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_BattleMenu.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_EndBattle.h"
#include "PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI.h"
//#include "PokemonSwSh_MaxLair_Run_CaughtScreen.h"
#include "PokemonSwSh_MaxLair_Run_Battle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


bool read_battle_menu(
    ProgramEnvironment& env,
    ConsoleHandle& console, size_t player_index,
    GlobalState& state,
    const ConsoleSpecificOptions& settings,
    bool currently_dmaxed, bool cheer_only
){
    PlayerState& player = state.players[player_index];

    std::deque<InferenceBoxScope> boxes;
    BattleMenuReader reader(console, settings.language);
    BattleMoveArrowFinder arrow_finder(console);
    arrow_finder.make_overlays(boxes, console);


    //  Read raid mon.
    do{
        std::set<std::string> mon = reader.read_opponent(console, env, console);
        if (mon.size() == 1){
            state.opponent = std::move(mon);
            break;
        }
        if (mon.size() > 1){
            console.log("Ambiguous Read Result: " + set_to_str(mon), "purple");
            if (state.opponent.size() == 1 && mon.find(*state.opponent.begin()) != mon.end()){
                console.log("Using previous known value to disambiguate: " + set_to_str(mon), "purple");
                break;
            }
        }
        console.log("Attempting to read from summary.", "purple");
        pbf_press_button(console, BUTTON_Y, 10, TICKS_PER_SECOND);
        pbf_press_dpad(console, DPAD_UP, 10, 50);
        pbf_press_button(console, BUTTON_A, 10, 2 * TICKS_PER_SECOND);
        console.botbase().wait_for_all_requests();
        mon = reader.read_opponent_in_summary(console, console.video().snapshot());
        pbf_mash_button(console, BUTTON_B, 3 * TICKS_PER_SECOND);
        state.opponent = std::move(mon);

        if (state.wins == 3 && !state.boss.empty()){
            if (!state.opponent.empty() && *state.opponent.begin() != state.boss){
                console.log("Inconsistent Boss: Expected " + state.boss + ", Read: " + *state.opponent.begin(), Qt::red);
            }
            state.opponent = {state.boss};
            break;
        }
    }while (false);
    console.botbase().wait_for_all_requests();

    const std::string& opponent = state.opponent.empty()
        ? ""
        : *state.opponent.begin();

    if (state.wins != 3 && is_boss(opponent)){
        console.log("Boss found before 3 wins. Something is seriously out-of-sync.", Qt::red);
        dump_image(console, MODULE_NAME, "BossBeforeEnd", console.video().snapshot());
//        send_program_telemetry(
//            env.logger(), true, Qt::red, MODULE_NAME,
//            "Error",
//            {{"Message", "Boss found before 3 wins."}},
//            ""
//        );
        return false;
    }

    //  Infer the boss if we don't know it.
    if (state.boss.empty() && state.wins == 3){
        state.boss = opponent;
    }

    //  Read misc.
    QImage screen = console.video().snapshot();
    state.opponent_hp = reader.read_opponent_hp(console, screen);
    if (cheer_only){
        player.dmax_turns_left = 0;
        player.health = Health{0, 1};
        player.can_dmax = false;
        pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
        console.botbase().wait_for_all_requests();
        return true;
    }

    if (currently_dmaxed){
        player.dmax_turns_left--;
        if (player.dmax_turns_left <= 0){
            console.log("State Inconsistency: dmax_turns_left <= 0 && currently_dmaxed == true", Qt::red);
            player.dmax_turns_left = 1;
        }
    }else{
        std::string name = reader.read_own_mon(console, screen);
        if (!name.empty()){
            state.players[player_index].pokemon = std::move(name);
        }
        if (player.dmax_turns_left > 1){
            console.log("State Inconsistency: dmax_turns_left > 0 && currently_dmaxed == false", Qt::red);
            state.move_slot = 0;
        }
        if (player.dmax_turns_left == 1){
            console.log("End of Dmax.");
            state.move_slot = 0;
        }
        player.dmax_turns_left = 0;
    }

    Health health[4];
    reader.read_hp(console, screen, health, player_index);
    if (health[0].hp >= 0) state.players[0].health = health[0];
    if (health[1].hp >= 0) state.players[1].health = health[1];
    if (health[2].hp >= 0) state.players[2].health = health[2];
    if (health[3].hp >= 0) state.players[3].health = health[3];


    //  Enter move selection to read PP.
//    AsyncVisualInferenceSession inference(env, console, console);
//    inference += arrow_finder;

    pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
    console.botbase().wait_for_all_requests();


    //  Clear move blocked status.
    player.move_blocked[0] = false;
    player.move_blocked[1] = false;
    player.move_blocked[2] = false;
    player.move_blocked[3] = false;


    screen = console.video().snapshot();

    int8_t pp[4] = {-1, -1, -1, -1};
    reader.read_own_pp(console, screen, pp);
    player.pp[0] = pp[0];
    player.pp[1] = pp[1];
    player.pp[2] = pp[2];
    player.pp[3] = pp[3];

    player.can_dmax = reader.can_dmax(screen);

    //  Read move slot.
//    int8_t move_slot = arrow_finder.get_slot();
    int8_t move_slot = arrow_finder.detect(screen);
    if (move_slot < 0){
        console.log("Unable to detect move slot.", Qt::red);
        dump_image(console, MODULE_NAME, "MoveSlot", screen);
        pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
        pbf_press_dpad(console, DPAD_RIGHT, 2 * TICKS_PER_SECOND, 0);
        pbf_press_dpad(console, DPAD_UP, 2 * TICKS_PER_SECOND, 0);
        move_slot = 0;
    }else{
        console.log("Current Move Slot: " + std::to_string(move_slot), Qt::blue);
    }
    if (move_slot != state.move_slot){
        console.log(
            "Move Slot Mismatch: Expected = " + std::to_string(state.move_slot) + ", Actual = " + std::to_string(move_slot),
            Qt::red
        );
    }
    state.move_slot = move_slot;

//    inference.stop();
    return true;
}


StateMachineAction run_move_select(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    GlobalStateTracker& state_tracker,
    const ConsoleSpecificOptions& settings,
    bool currently_dmaxed, bool cheer_only
){
    size_t console_index = console.index();
    GlobalState& state = state_tracker[console_index];
    size_t player_index = state.find_player_index(console_index);
    PlayerState& player = state.players[player_index];


    if (!read_battle_menu(
        env, console, player_index,
        state, settings,
        currently_dmaxed, cheer_only
    )){
        return StateMachineAction::RESET_RECOVER;
    }


    GlobalState inferred = state_tracker.synchronize(env, console, console_index);


    while (true){
        console.log("Selecting move...");

        if (cheer_only){
            console.log("Choosing move Cheer. (you are dead)", "purple");
//            pbf_mash_button(console, BUTTON_A, 2 * TICKS_PER_SECOND);
//            console.botbase().wait_for_all_requests();
            break;
        }

        std::pair<uint8_t, bool> move = select_move(
            console,
            inferred,
            player_index
        );
        console.log("Choosing move " + std::to_string((int)move.first) + (move.second ? " (dmax)." : "."), "purple");

        if (player.can_dmax && move.second){
            pbf_press_dpad(console, DPAD_LEFT, 10, 50);
            pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
            player.dmax_turns_left = 3;
        }
        while (state.move_slot != move.first){
            pbf_press_dpad(console, DPAD_DOWN, 10, 50);
            state.move_slot++;
            state.move_slot %= 4;
        }

        //  Enter the move.
        pbf_mash_button(console, BUTTON_A, 2 * TICKS_PER_SECOND);
        console.botbase().wait_for_all_requests();

//        inference.stop();

        //  Back out and look for battle menu. This indicates that the move wasn't selectable.
        BattleMenuDetector detector;
        int result = run_until(
            env, console,
            [](const BotBaseContext& context){
                pbf_mash_button(context, BUTTON_B, 5 * TICKS_PER_SECOND);
            },
            { &detector },
            INFERENCE_RATE
        );

        //  No battle menu detected, we're good.
        if (result < 0){
            player.move_blocked[state.move_slot] = false;
            break;
        }

        //  Battle menu detected. It means the move wasn't selectable.

        console.log("Move not selectable.", Qt::magenta);
        player.move_blocked[state.move_slot] = true;

        bool no_moves = true;
        for (size_t c = 0; c < 4; c++){
            no_moves &= player.move_blocked[c];
        }
        if (no_moves){
            console.log("All moves reported as blocked. This is impossible. Clearing state.", Qt::red);
            for (size_t c = 0; c < 4; c++){
                player.move_blocked[c] = false;
            }
        }

        state_tracker.push_update(console_index);

        //  Reset position.
        pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
        pbf_press_dpad(console, DPAD_RIGHT, 2 * TICKS_PER_SECOND, 0);
        pbf_press_dpad(console, DPAD_UP, 2 * TICKS_PER_SECOND, 0);
        state.move_slot = 0;

        inferred = state_tracker.infer_actual_state(console_index);
//        inferred.players[player_index].move_blocked[state.move_slot] = true;

    }

//    inference.stop();
    return StateMachineAction::KEEP_GOING;
}



StateMachineAction throw_balls(
    AdventureRuntime& runtime,
    ProgramEnvironment& env,
    ConsoleHandle& console, Language language,
    GlobalStateTracker& state_tracker,
    const EndBattleDecider& decider
){
    size_t console_index = console.index();
    GlobalState& state = state_tracker[console_index];
    state.clear_battle_state();

    state.wins++;
    state.players[0].health.value.dead = 0;
    state.players[1].health.value.dead = 0;
    state.players[2].health.value.dead = 0;
    state.players[3].health.value.dead = 0;


    GlobalState inferred = state_tracker.synchronize(env, console, console_index);


    std::string ball;

    bool boss = inferred.wins == 4;
    if (boss){
        ball = decider.boss_ball(console_index, inferred.boss);
    }else{
        ball = decider.normal_ball(console_index);
    }

    BattleBallReader reader(console, language);
    pbf_press_button(console, BUTTON_A, 10, 125);
    console.botbase().wait_for_all_requests();

    int16_t balls = move_to_ball(reader, console, ball);
    if (balls != 0){
        pbf_press_button(console, BUTTON_A, 10, 125);
    }else{
        console.log("Unable to find appropriate ball. Did you run out?", Qt::red);
        PA_THROW_StringException("Unable to find appropriate ball. Did you run out?");
    }

    ReadableQuantity999& stat = boss
        ? runtime.consoles[console_index].boss_balls
        : runtime.consoles[console_index].normal_balls;

    stat.update_with_ocr(balls, boss ? -1 : 1);
    stat.quantity = std::max(stat.quantity - 1, 0);

    return StateMachineAction::KEEP_GOING;
}




}
}
}
}
