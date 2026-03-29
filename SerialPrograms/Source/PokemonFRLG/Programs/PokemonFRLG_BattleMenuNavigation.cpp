/*  Battle Menu Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "cmath"
#include "limits"
#include "functional"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "PokemonFRLG/Inference/PokemonFRLG_BattleSelectionArrowDetector.h"
#include "PokemonFRLG_BattleMenuNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


namespace{

const ImageFloatBox BATTLE_MENU_ARROW_BOX(0.768, 0.868, 0.212, 0.159);

const int BATTLE_MENU_COLS = 2;

bool move_cursor_2d_impl(
    ConsoleHandle& console,
    ProControllerContext& context,
    const ImageFloatBox& search_box,
    int option_count,
    int cols,
    std::function<ImageFloatBox(int)> box_for_index,
    int destination_index
){
    BattleSelectionArrowWatcher arrow(COLOR_RED, &console.overlay(), search_box);

    int ret = wait_until(
        console, context,
        std::chrono::seconds(2),
        { arrow }
    );

    if (ret < 0){
        console.log("Unable to detect battle selection arrow. Not moving cursor.", COLOR_RED);
        return false;
    }

    double detected_cx = arrow.last_detected().x + arrow.last_detected().width / 2.0;
    double detected_cy = arrow.last_detected().y + arrow.last_detected().height / 2.0;

    int current_index = 0;
    double closest_dist = std::numeric_limits<double>::max();
    for (int i = 0; i < option_count; i++){
        ImageFloatBox pos_box = box_for_index(i);
        double cx = pos_box.x + pos_box.width / 2.0;
        double cy = pos_box.y + pos_box.height / 2.0;
        double dx = detected_cx - cx;
        double dy = detected_cy - cy;
        double dist = dx * dx + dy * dy;
        if (dist < closest_dist){
            closest_dist = dist;
            current_index = i;
        }
    }

    int current_row = current_index / cols;
    int current_col = current_index % cols;
    int dest_row = destination_index / cols;
    int dest_col = destination_index % cols;

    int row_diff = dest_row - current_row;
    for (int i = 0; i < std::abs(row_diff); i++){
        pbf_press_dpad(context, row_diff > 0 ? DPAD_DOWN : DPAD_UP, 320ms, 400ms);
    }

    int col_diff = dest_col - current_col;
    for (int i = 0; i < std::abs(col_diff); i++){
        pbf_press_dpad(context, col_diff > 0 ? DPAD_RIGHT : DPAD_LEFT, 320ms, 400ms);
    }

    context.wait_for_all_requests();
    return true;
}

} // anonymous namespace


bool move_cursor_to_option(ConsoleHandle& console, ProControllerContext& context, BattleMenuOption destination){
    return move_cursor_2d_impl(
        console, context,
        BATTLE_MENU_ARROW_BOX,
        BATTLE_MENU_OPTION_COUNT,
        BATTLE_MENU_COLS,
        [](int i){ return BattleSelectionArrowDetector::box_for_option(static_cast<BattleMenuOption>(i)); },
        static_cast<int>(destination)
    );
}

bool move_cursor_to_option(ConsoleHandle& console, ProControllerContext& context, SafariBattleMenuOption destination){
    return move_cursor_2d_impl(
        console, context,
        BATTLE_MENU_ARROW_BOX,
        SAFARI_BATTLE_MENU_OPTION_COUNT,
        BATTLE_MENU_COLS,
        [](int i){ return BattleSelectionArrowDetector::box_for_option(static_cast<SafariBattleMenuOption>(i)); },
        static_cast<int>(destination)
    );
}


}
}
}
