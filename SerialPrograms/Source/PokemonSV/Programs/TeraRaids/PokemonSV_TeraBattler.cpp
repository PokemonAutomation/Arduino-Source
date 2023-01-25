/*  Tera Battler
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Inference/Battles/PokemonSV_BattleMenuDetector.h"
#include "PokemonSV/Inference/PokemonSV_OverworldDetector.h"
#include "PokemonSV_TeraBattler.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



bool run_tera_battle(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    BotBaseContext& context,
    EventNotificationOption& error_notification,
    TeraAIOption& battle_AI
){

    size_t turn = 0;
    std::vector<TeraMoveEntry> move_table = battle_AI.MOVE_TABLE.snapshot();
    TeraMoveEntry current_move{TeraMoveType::Move1, 0, TeraTarget::Opponent};
    if (!move_table.empty()){
        current_move = move_table[0];
    }

    size_t consecutive_timeouts = 0;
    size_t consecutive_move_select = 0;
    bool next_turn_on_battle_menu = false;
    while (true){
        // warning, this terastallizing detector isn't used in the wait_until below
        TerastallizingDetector terastallizing(COLOR_ORANGE);
        VideoOverlaySet overlay_set(console);
        terastallizing.make_overlays(overlay_set);

        TeraBattleMenuWatcher battle_menu(COLOR_RED);
        MoveSelectWatcher move_select_menu(COLOR_YELLOW);
        TargetSelectWatcher target_select_menu(COLOR_CYAN);
        TeraCatchWatcher catch_menu(COLOR_BLUE);
        OverworldWatcher overworld(COLOR_GREEN);
        context.wait_for_all_requests();
        int ret = wait_until(
            console, context,
            std::chrono::seconds(120),
            {
                battle_menu,
                move_select_menu,
                target_select_menu,
                catch_menu,
                overworld,
            }
        );
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:{
            console.log("Detected battle menu.");
            if (next_turn_on_battle_menu){
                console.log("Detected battle menu. Turn: " + std::to_string(turn));
                turn++;
                //  Reset the move to the table entry in case we were forced to
                //  change moves due to move being unselectable.
                if (move_table.empty()){
                    current_move = TeraMoveEntry{TeraMoveType::Move1, 0, TeraTarget::Opponent};
                }else if (turn < move_table.size()){
                    current_move = move_table.back();
                }
                next_turn_on_battle_menu = false;
            }
            console.log("Current Move Selection: " + current_move.to_str());
            switch (current_move.type){
            case TeraMoveType::Wait:
                context.wait_for(std::chrono::seconds(current_move.seconds));
                next_turn_on_battle_menu = true;
                continue;
            case TeraMoveType::Move1:
            case TeraMoveType::Move2:
            case TeraMoveType::Move3:
            case TeraMoveType::Move4:
                if (battle_menu.move_to_slot(console, context, 0)){
                    pbf_press_button(context, BUTTON_A, 20, 10);
                }
                continue;
            case TeraMoveType::Cheer_AllOut:
                if (!battle_menu.move_to_slot(console, context, 1)){
                    continue;
                }
                pbf_press_button(context, BUTTON_A, 20, 105);
                pbf_press_button(context, BUTTON_A, 20, 105);
                next_turn_on_battle_menu = true;
                continue;
            case TeraMoveType::Cheer_HangTough:
                if (!battle_menu.move_to_slot(console, context, 1)){
                    continue;
                }
                pbf_press_button(context, BUTTON_A, 20, 105);
                pbf_press_dpad(context, DPAD_DOWN, 20, 30);
                pbf_press_button(context, BUTTON_A, 20, 105);
                next_turn_on_battle_menu = true;
                continue;
            case TeraMoveType::Cheer_HealUp:
                if (!battle_menu.move_to_slot(console, context, 1)){
                    continue;
                }
                pbf_press_button(context, BUTTON_A, 20, 105);
                pbf_press_dpad(context, DPAD_UP, 20, 30);
                pbf_press_button(context, BUTTON_A, 20, 105);
                next_turn_on_battle_menu = true;
                continue;
            }
            continue;
        }
        case 1:{
            console.log("Detected move select. Turn: " + std::to_string(turn));
            consecutive_move_select++;

            uint8_t index = 0;
            switch (current_move.type){
            case TeraMoveType::Move1:
                index = 0;
                break;
            case TeraMoveType::Move2:
                index = 1;
                break;
            case TeraMoveType::Move3:
                index = 2;
                break;
            case TeraMoveType::Move4:
                index = 3;
                break;
            default:
                pbf_press_button(context, BUTTON_B, 20, 10);
                continue;
            }

            //  If we end up here consecutively too many times, the move is
            //  probably disabled. Select a different move.
            if (consecutive_move_select > 3){
                console.log("Failed to select a move 3 times. Choosing a different move.", COLOR_RED);
//                pbf_press_dpad(context, DPAD_DOWN, 20, 40);
                index++;
                if (index >= 4){
                    index = 0;
                }
                current_move.type = (TeraMoveType)((uint8_t)TeraMoveType::Move1 + index);
            }
            if (terastallizing.detect(console.video().snapshot())){
                console.log("Terastallization: Available");
                if (battle_AI.TRY_TO_TERASTILLIZE){
                    pbf_press_button(context, BUTTON_R, 20, 4 * TICKS_PER_SECOND);
                }
            }else{
                console.log("Terastallization: Not Available");
            }
            if (move_select_menu.move_to_slot(console, context, index)){
                pbf_press_button(context, BUTTON_A, 20, 10);
            }
            continue;
        }
        case 2:
            console.log("Detected target select. Turn: " + std::to_string(turn));
            consecutive_move_select = 0;
            switch (current_move.type){
            case TeraMoveType::Move1:
            case TeraMoveType::Move2:
            case TeraMoveType::Move3:
            case TeraMoveType::Move4:
                target_select_menu.move_to_slot(console, context, (uint8_t)current_move.target);
                pbf_press_button(context, BUTTON_A, 20, 10);
                next_turn_on_battle_menu = true;
                continue;
            default:
                pbf_press_button(context, BUTTON_B, 20, 10);
                continue;
            }
        case 3:
            console.log("Detected a win!", COLOR_BLUE);
            pbf_mash_button(context, BUTTON_B, 30);
            return true;
        case 4:
            console.log("Detected a loss!", COLOR_ORANGE);
            return false;
        default:
            consecutive_timeouts++;
            if (consecutive_timeouts == 3){
                dump_image_and_throw_recoverable_exception(
                    env, console, error_notification,
                    "NoStateFound",
                    "No state detected after 6 minutes."
                );
            }
            console.log("Unable to detect any state for 2 minutes. Mashing B...", COLOR_RED);
            pbf_mash_button(context, BUTTON_B, 250);
        }
    }

    return false;
}









}
}
}
