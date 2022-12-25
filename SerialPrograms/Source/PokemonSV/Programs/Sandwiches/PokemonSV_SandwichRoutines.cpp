/*  Sandwich Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Concurrency/AsyncDispatcher.h"
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_ScalarButtons.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_PicnicDetector.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_SandwichHandDetector.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_SandwichRecipeDetector.h"
#include "PokemonSV_SandwichRoutines.h"


#include <cmath>
#include <algorithm>
#include <sstream>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

bool enter_sandwich_recipe_list(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
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
            dump_image_and_throw_recoverable_exception(info, console, "NotEnterSandwichList",
                "enter_sandwich_recipe_list(): No recognized state after 60 seconds.");
        }
    }
}


bool select_sandwich_recipe(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context, size_t target_sandwich_ID){
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
                dump_image_and_throw_recoverable_exception(info, console, "RecipeSelectionArrowNotDetected",
                    "select_sandwich_recipe(): Cannot detect recipe selection arrow.");
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
    const double x = std::max(0.0, box.x - box.width * 1.5);
    const double y = std::max(0.0, box.y - box.height * 1.5);
    const double width = std::min(box.width*4, 1.0 - x);
    const double height = std::min(box.height*4, 1.0 - y);
    return ImageFloatBox(x, y, width, height);
}

ImageFloatBox hand_location_to_box(const std::pair<double, double>& loc){
    const double hand_width = 0.071, hand_height = 0.106;
    return {loc.first - hand_width/2, loc.second - hand_height/2, hand_width, hand_height};
}

std::string box_to_string(const ImageFloatBox& box){
    std::ostringstream os;
    os << "(" << box.x << ", " << box.y << ", " << box.width << ", " << box.height << ")";
    return os.str();
}

ImageFloatBox move_sandwich_hand(
    const ProgramInfo& info,
    AsyncDispatcher& dispatcher,
    ConsoleHandle& console,
    BotBaseContext& context,
    SandwichHandType hand_type,
    bool pressing_A,
    const ImageFloatBox& start_box,
    const ImageFloatBox& end_box
){
    context.wait_for_all_requests();
    console.log(std::string("Start moving sandwich hand: ") + ((hand_type == SandwichHandType::FREE) ? "FREE" : "GRABBING")
        + " start box " + box_to_string(start_box) + " end box " + box_to_string(end_box));

    
    uint8_t joystick_x = 128;
    uint8_t joystick_y = 128;

    SandwichHandWatcher hand_watcher(hand_type, start_box);

    // A session that creates a new thread to send button commands to controller
    AsyncCommandSession move_session(context, console.logger(), dispatcher, console.botbase());
    
    if (pressing_A){
        move_session.dispatch([](BotBaseContext& context){
            pbf_controller_state(context, BUTTON_A, DPAD_NONE, 128, 128, 128, 128, 3000);
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
            dump_image_and_throw_recoverable_exception(info, console, "SandwichHandNotDetected", "move_sandwich_hand(): Cannot detect hand.");
        }

        auto cur_loc = hand_watcher.location();
        console.log("Hand location: " + std::to_string(cur_loc.first) + ", " + std::to_string(cur_loc.second));
        cur_time = current_time();

        const ImageFloatBox hand_bb = hand_location_to_box(cur_loc); 
        const ImageFloatBox expanded_hand_bb = expand_box(hand_bb);
        hand_watcher.change_box(expanded_hand_bb);

        overlay_set.clear();
        overlay_set.add(COLOR_RED, hand_bb);
        overlay_set.add(COLOR_BLUE, expanded_hand_bb);

        std::pair<double, double> dif(target_loc.first - cur_loc.first, target_loc.second - cur_loc.second);
        // console.log("float diff to target: " + std::to_string(dif.first) + ", " + std::to_string(dif.second));
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
        // console.log("scaled diff to target: " + std::to_string(real_dif.first) + ", " + std::to_string(real_dif.second)
        //     + " distance " + std::to_string(distance));

        // Build a P-D controller!

        // We assume for a screen distance of 4 (1/4 of the width), we can use max joystick push, 128.
        // So for distance of value 1.0, we multiply by 32 to get joystick push
        double target_joystick_push = std::min(distance * 32, 128.0);

        std::pair<double, double> push(real_dif.first * target_joystick_push / distance, real_dif.second * target_joystick_push / distance);
        // console.log("push force " + std::to_string(push.first) + ", " + std::to_string(push.second));

        if (last_loc.first < 0){
            speed = std::make_pair(0.0, 0.0);
        } else {
            std::chrono::microseconds time = std::chrono::duration_cast<std::chrono::microseconds>(cur_time - last_time);
            double time_s = time.count() / 1000000.0;
            std::pair<double, double> moved((cur_loc.first - last_loc.first) * 16, (cur_loc.second - last_loc.second) * 9);

            // Currently set to zero damping as it seems we don't need them for now
            double damping_factor = 0.0;
            double damping_multiplier = (-1.0) * damping_factor / time_s;
            std::pair<double, double> damped_push_offset(moved.first * damping_multiplier, moved.second * damping_multiplier);

            push.first += damped_push_offset.first;
            push.second += damped_push_offset.second;
        }

        joystick_x = (uint8_t) std::max(std::min(int(push.first + 0.5) + 128, 255), 0);
        joystick_y = (uint8_t) std::max(std::min(int(push.second + 0.5) + 128, 255), 0);
        // console.log("joystick push " + std::to_string(joystick_x) + ", " + std::to_string(joystick_y));

        // Dispatch a new series of commands that overwrites the last ones
        move_session.dispatch([&](BotBaseContext& context){
            if (pressing_A){
                // Note: joystick_x and joystick_y must be defined to outlive `move_session`.
//                pbf_controller_state(context, BUTTON_A, DPAD_NONE, joystick_x, joystick_y, 128, 128, 20);
                ssf_press_button(context, BUTTON_A, 0, 1000, 0);
            }
            pbf_move_left_joystick(context, joystick_x, joystick_y, 20, 0);
        });
        
        console.log("Moved joystick");

        last_loc = cur_loc;
        last_time = cur_time;
        context.wait_for(std::chrono::milliseconds(80));
    }
}

} // end anonymous namesapce

void make_great_peanut_butter_sandwich(const ProgramInfo& info, AsyncDispatcher& dispatcher, ConsoleHandle& console, BotBaseContext& context){

    const ImageFloatBox initial_box{0.440, 0.455, 0.112, 0.179};
    const ImageFloatBox ingredient_box{0.455, 0.130, 0.090, 0.030};

    const ImageFloatBox sandwich_target_box_left  {0.386, 0.507, 0.060, 0.055};
    const ImageFloatBox sandwich_target_box_middle{0.470, 0.507, 0.060, 0.055};
    const ImageFloatBox sandwich_target_box_right {0.554, 0.507, 0.060, 0.055};

    const ImageFloatBox upper_bread_drop_box{0.482, 0.400, 0.036, 0.030};

    SandwichHandWatcher free_hand(SandwichHandType::FREE, initial_box);
    int ret = wait_until(console, context, std::chrono::seconds(30), {free_hand});
    if (ret < 0){
        dump_image_and_throw_recoverable_exception(info, console, "FreeHandNotDetected",
            "make_great_peanut_butter_sandwich(): Cannot detect starting hand.");
    }

    console.overlay().add_log("Pick first banana", COLOR_WHITE);
    auto end_box = move_sandwich_hand(info, dispatcher, console, context, SandwichHandType::FREE, false, initial_box, ingredient_box);

    console.overlay().add_log("Drop first banana", COLOR_WHITE);
    // visual feedback grabbing is not reliable. Switch to blind grabbing:
    end_box = move_sandwich_hand(info, dispatcher, console, context, SandwichHandType::GRABBING, true, expand_box(end_box), sandwich_target_box_left);
    
    // pbf_controller_state(context, BUTTON_A, DPAD_NONE, 100, 200, 128, 128, 120);
    // context.wait_for(std::chrono::milliseconds(100));
    // context.wait_for_all_requests();

    console.overlay().add_log("Pick second banana", COLOR_WHITE);
    end_box = move_sandwich_hand(info, dispatcher, console, context, SandwichHandType::FREE, false, {0, 0, 1.0, 1.0}, ingredient_box);

    console.overlay().add_log("Drop second banana", COLOR_WHITE);
    end_box = move_sandwich_hand(info, dispatcher, console, context, SandwichHandType::GRABBING, true, expand_box(end_box), sandwich_target_box_middle);
    // pbf_controller_state(context, BUTTON_A, DPAD_NONE, 128, 200, 128, 128, 120);
    // context.wait_for(std::chrono::milliseconds(100));
    // context.wait_for_all_requests();

    console.overlay().add_log("Pick third banana", COLOR_WHITE);
    end_box = move_sandwich_hand(info, dispatcher, console, context, SandwichHandType::FREE, false, {0, 0, 1.0, 1.0}, ingredient_box);
    
    console.overlay().add_log("Drop third banana", COLOR_WHITE);
    end_box = move_sandwich_hand(info, dispatcher, console, context, SandwichHandType::GRABBING, true, expand_box(end_box), sandwich_target_box_right);
    // pbf_controller_state(context, BUTTON_A, DPAD_NONE, 156, 200, 128, 128, 120);
    // context.wait_for(std::chrono::milliseconds(100));
    // context.wait_for_all_requests();

    // Drop upper bread and pick
    console.overlay().add_log("Drop upper bread and pick", COLOR_WHITE);
    SandwichHandWatcher grabbing_hand(SandwichHandType::FREE, {0, 0, 1.0, 1.0});
    ret = wait_until(console, context, std::chrono::seconds(30), {grabbing_hand});
    if (ret < 0){
        dump_image_and_throw_recoverable_exception(info, console, "GrabbingHandNotDetected",
            "make_great_peanut_butter_sandwich(): Cannot detect grabing hand when waiting for upper bread.");
    }

    auto hand_box = hand_location_to_box(grabbing_hand.location());

    end_box = move_sandwich_hand(info, dispatcher, console, context, SandwichHandType::GRABBING, false, expand_box(hand_box), upper_bread_drop_box);
    pbf_mash_button(context, BUTTON_A, 125 * 5);

    console.log("Hand end box " + box_to_string(end_box));
    console.overlay().add_log("Built sandwich", COLOR_WHITE);

    context.wait_for_all_requests();
}

void finish_sandwich_eating(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    console.overlay().add_log("Eating", COLOR_WHITE);
    PicnicWatcher picnic_watcher;
    int ret = run_until(
        console, context,
        [](BotBaseContext& context){
            for(int i = 0; i < 20; i++){
                pbf_press_button(context, BUTTON_A, 20, 3*TICKS_PER_SECOND - 20);
            }
        },
        {picnic_watcher}
    );
    if (ret < 0){
        dump_image_and_throw_recoverable_exception(info, console, "PicnicNotDetected",
            "finish_sandwich_eating(): cannot detect picnic after 60 seconds.");
    }
    console.overlay().add_log("Finish eating", COLOR_WHITE);
    context.wait_for(std::chrono::seconds(1));
}


}
}
}
