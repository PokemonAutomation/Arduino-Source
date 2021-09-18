/*  Max Lair Battle
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Exception.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Inference/VisualInferenceSession.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.h"
#include "PokemonSwSh/Inference/Dens/PokemonSwSh_RaidCatchDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_BasicCatcher.h"
#include "PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonReader.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_BattleMenu.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_EndBattle.h"
#include "PokemonSwSh_MaxLair_Run_Battle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void read_battle_menu(
    ProgramEnvironment& env,
    ConsoleHandle& console, size_t player_index,
    GlobalState& state,
    const MaxLairPlayerOptions& settings,
    bool currently_dmaxed, bool cheer_only
){
    PlayerState& player = state.players[player_index];

    BattleMenuReader reader(console, settings.language);
    BattleMoveArrowFinder arrow_finder(console);


    QImage screen = console.video().snapshot();

    //  Read raid mon.
    std::set<std::string> mon = reader.read_opponent(console, screen);
    if (!mon.empty()){
        if (mon.size() > 1){
//            console.log("Ambiguous Read Result: " + set_to_str(mon) + " - Disambiguation not yet supported.", Qt::red);
            console.log("Ambiguous Read Result: " + set_to_str(mon), "purple");
            if (state.opponent.size() == 1 && mon.find(*state.opponent.begin()) != mon.end()){
                console.log("Using previous known value to disambiguate: " + set_to_str(mon), "purple");
            }else{
                pbf_press_button(console, BUTTON_Y, 10, TICKS_PER_SECOND);
                pbf_press_dpad(console, DPAD_UP, 10, 50);
                pbf_press_button(console, BUTTON_A, 10, 2 * TICKS_PER_SECOND);
                console.botbase().wait_for_all_requests();
                reader.disambiguate_opponent(console, mon, console.video().snapshot());
                pbf_mash_button(console, BUTTON_B, 3 * TICKS_PER_SECOND);
            }
        }
        state.opponent = std::move(mon);
    }

    //  Read misc.
    state.opponent_hp = reader.read_opponent_hp(console, screen);
    if (cheer_only){
        player.dmax_turns_left = 0;
        player.health = Health{0, 1};
        player.can_dmax = false;
        pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
        console.botbase().wait_for_all_requests();
        return;
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
    AsyncVisualInferenceSession inference(env, console, console);
    inference += arrow_finder;

    pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
    console.botbase().wait_for_all_requests();


    screen = console.video().snapshot();

    int8_t pp[4] = {-1, -1, -1, -1};
    reader.read_own_pp(console, screen, pp);
    player.pp[0] = pp[0];
    player.pp[1] = pp[1];
    player.pp[2] = pp[2];
    player.pp[3] = pp[3];

    player.can_dmax = reader.can_dmax(screen);

    //  Read move slot.
    int8_t move_slot = arrow_finder.get_slot();
    if (move_slot < 0){
        console.log("Unable to detect move slot.", Qt::red);
//            dump_image(console, screen, "MoveSlot");
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

    inference.stop();
}


void run_move_select(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    GlobalStateTracker& state_tracker,
    const MaxLairPlayerOptions& settings,
    bool currently_dmaxed, bool cheer_only
){
    size_t console_index = console.index();
    GlobalState& state = state_tracker[console_index];
    size_t player_index = state.find_player_index(console_index);
    PlayerState& player = state.players[player_index];


    read_battle_menu(
        env, console, player_index,
        state, settings,
        currently_dmaxed, cheer_only
    );


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
                pbf_mash_button(context, BUTTON_B, 2 * TICKS_PER_SECOND);
                context->wait_for_all_requests();
            },
            { &detector }
        );
//        pbf_mash_button(console, BUTTON_B, 2 * TICKS_PER_SECOND);
//        console.botbase().wait_for_all_requests();

        //  If we detect the battle menu here, it means the move wasn't selectable.
        if (result < 0){
            player.move_blocked[state.move_slot] = false;
            break;
        }

        console.log("Move not selectable.", Qt::magenta);
        player.move_blocked[state.move_slot] = true;
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
}



StateMachineAction throw_balls(
    MaxLairRuntime& runtime,
    ProgramEnvironment& env,
    ConsoleHandle& console,
    GlobalStateTracker& state_tracker,
    const MaxLairPlayerOptions& settings
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


    bool boss = inferred.wins == 4;
    if (boss && runtime.action == CaughtScreenAction::ALWAYS_STOP){
        return StateMachineAction::STOP_PROGRAM;
    }

    BattleBallReader reader(console, settings.language);
    pbf_press_button(console, BUTTON_A, 10, 125);
    console.botbase().wait_for_all_requests();

    std::string ball = boss
        ? settings.boss_ball.slug()
        : settings.normal_ball.slug();

    if (move_to_ball(reader, console, ball)){
        pbf_press_button(console, BUTTON_A, 10, 125);
    }else{
        console.log("Unable to find appropriate ball. Did you run out?", Qt::red);
//        pbf_press_dpad(console, DPAD_DOWN, 10, 50);
//        pbf_press_button(console, BUTTON_A, 10, 125);
        PA_THROW_StringException("Unable to find appropriate ball. Did you run out?");
    }

    return StateMachineAction::KEEP_GOING;
}




}
}
}
}
