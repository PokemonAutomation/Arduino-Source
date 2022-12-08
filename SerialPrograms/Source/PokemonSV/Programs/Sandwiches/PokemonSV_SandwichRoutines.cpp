/*  Sandwich Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_ScalarButtons.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/PokemonSV_PicnicDetector.h"
#include "PokemonSV/Inference/PokemonSV_GradientArrowDetector.h"
#include "PokemonSV/Inference/PokemonSV_SandwichHandDetector.h"
#include "PokemonSV/Inference/PokemonSV_SandwichRecipeDetector.h"
#include "PokemonSV_SandwichRoutines.h"


#include <cmath>
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

namespace {

// expand the hand bounding box so that the hand watcher can pick the hand in the next iteration
ImageFloatBox expand_box(const ImageFloatBox& box){
    return ImageFloatBox(box.x - box.width, box.y - box.height, box.width*3, box.height*3);
}

std::string box_to_string(const ImageFloatBox& box){
    std::ostringstream os;
    os << "(" << box.x << ", " << box.y << ", " << box.width << ", " << box.height << ")";
    return os.str();
}

ImageFloatBox move_sandwich_hand(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    BotBaseContext& context,
    SandwichHandType hand_type,
    bool pressing_A,
    const ImageFloatBox& start_box,
    const ImageFloatBox& end_box
){
    context.wait_for_all_requests();
    env.log(std::string("Start moving sandwich hand: ") + ((hand_type == SandwichHandType::FREE) ? "FREE" : "GRABBING")
        + " start box " + box_to_string(start_box) + " end box " + box_to_string(end_box));

    SandwichHandWatcher hand_watcher(hand_type, start_box);

    // A session that creates a new thread to send button commands to controller
    AsyncCommandSession move_session(context, console.logger(), env.realtime_dispatcher(), console.botbase());
    
    if (pressing_A){
        move_session.dispatch([&](BotBaseContext& context){
            ssf_press_button(context, BUTTON_A, 0, 3000);
        });
    }

    const std::pair<double, double> target_loc(end_box.x + end_box.width/2, end_box.y + end_box.height/2);

    std::pair<double, double> last_loc(-1, -1);
    std::pair<double, double> speed(-1, -1);
    WallClock cur_time, last_time;
    VideoOverlaySet overlay_set(console.overlay());

    while(true){
        int ret = wait_until(console, context, std::chrono::seconds(5), {hand_watcher});
        if (ret < 0){
            throw OperationFailedException(console.logger(), "move_sandwich_hand(): Cannot detect hand.");
        }

        auto cur_loc = hand_watcher.location();
        console.log("Hand location: " + std::to_string(cur_loc.first) + ", " + std::to_string(cur_loc.second));
        cur_time = current_time();

        const double hand_width = 0.071, hand_height = 0.106;
        const ImageFloatBox hand_bb(cur_loc.first - hand_width/2, cur_loc.second - hand_height/2, hand_width, hand_height);
        const ImageFloatBox expanded_hand_bb = expand_box(hand_bb);
        console.log("Hand bb " + box_to_string(hand_bb) + " expanded " + box_to_string(expanded_hand_bb));
        hand_watcher.change_box(expanded_hand_bb);

        overlay_set.clear();
        overlay_set.add(COLOR_RED, hand_bb);
        overlay_set.add(COLOR_BLUE, expanded_hand_bb);

        std::pair<double, double> dif(target_loc.first - cur_loc.first, target_loc.second - cur_loc.second);
        console.log("float diff to target: " + std::to_string(dif.first) + ", " + std::to_string(dif.second));
        if (std::fabs(dif.first) < end_box.width/2 && std::fabs(dif.second) < end_box.height/2){
            console.log("Free hand reached target.");
            move_session.stop_session_and_rethrow(); // Stop the commands
            if (hand_type == SandwichHandType::GRABBING){
                // wait for some time to let hand release ingredient
                context.wait_for(std::chrono::milliseconds(100));
            }
            return hand_bb;
        }

        // Assume screen width is 16.0, then the screen height is 9.0
        std::pair<double, double> real_dif(dif.first * 16, dif.second * 9);
        double distance = std::sqrt(real_dif.first * real_dif.first + real_dif.second * real_dif.second);
        console.log("scaled diff to target: " + std::to_string(real_dif.first) + ", " + std::to_string(real_dif.second)
            + " distance " + std::to_string(distance));

        // Build a P-D controller!

        // We assume for a screen distance of 4 (1/4 of the width), we can use max joystick push, 128.
        // So for distance of value 1.0, we multiply by 32 to get joystick push
        double target_joystick_push = std::min(distance * 32, 128.0);

        std::pair<double, double> push(real_dif.first * target_joystick_push / distance, real_dif.second * target_joystick_push / distance);
        console.log("push force " + std::to_string(push.first) + ", " + std::to_string(push.second));

        if (last_loc.first < 0){
            speed = std::make_pair(0.0, 0.0);
        } else {
            std::chrono::microseconds time = cur_time - last_time;
            double time_s = time.count() / 1000000.0;
            std::pair<double, double> moved((cur_loc.first - last_loc.first) * 16, (cur_loc.second - last_loc.second) * 9);

            double damping_factor = 0.0;
            double damping_multiplier = (-1.0) * damping_factor / time_s;
            std::pair<double, double> damped_push_offset(moved.first * damping_multiplier, moved.second * damping_multiplier);

            push.first += damped_push_offset.first;
            push.second += damped_push_offset.second;
        }

        uint8_t joystick_x = (uint8_t) std::max(std::min(int(push.first + 0.5) + 128, 255), 0);
        uint8_t joystick_y = (uint8_t) std::max(std::min(int(push.second + 0.5) + 128, 255), 0);
        console.log("joystick push " + std::to_string(joystick_x) + ", " + std::to_string(joystick_y));

        // Dispatch a new series of commands that overwrites the last ones
        move_session.dispatch([&](BotBaseContext& context){
            if (pressing_A){
                ssf_press_button(context, BUTTON_A, 0, 300);
            }
            ssf_press_joystick(context, true, joystick_x, joystick_y, 0, 100);
        });
        
        console.log("Moved joystick");

        last_loc = cur_loc;
        last_time = cur_time;
        context.wait_for(std::chrono::milliseconds(5));
    }
}

} // end anonymous namesapce

void make_great_peanut_butter_sandwich(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){

    const ImageFloatBox initial_box{0.440, 0.455, 0.112, 0.179};
    const ImageFloatBox ingredient_box{0.455, 0.090, 0.090, 0.089};

    const ImageFloatBox sandwich_target_box_left{0.386, 0.500, 0.060, 0.069};
    const ImageFloatBox sandwich_target_box_middle{0.470, 0.500, 0.060, 0.069};
    const ImageFloatBox sandwich_target_box_right{0.554, 0.500, 0.060, 0.069};

    const ImageFloatBox upper_bread_drop_box{0.482, 0.363, 0.036, 0.045};

    SandwichHandWatcher free_hand(SandwichHandType::FREE, initial_box);
    int ret = wait_until(console, context, std::chrono::seconds(30), {free_hand});
    if (ret < 0){
        throw OperationFailedException(console.logger(), "make_great_peanut_butter_sandwich(): Cannot detect starting hand.");
    }

    auto end_box = move_sandwich_hand(env, console, context, SandwichHandType::FREE, false, initial_box, ingredient_box);
    end_box = move_sandwich_hand(env, console, context, SandwichHandType::GRABBING, true, expand_box(end_box), sandwich_target_box_left);
    end_box = move_sandwich_hand(env, console, context, SandwichHandType::FREE, false, expand_box(end_box), ingredient_box);
    end_box = move_sandwich_hand(env, console, context, SandwichHandType::GRABBING, true, expand_box(end_box), sandwich_target_box_middle);
    end_box = move_sandwich_hand(env, console, context, SandwichHandType::FREE, false, expand_box(end_box), ingredient_box);
    end_box = move_sandwich_hand(env, console, context, SandwichHandType::GRABBING, true, expand_box(end_box), sandwich_target_box_right);

    // Drop upper bread
    end_box = move_sandwich_hand(env, console, context, SandwichHandType::GRABBING, false, expand_box(end_box), upper_bread_drop_box);
    pbf_mash_button(context, BUTTON_A, 125);
    context.wait_for_all_requests();
}

void finish_sandwich_eating(ConsoleHandle& console, BotBaseContext& context){
    PicnicWatcher picnic_watcher;
    int ret = run_until(
        console, context,
        [](BotBaseContext& context){
            for(int i = 0; i < 60; i++){
                pbf_press_button(context, BUTTON_A, 20, 105);
            }
        },
        {picnic_watcher}
    );
    if (ret < 0){
        throw OperationFailedException(console.logger(), "finish_sandwich_eating(): cannot detect picnic after 60 seconds.");
    }
    context.wait_for(std::chrono::seconds(1));
}


}
}
}
