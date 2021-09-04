/*  Max Lair Battle
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Inference/VisualInferenceSession.h"
#include "PokemonSwSh/Inference/Dens/PokemonSwSh_RaidCatchDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_BasicCatcher.h"
#include "PokemonSwSh/Programs/MaxLair/PokemonSwSh_MaxLair_AI.h"
#include "PokemonSwSh/Programs/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonReader.h"
#include "PokemonSwSh/Programs/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_BattleMenu.h"
#include "PokemonSwSh/Programs/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_EndBattle.h"
#include "PokemonSwSh_MaxLair_Run_Battle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void run_move_select(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    GlobalStateTracker& state_tracker,
    const MaxLairPlayerOptions& settings
){
    size_t console_index = console.index();
    GlobalState& state = state_tracker[console_index];
    size_t player_index = state.find_player_index(console_index);


    BattleMenuReader reader(console);
    pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
    QImage screen = console.video().snapshot();
    std::string mon = read_raid_mon(
        console, console,
        screen,
        settings.language
    );
    if (!mon.empty()){
        state.opponent = std::move(mon);
    }
    state.opponent_hp = reader.read_opponent_hp(console, screen);
    console.botbase().wait_for_all_requests();


    GlobalState inferred = state_tracker.synchronize(console_index);


    while (true){
        console.log("Selecting move...");

        PlayerState& player = state.players[player_index];

        std::pair<uint8_t, bool> move = select_move(
            console,
            inferred,
            player_index
        );
        console.log("Choosing move " + std::to_string((int)move.first) + (move.second ? " (dmax)." : "."), "purple");

        if (move.second){
            pbf_press_dpad(console, DPAD_LEFT, 10, 50);
        }
        while (state.move_slot != move.first){
            pbf_press_dpad(console, DPAD_DOWN, 10, 50);
            state.move_slot++;
            state.move_slot %= 4;
        }

        //  Enter the move.
        pbf_mash_button(console, BUTTON_A, 2 * TICKS_PER_SECOND);
        console.botbase().wait_for_all_requests();

        //  Back out and look for battle menu. This indicates that the move wasn't selectable.
        BattleMenuDetector detector(console);
        pbf_mash_button(console, BUTTON_B, 2 * TICKS_PER_SECOND);
        console.botbase().wait_for_all_requests();

        //  If we detect the battle menu here, it means the move wasn't selectable.
        if (!detector.detect(console.video().snapshot())){
            player.move_blocked[state.move_slot] = false;
            return;
        }

        console.log("Move not selectable.", Qt::magenta);
        player.move_blocked[state.move_slot] = true;
        state_tracker.push_update(console_index);

        pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
        inferred = state_tracker.infer_actual_state(console_index);
//        inferred.players[player_index].move_blocked[state.move_slot] = true;
    }
}



void throw_balls(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    GlobalStateTracker& state_tracker,
    const MaxLairPlayerOptions& settings
){
    size_t console_index = console.index();
    GlobalState& state = state_tracker[console_index];
    state.clear_battle_state();


    state.wins++;


    GlobalState inferred = state_tracker.synchronize(console_index);


    BattleBallReader reader(console, settings.language);
    pbf_press_button(console, BUTTON_A, 10, 125);
    console.botbase().wait_for_all_requests();

    std::string ball = inferred.wins == 4
        ? settings.boss_ball.slug()
        : settings.normal_ball.slug();

    if (move_to_ball(reader, console, ball)){
        pbf_press_button(console, BUTTON_A, 10, 125);
    }else{
        console.log("Unable to find appropriate ball. Did you run out?", Qt::red);
        pbf_press_dpad(console, DPAD_DOWN, 10, 50);
        pbf_press_button(console, BUTTON_A, 10, 125);
    }
}




}
}
}
}
