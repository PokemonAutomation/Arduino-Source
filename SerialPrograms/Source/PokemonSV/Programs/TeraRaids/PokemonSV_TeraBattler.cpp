/*  Tera Battler
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <mutex>
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV/Inference/Battles/PokemonSV_TeraBattleMenus.h"
#include "PokemonSV/Inference/Battles/PokemonSV_TeraRewardsMenu.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV_TeraBattler.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


//  If two Switches send commands at exactly the same time, it may cause the
//  Switches to desync and fork the battle state. So we use this lock prevent
//  any two Switches from sending commands too close to each other.
std::mutex tera_battle_throttle_lock;


enum class BattleMenuResult{
    RESUME_CURRENT_TURN,
    RESUME_ADVANCE_TURN,
    BATTLE_WON,
    BATTLE_LOST,
};
BattleMenuResult run_battle_menu(
    VideoStream& stream, ProControllerContext& context,
    TeraBattleMenuDetector& battle_menu,
    TeraCatchWatcher& catch_menu,
    OverworldWatcher& overworld,
    const TeraMoveEntry& move
){
    stream.log("Current Move Selection: " + move.to_str());
    switch (move.type){
    case TeraMoveType::Wait:{
        int ret = run_until<ProControllerContext>(
            stream, context,
            [&](ProControllerContext& context){
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
        if (battle_menu.move_to_slot(stream, context, 0)){
            pbf_press_button(context, BUTTON_A, 20, 10);
        }
        break;
    case TeraMoveType::Cheer_AllOut:
    case TeraMoveType::Cheer_HangTough:
    case TeraMoveType::Cheer_HealUp:
        if (battle_menu.move_to_slot(stream, context, 1)){
            pbf_press_button(context, BUTTON_A, 20, 10);
        }
        break;
    }
    return BattleMenuResult::RESUME_CURRENT_TURN;
}
bool run_cheer_select(
    VideoStream& stream, ProControllerContext& context,
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
    if (cheer_select_menu.move_to_slot(stream, context, index)){
        std::lock_guard<std::mutex> lg(tera_battle_throttle_lock);
        pbf_press_button(context, BUTTON_A, 20, 40);
        context.wait_for_all_requests();
    }
    return true;
}
bool run_move_select(
    VideoStream& stream, ProControllerContext& context,
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
        stream.log("Failed to select a move 3 times. Choosing a different move.", COLOR_RED);
//        pbf_press_dpad(context, DPAD_DOWN, 20, 40);
        index++;
        if (index >= 4){
            index = 0;
        }
        move.type = (TeraMoveType)((uint8_t)TeraMoveType::Move1 + index);
    }

    do{
        if (!battle_AI.TRY_TO_TERASTILLIZE){
            stream.log("Skipping Terastallization. Reason: Disabled by settings.");
            break;
        }
        if (consecutive_move_select > 1){
            stream.log("Skipping Terastallization. Reason: Previously failed move select.");
            break;
        }
        if (!terastallizing.detect(stream.video().snapshot())){
            stream.log("Skipping Terastallization. Reason: Not ready.");
            break;
        }

        stream.log("Attempting to Terastallize...");
        pbf_press_button(context, BUTTON_R, 20, 4 * TICKS_PER_SECOND);
    }while (false);

    if (move_select_menu.move_to_slot(stream, context, index)){
        pbf_press_button(context, BUTTON_A, 20, 10);
    }
    return true;
}
bool run_target_select(
    VideoStream& stream, ProControllerContext& context,
    TeraTargetSelectDetector& target_select_menu,
    TeraMoveEntry& move
){
    switch (move.type){
    case TeraMoveType::Move1:
    case TeraMoveType::Move2:
    case TeraMoveType::Move3:
    case TeraMoveType::Move4:{
        target_select_menu.move_to_slot(stream, context, (uint8_t)move.target);
        std::lock_guard<std::mutex> lg(tera_battle_throttle_lock);
        pbf_press_button(context, BUTTON_A, 20, 40);
        context.wait_for_all_requests();
        return true;
    }
    default:
        pbf_press_button(context, BUTTON_B, 20, 10);
        return false;
    }
}




bool run_tera_battle(
    ProgramEnvironment& env,
    VideoStream& stream, ProControllerContext& context,
    TeraAIOption& battle_AI
){
    stream.log("Starting tera battle routine...");

    size_t turn = 0;
    std::vector<TeraMoveEntry> move_table = battle_AI.MOVE_TABLE.snapshot();
    TeraMoveEntry current_move{TeraMoveType::Move1, 0, TeraTarget::Opponent};
    if (!move_table.empty()){
        current_move = move_table[0];
    }

    size_t consecutive_timeouts = 0;
    size_t consecutive_move_select = 0;
    bool battle_menu_seen = false;
    bool next_turn_on_battle_menu = false;
    while (true){
        // Warning, this terastallizing detector isn't used in the wait_until() below.
        TerastallizingDetector terastallizing(COLOR_ORANGE);
        VideoOverlaySet overlay_set(stream.overlay());
        terastallizing.make_overlays(overlay_set);

        TeraBattleMenuWatcher battle_menu(COLOR_RED);
        CheerSelectWatcher cheer_select_menu(COLOR_YELLOW);
        MoveSelectWatcher move_select_menu(COLOR_YELLOW);
        TargetSelectWatcher target_select_menu(COLOR_CYAN);
        TeraRewardsMenuWatcher rewards_menu(
            COLOR_BLUE,
            //  This can false positive against the back (B) button while in the
            //  lobby. So don't trigger this unless we see the battle menu first.
            //  At the same time, there's a possibility that we miss the battle
            //  menu if the raid is won before it even loads. And this can only
            //  happen if the raid was uncatchable to begin with.
            std::chrono::seconds(battle_menu_seen ? 5 : 180)
        );
#if 0
        WhiteButtonWatcher next_button(
            COLOR_CYAN,
            WhiteButton::ButtonA,
            {0.8, 0.93, 0.2, 0.07},
            WhiteButtonWatcher::FinderType::PRESENT,
            //  This can false positive against the back (B) button while in the
            //  lobby. So don't trigger this unless we see the battle menu first.
            //  At the same time, there's a possibility that we miss the battle
            //  menu if the raid is won before it even loads. And this can only
            //  happen if the raid was uncatchable to begin with.
            std::chrono::seconds(battle_menu_seen ? 5 : 180)
        );
#endif
        TeraCatchWatcher catch_menu(COLOR_BLUE);
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
                cheer_select_menu,
                move_select_menu,
                target_select_menu,
//                next_button,
                rewards_menu,
                catch_menu,
                overworld,
            }
        );
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:{
            stream.log("Detected battle menu.");
            battle_menu_seen = true;

            //  If we enter here, we advance to the next turn.
            if (next_turn_on_battle_menu){
                stream.log("Detected battle menu. Turn: " + std::to_string(turn));
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

            stream.log("Current Move Selection: " + current_move.to_str());
            BattleMenuResult battle_menu_result = run_battle_menu(
                stream, context,
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
                stream.log("Detected a win!", COLOR_BLUE);
                pbf_mash_button(context, BUTTON_B, 30);
                return true;
            case BattleMenuResult::BATTLE_LOST:
                stream.log("Detected a loss!", COLOR_ORANGE);
                return false;
            }
        }
        case 1:{
            stream.log("Detected cheer select. Turn: " + std::to_string(turn));
            if (run_cheer_select(stream, context, cheer_select_menu, current_move)){
                next_turn_on_battle_menu = true;
            }
            continue;
        }
        case 2:{
            stream.log("Detected move select. Turn: " + std::to_string(turn));
            consecutive_move_select++;
            run_move_select(
                stream, context,
                battle_AI,
                terastallizing,
                move_select_menu,
                current_move,
                consecutive_move_select
            );
            continue;
        }
        case 3:
            stream.log("Detected target select. Turn: " + std::to_string(turn));
            consecutive_move_select = 0;
            if (run_target_select(stream, context, target_select_menu, current_move)){
                next_turn_on_battle_menu = true;
            }
            continue;
        case 4:
            stream.log("Detected item rewards menu!", COLOR_BLUE);
            pbf_mash_button(context, BUTTON_B, 30);
            return true;
        case 5:
            stream.log("Detected catch menu!", COLOR_BLUE);
            pbf_mash_button(context, BUTTON_B, 30);
            return true;
        case 6:
            stream.log("Detected a loss!", COLOR_ORANGE);
            return false;
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

    return false;
}









}
}
}
