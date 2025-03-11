/*  Singles Battler
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV_SinglesBattler.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


//  Run a single move for a single turn.
//  Returns true if move is successfully selected.
bool run_move_select(
    VideoStream& stream, ProControllerContext& context,
    MoveSelectWatcher& move_select_menu,
    SinglesMoveEntry& move, size_t consecutive_move_select
){
    uint8_t index = 0;
    switch (move.type){
    case SinglesMoveType::Move1:
        index = 0;
        break;
    case SinglesMoveType::Move2:
        index = 1;
        break;
    case SinglesMoveType::Move3:
        index = 2;
        break;
    case SinglesMoveType::Move4:
        index = 3;
        break;
    default:
        pbf_press_button(context, BUTTON_B, 20, 10);
        return false;
    }

    //  If we end up here consecutively too many times, the move is
    //  probably disabled. Select a different move.
    if (consecutive_move_select > 3){
        stream.log("Failed to select a move 3 times. Choosing a different move.", COLOR_RED);
//        pbf_press_dpad(context, DPAD_DOWN, 20, 40);
        index++;
        if (index >= 4){
            index = 0;
        }
        move.type = (SinglesMoveType)((uint8_t)SinglesMoveType::Move1 + index);
    }

    do{
        if (!move.terastallize){
            stream.log("Skipping Terastallization. Reason: Not requested.");
            break;
        }
        if (consecutive_move_select > 1){
            stream.log("Skipping Terastallization. Reason: Previously failed move select.");
            break;
        }

        stream.log("Attempting to Terastallize...");
        pbf_press_button(context, BUTTON_R, 20, 1 * TICKS_PER_SECOND);
    }while (false);

    context.wait_for_all_requests();

    if (move_select_menu.move_to_slot(stream, context, index)){
        pbf_press_button(context, BUTTON_A, 20, 10);
    }
    return true;
}


bool run_battle_menu(
    VideoStream& stream, ProControllerContext& context,
    NormalBattleMenuWatcher& battle_menu,
    const SinglesMoveEntry& move
){
    stream.log("Current Move Selection: " + move.to_str());
    switch (move.type){
    case SinglesMoveType::Move1:
    case SinglesMoveType::Move2:
    case SinglesMoveType::Move3:
    case SinglesMoveType::Move4:
        if (battle_menu.move_to_slot(stream, context, 0)){
            pbf_press_button(context, BUTTON_A, 20, 10);
            return true;
        }else{
            stream.log("Unable to move to battle slot.", COLOR_RED);
            pbf_mash_button(context, BUTTON_B, 125);
            return false;
        }
    case SinglesMoveType::Run:
        if (battle_menu.move_to_slot(stream, context, 3)){
            pbf_press_button(context, BUTTON_A, 20, 10);
            return true;
        }else{
            stream.log("Unable to move to run option.", COLOR_RED);
            pbf_mash_button(context, BUTTON_B, 125);
            return false;
        }
    }
    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "Invalid SinglesMoveType: " + std::to_string((int)move.type),
        stream
    );
}


//  Run a battle using the current Pokemon. Returns false if it fainted.
bool run_pokemon(
    VideoStream& stream, ProControllerContext& context,
    const std::vector<SinglesMoveEntry>& move_table,
    bool trainer_battle, bool& terastallized
){
    stream.log("Starting singles battle routine for one " + STRING_POKEMON + "...");

    size_t turn = 0;
    SinglesMoveEntry current_move{SinglesMoveType::Move1, false};
    if (!move_table.empty()){
        current_move = move_table[0];
    }

    size_t consecutive_timeouts = 0;
    size_t consecutive_move_select = 0;
//    bool battle_menu_seen = false;
    bool next_turn_on_battle_menu = false;
    while (true){

        NormalBattleMenuWatcher battle_menu(COLOR_RED);
        MoveSelectWatcher move_select_menu(COLOR_YELLOW);
        GradientArrowWatcher next_pokemon(COLOR_BLUE, GradientArrowType::RIGHT, {0.50, 0.51, 0.30, 0.10});
        SwapMenuWatcher swap_menu(COLOR_BLUE);
        AdvanceDialogWatcher dialog(COLOR_CYAN);
        OverworldWatcher overworld(stream.logger(), COLOR_GREEN);
        context.wait_for_all_requests();
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                for (size_t c = 0; c < 4; c++){
                    pbf_wait(context, 30 * TICKS_PER_SECOND);
                    pbf_press_button(context, BUTTON_B, 20, 0);
                }
            },
            {
                battle_menu,
                move_select_menu,
                next_pokemon,
                swap_menu,
                dialog,
                overworld,
            }
        );
        if (ret >= 0){
            consecutive_timeouts = 0;
        }
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:{
            stream.log("Detected battle menu.");
            consecutive_move_select = 0;
//            battle_menu_seen = true;

            //  If we enter here, we advance to the next turn.
            if (next_turn_on_battle_menu){
                stream.log("Detected battle menu. Turn: " + std::to_string(turn));
                turn++;
                //  Reset the move to the table entry in case we were forced to
                //  change moves due to move being unselectable.
                if (move_table.empty()){
                    current_move = SinglesMoveEntry{SinglesMoveType::Move1, false};
                }else if (turn < move_table.size()){
                    current_move = move_table[turn];
                }else{
                    current_move = move_table.back();
                }
//                next_turn_on_battle_menu = false;
            }

            if (terastallized){
                stream.log("Already used terastallization. Cannot use again.", COLOR_ORANGE);
                current_move.terastallize = false;
            }
            next_turn_on_battle_menu = run_battle_menu(stream, context, battle_menu, current_move);
            if (current_move.terastallize){
                terastallized = true;
            }
            continue;
        }
        case 1:{
            stream.log("Detected move select. Turn: " + std::to_string(turn));
            consecutive_move_select++;
            run_move_select(
                stream, context,
                move_select_menu,
                current_move,
                consecutive_move_select
            );
            continue;
        }
        case 2:
            if (trainer_battle){
                stream.log("Detected switch prompt...", COLOR_BLUE);
                pbf_press_button(context, BUTTON_B, 20, 10);
            }else{
                stream.log("Detected own " + STRING_POKEMON + " fainted...", COLOR_ORANGE);
                pbf_press_button(context, BUTTON_A, 20, 10);
            }
            continue;
        case 3:
            stream.log("Detected switch " + STRING_POKEMON + " menu...", COLOR_ORANGE);
            return false;
        case 4:
            stream.log("Detected advance dialog!", COLOR_ORANGE);
            return true;
        case 5:
            stream.log("Detected overworld!", COLOR_ORANGE);
            return true;
        default:
            consecutive_timeouts++;
            if (consecutive_timeouts == 3){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "No state detected after 6 minutes.",
                    stream
                );
            }
            stream.log("Unable to detect any state for 2 minutes. Mashing B...", COLOR_RED);
            pbf_mash_button(context, BUTTON_B, 250);
        }


    }



}







bool run_singles_battle(
    ProgramEnvironment& env,
    VideoStream& stream, ProControllerContext& context,
    SinglesAIOption& battle_AI,
    bool trainer_battle
){
    stream.log("Starting singles battle routine...");

    bool terastallized = false;
    size_t faint_counter = 0;
    while (true){
        bool win = run_pokemon(
            stream, context,
            battle_AI.MOVE_TABLES[std::min<size_t>(faint_counter, 5)].snapshot(),
            trainer_battle, terastallized
        );

        if (win){
            return true;
        }

        //  Find something to send out.
        faint_counter++;

        NormalBattleMenuWatcher battle_menu(COLOR_RED);
        AdvanceDialogWatcher dialog(COLOR_CYAN);
        SwapMenuWatcher swap_menu(COLOR_BLUE);
        while (true){
            context.wait_for_all_requests();
            int ret = wait_until(
                stream, context,
                std::chrono::seconds(120),
                {battle_menu, dialog, swap_menu}
            );
            context.wait_for(std::chrono::milliseconds(100));
            switch (ret){
            case 0:
                stream.log("Detected battle menu. Calling battle routine...");
                break;
            case 1:
                stream.log("Detected advance dialog. Battle has been expectedly won already.", COLOR_RED);
                return true;
            case 2:
                stream.log("Attempting to send in next " + STRING_POKEMON + "...");
                pbf_press_dpad(context, DPAD_DOWN, 20, 105);
                pbf_press_button(context, BUTTON_A, 20, 105);
                pbf_press_button(context, BUTTON_A, 20, 105);
                pbf_mash_button(context, BUTTON_B, 250);
                pbf_wait(context, 50);
                continue;
            default:
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Unable to send in a " + STRING_POKEMON + ".",
                    stream
                );
            }
            break;
        }
    }
}







}
}
}
