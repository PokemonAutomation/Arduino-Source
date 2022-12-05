/*  Sandwich Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/PokemonSV_PicnicDetector.h"
#include "PokemonSV/Inference/PokemonSV_GradientArrowDetector.h"
#include "PokemonSV/Inference/PokemonSV_SandwichRecipeDetector.h"
#include "PokemonSV_SandwichRoutines.h"

#include <algorithm>
#include <sstream>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

bool enter_sandwich_recipe_list(ConsoleHandle& console, BotBaseContext& context){
    context.wait_for_all_requests();
    console.log("Opening sandwich menu at picnic table.");

    // Firt, try pressing button A to bring up the menu to make sandwich
    pbf_press_button(context, BUTTON_A, 20, 80);

    bool opened_table_menu = false;
    while(true){
        context.wait_for_all_requests();

        PicnicWatcher picnic_watcher;
        GradientArrowWatcher sandwich_arrow(COLOR_YELLOW, GradientArrowType::RIGHT, {0.551, 0.311, 0.310, 0.106});
        GradientArrowWatcher recipe_arrow(COLOR_YELLOW, GradientArrowType::DOWN, {0.103, 0.074, 0.068, 0.085});
        AdvanceDialogWatcher dialog_watcher(COLOR_RED);

        const int ret = wait_until(
            console, context,
            std::chrono::seconds(30),
            {picnic_watcher, sandwich_arrow, recipe_arrow, dialog_watcher}
        );
        switch (ret){
        case 0:
            console.log("Detected picnic. Maybe button A press dropped.");
            pbf_press_button(context, BUTTON_A, 20, 80);
            continue;
        case 1:
            console.log("Detected \"make a sandwich\" menu item selection arrrow.");
            console.overlay().add_log("Open sandwich recipes", COLOR_WHITE);
            opened_table_menu = true;
            pbf_press_button(context, BUTTON_A, 20, 100);
            continue;
        case 2:
            console.log("Detected recipe selection arrow.");
            context.wait_for(std::chrono::seconds(1)); // wait one second to make sure the menu is fully loaded.
            return true;
        case 3:
            console.log("Detected advance dialog.");
            if (opened_table_menu){
                console.log("Advance dialog after \"make a sandwich\" menu item. No ingredients.", COLOR_RED);
                console.overlay().add_log("No ingredient!", COLOR_RED);
                return false;
            }
            pbf_press_button(context, BUTTON_A, 20, 80);
            continue;
        default:
            throw OperationFailedException(console.logger(), "enter_sandwich_recipe_list(): No recognized state after 60 seconds.");
        }
    }
}


bool select_sandwich_recipe(ConsoleHandle& console, BotBaseContext& context, size_t target_sandwich_ID){
    context.wait_for_all_requests();
    console.log("Choosing sandwich recipe: " + std::to_string(target_sandwich_ID));
    console.overlay().add_log("Search recipe " + std::to_string(target_sandwich_ID), COLOR_WHITE);

    SandwichRecipeNumberDetector recipe_detector(console.logger());
    SandwichRecipeSelectionWatcher selection_watcher;

    VideoOverlaySet overlay_set(console.overlay());
    recipe_detector.make_overlays(overlay_set);
    selection_watcher.make_overlays(overlay_set);

    bool found_recipe = false;
    int max_move_down_list_attempts = 100; // There are 151 total recipes, so 76 rows.
    for(int move_down_list_attempt = 0; move_down_list_attempt < max_move_down_list_attempts; move_down_list_attempt++) {
        context.wait_for_all_requests();

        auto snapshot = console.video().snapshot();
        size_t recipe_IDs[6] = {0, 0, 0, 0, 0, 0};
        recipe_detector.detect_recipes(snapshot, recipe_IDs);
        {
            std::ostringstream os;
            os << "Recipe IDs detected: ";
            for(int i = 0; i < 6; i++){
                os << recipe_IDs[i] << ", ";
            }
            console.log(os.str());
        }

        size_t min_ID = 300;
        for(int i = 0; i < 6; i++){
            if (recipe_IDs[i] > 0 && recipe_IDs[i] < min_ID){
                min_ID = recipe_IDs[i];
            }
        }
        if (min_ID == 300){
            min_ID = 0; // set case of no recipe ID detected to be 0 min_ID
        }
        size_t max_ID = *std::max_element(recipe_IDs, recipe_IDs+6);
        console.log("min, max IDs " + std::to_string(min_ID) + ", " + std::to_string(max_ID));

        if (0 < min_ID && min_ID <= target_sandwich_ID && target_sandwich_ID <= max_ID){
            // target is in this page!
            
            int target_cell = -1;
            for(int i = 0; i < 6; i++){
                if (recipe_IDs[i] == target_sandwich_ID){
                    target_cell = i;
                    break;
                }
            }
            if (target_cell == -1){ // not targe recipe found in this page, probably not enough ingredients
                console.log("Not enough ingredients for target recipe.", COLOR_RED);
                console.overlay().add_log("Not enough ingredients", COLOR_RED);
                return false;
            }

            console.log("found recipe in the current page, cell " + std::to_string(target_cell));
            
            int ret = wait_until(console, context, std::chrono::seconds(10), {selection_watcher});
            int selected_cell = selection_watcher.selected_recipe_cell();
            if (ret < 0 || selected_cell < 0){
                throw OperationFailedException(console.logger(), "select_sandwich_recipe(): Cannot detect recipe selection arrow.");
            }

            console.log("Current selected cell " + std::to_string(selected_cell));

            if (target_cell == selected_cell){
                // Selected target recipe!
                console.log("Found recipe at cell " + std::to_string(selected_cell));
                console.overlay().add_log("Found recipe", COLOR_WHITE);
                found_recipe = true;
                break;
            }
            else if (target_cell == selected_cell + 1){
                console.log("Move to the right column.");
                // Target is in a different column
                // Move cursor right.
                pbf_press_dpad(context, DPAD_RIGHT, 10, 50);
                continue;
            }
            // else, continue moving down the list
        }

        // target sandwich recipe is still below the current displayed recipes.
        // Move down the list.
        pbf_press_dpad(context, DPAD_DOWN, 10, 50);
    } // end for moving down the recipe list

    overlay_set.clear();

    if (found_recipe){
        // Press A to enter the pick selection 
        pbf_press_button(context, BUTTON_A, 30, 100);
        context.wait_for_all_requests();

        GradientArrowWatcher pick_selection(COLOR_YELLOW, GradientArrowType::RIGHT, {0.004, 0.162, 0.100, 0.092});
        while(true){
            int ret = wait_until(console, context, std::chrono::seconds(3),
                {selection_watcher, pick_selection});
            
            if (ret == 0){
                console.log("Detected recipe selection. Dropped Button A?");
                pbf_press_button(context, BUTTON_A, 30, 100);
                continue;
            } else if (ret == 1){
                console.log("Detected pick selection.");
                pbf_press_button(context, BUTTON_A, 30, 100);
                continue;
            } else{
                console.log("Entered sandwich minigame.");
                break;
            }
        }
        return true;
    }

    // we cannot find the receipt
    console.log("Max list travese attempt reached. Target recipe not found", COLOR_RED);
    console.overlay().add_log("Recipe not found", COLOR_RED);

    return false;
}

}
}
}
