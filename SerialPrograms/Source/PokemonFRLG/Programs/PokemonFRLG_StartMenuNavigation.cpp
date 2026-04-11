/*  Start Menu Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Time.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_StartMenuDetector.h"
#include "PokemonFRLG/Inference/PokemonFRLG_SelectionArrowDetector.h"
#include "PokemonFRLG_StartMenuNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG {

void open_start_menu(ConsoleHandle& console, ProControllerContext& context){
    uint16_t errors = 0;

    while(true){
        if (errors > 5){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to open Start menu 5 times in a row.",
                console
            );
        }

        StartMenuWatcher start_menu(COLOR_RED);

        context.wait_for_all_requests();
        int ret = run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context) {
                pbf_press_button(context, BUTTON_PLUS, 200ms, 1800ms);
            },
            { start_menu }
        );

        if (ret < 0){
            console.log("Failed to open Start menu.");
            errors++;
            context.wait_for_all_requests();
            pbf_mash_button(context, BUTTON_B, 1000ms);
            continue;
        }
        
        console.log("Start menu opened.");
        return;
    }
}

void close_start_menu(ConsoleHandle& console, ProControllerContext& context){
    pbf_mash_button(context, BUTTON_B, 1000ms);
    // TODO: add overworld detection
    console.log("Start menu closed.");
}

namespace {

const ImageFloatBox MENU_ARROW_BOX(0.727692, 0.0523077, 0.0369231, 0.6438461);

bool move_cursor_impl(
    ConsoleHandle& console,
    ProControllerContext& context,
    const ImageFloatBox& search_box,
    int option_count,
    std::function<ImageFloatBox(int)> box_for_index,
    int destination_index
){
    SelectionArrowWatcher arrow(COLOR_RED, &console.overlay(), search_box);

    int ret = wait_until(
        console, context,
        std::chrono::seconds(2),
        { arrow }
    );

    if (ret < 0){
        console.log("Unable to detect selection arrow. Not moving cursor.", COLOR_RED);
        return false;
    }

    double detected_center_y = arrow.last_detected().y + arrow.last_detected().height / 2;
    int current_index = 0;
    double closest_dist = std::numeric_limits<double>::max();
    for (int i = 0; i < option_count; i++){
        ImageFloatBox pos_box = box_for_index(i);
        double pos_center_y = pos_box.y + pos_box.height / 2;
        double dist = std::abs(detected_center_y - pos_center_y);
        if (dist < closest_dist){
            closest_dist = dist;
            current_index = i;
        }
    }

    int forward = (destination_index - current_index + option_count) % option_count;
    int backward = (current_index - destination_index + option_count) % option_count;
    if (forward <= backward){
        for (int i = 0; i < forward; i++){
            pbf_press_dpad(context, DPAD_DOWN, 320ms, 400ms);
        }
    }
    else{
        for (int i = 0; i < backward; i++){
            pbf_press_dpad(context, DPAD_UP, 320ms, 400ms);
        }
    }
    context.wait_for_all_requests();

    return true;
}

} // anonymous namespace

bool move_cursor_to_position(ConsoleHandle& console, ProControllerContext& context, SelectionArrowPositionStartMenu destination){
    return move_cursor_impl(
        console, context,
        MENU_ARROW_BOX,
        START_MENU_OPTION_COUNT,
        [](int i){ return SelectionArrowDetector::arrow_box_for_position(static_cast<SelectionArrowPositionStartMenu>(i)); },
        static_cast<int>(destination)
    );
}

bool move_cursor_to_position(ConsoleHandle& console, ProControllerContext& context, SelectionArrowPositionSafariMenu destination){
    return move_cursor_impl(
        console, context,
        MENU_ARROW_BOX,
        SAFARI_START_MENU_OPTION_COUNT,
        [](int i){ return SelectionArrowDetector::arrow_box_for_position(static_cast<SelectionArrowPositionSafariMenu>(i)); },
        static_cast<int>(destination)
    );
}

void save_game_to_overworld(ConsoleHandle& console, ProControllerContext& context){

    WallClock start = current_time();
    while (true){
        context.wait_for_all_requests();

        if (current_time() - start > std::chrono::seconds(120)){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "save_game_to_overworld(): Unable to save game after 2 minutes.",
                console
            );
        }

        StartMenuWatcher start_menu = StartMenuWatcher(COLOR_RED);

        int ret = wait_until(
            console, context,
            std::chrono::seconds(1),
            {
                start_menu
            }
        );

        if (ret != 0){
            console.log("Start menu not detected. Attempting to open start menu.");
            pbf_press_button(context, BUTTON_X, 320ms, 320ms);
            context.wait_for_all_requests();
            continue;
        }

        if (!move_cursor_to_position(console, context, SelectionArrowPositionStartMenu::SAVE)){
            console.log("Unable to detect selection arrow. Attempting to open start menu again.");
            pbf_mash_button(context, BUTTON_B, 320ms);
            context.wait_for_all_requests();
            continue;
        }
        
        pbf_press_button(context, BUTTON_A, 320ms, 400ms);
        context.wait_for_all_requests();

        SelectionArrowWatcher save_arrow = SelectionArrowWatcher(
            COLOR_RED,
            &console.overlay(),
            SelectionArrowPositionConfirmationMenu::YES
        );

        int ret4 = wait_until(
            console, context,
            std::chrono::seconds(1),
            {
                save_arrow
            }
        );

        if (ret4 != 0){
            console.log("Unable to detect Save Arrow. Attempting to open start menu again.");
            pbf_mash_button(context, BUTTON_B, 320ms);
            context.wait_for_all_requests();
            continue;
        }

        console.log("Detected Save Arrow. Saving game.");
        pbf_press_button(context, BUTTON_A, 320ms, 200ms);
        context.wait_for_all_requests();

        SelectionArrowWatcher save_confirm_arrow = SelectionArrowWatcher(
            COLOR_RED,
            &console.overlay(),
            SelectionArrowPositionConfirmationMenu::YES
        );

        int ret5 = wait_until(
            console, context,
            std::chrono::seconds(2),
            {
                save_confirm_arrow
            }
        );

        if (ret5 != 0){
            console.log("Unable to detect Save Confirmation Arrow. Assuming this was the first save.");
            return;
        }

        console.log("Detected Save Confirmation Arrow. Saving game.");
        pbf_press_button(context, BUTTON_A, 320ms, 200ms);
        context.wait_for_all_requests();

        return;
    }

}

}
}
}
