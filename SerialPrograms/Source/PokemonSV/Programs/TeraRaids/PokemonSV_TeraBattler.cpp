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


enum class BattleMenuResult{
    RESUME_CURRENT_TURN,
    RESUME_ADVANCE_TURN,
    BATTLE_WON,
    BATTLE_LOST,
};
BattleMenuResult run_battle_menu(
    ConsoleHandle& console, BotBaseContext& context,
    const std::vector<TeraMoveEntry>& move_table,
    TeraBattleMenuDetector& battle_menu,
    TeraCatchWatcher& catch_menu,
    OverworldWatcher& overworld,
    TeraMoveEntry& move
){
    console.log("Current Move Selection: " + move.to_str());
    switch (move.type){
    case TeraMoveType::Wait:{
        int ret = run_until(
            console, context,
            [&](BotBaseContext& context){
                pbf_mash_button(context, BUTTON_B, move.seconds * TICKS_PER_SECOND);
            },
            {catch_menu, overworld}
        );
        switch (ret){
        case 0: return BattleMenuResult::BATTLE_WON;
        case 1: return BattleMenuResult::BATTLE_LOST;
        }
        return BattleMenuResult::RESUME_ADVANCE_TURN;
    }
    case TeraMoveType::Move1:
    case TeraMoveType::Move2:
    case TeraMoveType::Move3:
    case TeraMoveType::Move4:
        if (battle_menu.move_to_slot(console, context, 0)){
            pbf_press_button(context, BUTTON_A, 20, 10);
        }
        break;
    case TeraMoveType::Cheer_AllOut:
    case TeraMoveType::Cheer_HangTough:
    case TeraMoveType::Cheer_HealUp:
        if (battle_menu.move_to_slot(console, context, 1)){
            pbf_press_button(context, BUTTON_A, 20, 10);
        }
        break;
    }
    return BattleMenuResult::RESUME_CURRENT_TURN;
}
bool run_cheer_select(
    ConsoleHandle& console, BotBaseContext& context,
    CheerSelectDetector& cheer_select_menu,
    TeraMoveEntry& move
){
    uint8_t index = 0;
    switch (move.type){
    case TeraMoveType::Cheer_AllOut:
        index = 0;
        break;
    case TeraMoveType::Cheer_HangTough:
        index = 1;
        break;
    case TeraMoveType::Cheer_HealUp:
        index = 2;
        break;
    default:
        pbf_press_button(context, BUTTON_B, 20, 10);
        return false;
    }
    if (cheer_select_menu.move_to_slot(console, context, index)){
        pbf_press_button(context, BUTTON_A, 20, 10);
    }
    return true;
}
bool run_move_select(
    ConsoleHandle& console, BotBaseContext& context,
    TeraAIOption& battle_AI,
    TerastallizingDetector& terastallizing,
    MoveSelectWatcher& move_select_menu,
    TeraMoveEntry& move, size_t consecutive_move_select
){
    uint8_t index = 0;
    switch (move.type){
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
        return false;
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
        move.type = (TeraMoveType)((uint8_t)TeraMoveType::Move1 + index);
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
    return true;
}
bool run_target_select(
    ConsoleHandle& console, BotBaseContext& context,
    TargetSelectDetector& target_select_menu,
    TeraMoveEntry& move
){
    switch (move.type){
    case TeraMoveType::Move1:
    case TeraMoveType::Move2:
    case TeraMoveType::Move3:
    case TeraMoveType::Move4:
        target_select_menu.move_to_slot(console, context, (uint8_t)move.target);
        pbf_press_button(context, BUTTON_A, 20, 10);
        return true;
    default:
        pbf_press_button(context, BUTTON_B, 20, 10);
        return false;
    }
}




bool run_tera_battle(
    ProgramEnvironment& env,
    ConsoleHandle& console, BotBaseContext& context,
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
        // Warning, this terastallizing detector isn't used in the wait_until() below.
        TerastallizingDetector terastallizing(COLOR_ORANGE);
        VideoOverlaySet overlay_set(console);
        terastallizing.make_overlays(overlay_set);

        TeraBattleMenuWatcher battle_menu(COLOR_RED);
        CheerSelectWatcher cheer_select_menu(COLOR_YELLOW);
        MoveSelectWatcher move_select_menu(COLOR_YELLOW);
        TargetSelectWatcher target_select_menu(COLOR_CYAN);
        TeraCatchWatcher catch_menu(COLOR_BLUE);
        OverworldWatcher overworld(COLOR_GREEN);
        context.wait_for_all_requests();
        int ret = run_until(
            console, context,
            [](BotBaseContext& context){
                for (size_t c = 0; c < 4; c++){
                    pbf_wait(context, 30 * TICKS_PER_SECOND);
                    pbf_press_button(context, BUTTON_B, 20, 0);
                }
            },
            {
                battle_menu,
                cheer_select_menu,
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

            //  If we enter here, we advance to the next turn.
            if (next_turn_on_battle_menu){
                console.log("Detected battle menu. Turn: " + std::to_string(turn));
                turn++;
                //  Reset the move to the table entry in case we were forced to
                //  change moves due to move being unselectable.
                if (move_table.empty()){
                    current_move = TeraMoveEntry{TeraMoveType::Move1, 0, TeraTarget::Opponent};
                }else if (turn < move_table.size()){
                    current_move = move_table[turn];
                }else{
                    current_move = move_table.back();
                }
                next_turn_on_battle_menu = false;
            }

            console.log("Current Move Selection: " + current_move.to_str());
            BattleMenuResult battle_menu_result = run_battle_menu(
                console, context,
                move_table,
                battle_menu,
                catch_menu,
                overworld,
                current_move
            );
            switch (battle_menu_result){
            case BattleMenuResult::RESUME_CURRENT_TURN:
                continue;
            case BattleMenuResult::RESUME_ADVANCE_TURN:
                next_turn_on_battle_menu = true;
                continue;
            case BattleMenuResult::BATTLE_WON:
                console.log("Detected a win!", COLOR_BLUE);
                pbf_mash_button(context, BUTTON_B, 30);
                return true;
            case BattleMenuResult::BATTLE_LOST:
                console.log("Detected a loss!", COLOR_ORANGE);
                return false;
            }
        }
        case 1:{
            console.log("Detected cheer select. Turn: " + std::to_string(turn));
            if (run_cheer_select(console, context, cheer_select_menu, current_move)){
                next_turn_on_battle_menu = true;
            }
            continue;
        }
        case 2:{
            console.log("Detected move select. Turn: " + std::to_string(turn));
            consecutive_move_select++;
            run_move_select(
                console, context,
                battle_AI,
                terastallizing,
                move_select_menu,
                current_move,
                consecutive_move_select
            );
            continue;
        }
        case 3:
            console.log("Detected target select. Turn: " + std::to_string(turn));
            consecutive_move_select = 0;
            if (run_target_select(console, context, target_select_menu, current_move)){
                next_turn_on_battle_menu = true;
            }
            continue;
        case 4:
            console.log("Detected a win!", COLOR_BLUE);
            pbf_mash_button(context, BUTTON_B, 30);
            return true;
        case 5:
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
