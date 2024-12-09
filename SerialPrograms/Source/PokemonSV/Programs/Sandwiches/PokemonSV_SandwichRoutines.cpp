/*  Sandwich Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Concurrency/AsyncDispatcher.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
//#include "CommonFramework/ImageTools/ImageFilter.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_PicnicDetector.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_SandwichHandDetector.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_SandwichIngredientDetector.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_SandwichRecipeDetector.h"
#include "PokemonSV/Resources/PokemonSV_FillingsCoordinates.h"
#include "PokemonSV/Resources/PokemonSV_Ingredients.h"
#include "PokemonSV_SandwichRoutines.h"
#include "PokemonSV/Programs/Sandwiches/PokemonSV_IngredientSession.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_SandwichPlateDetector.h"

#include <cmath>
#include <algorithm>
#include <sstream>
#include <unordered_map>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

namespace{
    const ImageFloatBox HAND_INITIAL_BOX{0.440, 0.455, 0.112, 0.179};
    const ImageFloatBox INGREDIENT_BOX{0.455, 0.130, 0.090, 0.030};

void wait_for_initial_hand(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
     SandwichHandWatcher free_hand(SandwichHandType::FREE, HAND_INITIAL_BOX);
    int ret = wait_until(console, context, std::chrono::seconds(30), {free_hand});
    if (ret < 0){
        dump_image_and_throw_recoverable_exception(info, console, "FreeHandNotDetected",
            "Cannot detect hand at start of making a sandwich.");
    }
}

} // anonymous namespace

bool enter_sandwich_recipe_list(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    context.wait_for_all_requests();
    console.log("Opening sandwich menu at picnic table.");

    // Firt, try pressing button A to bring up the menu to make sandwich
    pbf_press_button(context, BUTTON_A, 20, 80);

    WallClock start = current_time();
    bool opened_table_menu = false;
    while(true){
        context.wait_for_all_requests();
        if (current_time() - start > std::chrono::minutes(1)){
            dump_image_and_throw_recoverable_exception(
                info, console, "FailToSandwich",
                "enter_sandwich_recipe_list(): Failed to open sandwich menu after 1 minute."
            );
        }

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
            // walk forward and press A again
            pbf_move_left_joystick(context, 128, 0, 100, 40);
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
    for (int move_down_list_attempt = 0; move_down_list_attempt < max_move_down_list_attempts; move_down_list_attempt++){
        context.wait_for(std::chrono::milliseconds(200));
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
            }else if (target_cell == selected_cell + 1){
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
//        context.wait_for_all_requests();

        SandwichIngredientArrowWatcher pick_selection(0, COLOR_YELLOW);
        while(true){
            context.wait_for_all_requests();
            int ret = wait_until(
                console, context, std::chrono::seconds(3),
                {selection_watcher, pick_selection}
            );

            if (ret == 0){
                console.log("Detected recipe selection. Dropped Button A?");
                pbf_press_button(context, BUTTON_A, 30, 100);
                continue;
            }else if (ret == 1){
                console.log("Detected pick selection.");
                pbf_press_button(context, BUTTON_A, 30, 100);
                continue;
            }else{
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

namespace{

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

/* 
- center the cursor by moving the cursor to the edge of the screen, then away from the edge.
- then search the whole screen for the sandwich hand, instead of just the box, and
update the location of the sandwich hand
- return true if successful. else throw an exception
*/
bool move_then_recover_sandwich_hand_position(
    const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context, 
    SandwichHandType& hand_type, SandwichHandWatcher& hand_watcher,
    AsyncCommandSession& move_session
){

    console.log("center the cursor: move towards bottom right, then left slightly.");
    uint16_t num_ticks_to_move_1 = TICKS_PER_SECOND*4;
    uint16_t num_ticks_to_move_2 = 100;

    // center the cursor
    if(SandwichHandType::FREE == hand_type){
        // move to bottom right corner,
        pbf_move_left_joystick(context, 255, 255, num_ticks_to_move_1, 100);
        // move to left slightly
        pbf_move_left_joystick(context, 0, 128, num_ticks_to_move_2, 100);
        context.wait_for_all_requests();
    }
    else if(SandwichHandType::GRABBING == hand_type){
        // center the cursor while holding the A button, so you don't drop the ingredient.

        uint16_t num_ticks_to_move_total = num_ticks_to_move_1 + num_ticks_to_move_2;
        uint16_t num_ticks_to_wait = num_ticks_to_move_total + TICKS_PER_SECOND; // add one extra second of waiting
        uint16_t num_miliseconds_to_wait = (num_ticks_to_wait*1000)/TICKS_PER_SECOND;
        uint16_t num_ticks_to_hold_A = num_ticks_to_wait + TICKS_PER_SECOND*10; // hold A for extra 10 seconds
        // the A button hold will be overwritten on the next move_session.dispatch, in the main function
        
        move_session.dispatch([&](BotBaseContext& context){
            // move to bottom right corner, while holding A
            pbf_controller_state(context, BUTTON_A, DPAD_NONE, 255, 255, 128, 128, num_ticks_to_move_1);

            // move to left slightly, while holding A
            pbf_controller_state(context, BUTTON_A, DPAD_NONE, 0, 128, 128, 128, num_ticks_to_move_2);

            // keep holding A. 
            pbf_press_button(context, BUTTON_A, num_ticks_to_hold_A, 0);
            // pbf_controller_state(context, BUTTON_A, DPAD_NONE, 128, 128, 128, 128, 3000);
        });

        // - wait long enough for the cursor movement to finish, before we try image matching
        // - wait_for_all_requests doesn't work since we want to still hold the A button.
        // - this is a workaround until there is a way to wait for a subset of a bunch of overlapping buttons to finish
        // - need to make sure the A button hold is long enough to last past this wait.
        context.wait_for(Milliseconds(num_miliseconds_to_wait));
    }
    
    const VideoSnapshot& frame = console.video().snapshot();
    console.log("Try to recover sandwich hand location.");
    if(hand_watcher.recover_sandwich_hand_position(frame)){
        // sandwich hand detected.
        return true;
    }

    // if still can't find the sandwich hand, throw a exception
    dump_image_and_throw_recoverable_exception(
        info, console,
        SANDWICH_HAND_TYPE_NAMES(hand_type) + "SandwichHandNotDetected",
        "move_sandwich_hand(): Cannot detect " + SANDWICH_HAND_TYPE_NAMES(hand_type) + " hand."
    );
}

/* 
- moves the sandwich hand from start_box to end_box
- It detects the location of the sandwich hand, from within the bounds of the last frame's 
expanded_hand_bb (i.e. m_box field in SandwichHandLocator). 
Then updates the current location of expanded_hand_bb. 
Then moves the sandwich hand closer towards end_box. 
 */
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
    console.log("Start moving sandwich hand: " + SANDWICH_HAND_TYPE_NAMES(hand_type)
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

    if (PreloadSettings::instance().DEVELOPER_MODE){
        #if 0
            // to intentionally trigger failures in hand detection, for testing recovery
            if (SandwichHandType::FREE == hand_type){
                std::pair<double, double> hand_location(1.0, 1.0);
                const ImageFloatBox hand_bb_debug = hand_location_to_box(hand_location); 
                const ImageFloatBox expanded_hand_bb_debug = expand_box(hand_bb_debug);
                hand_watcher.change_box(expanded_hand_bb_debug);
                overlay_set.clear();
                overlay_set.add(COLOR_RED, hand_bb_debug);
                overlay_set.add(COLOR_BLUE, expanded_hand_bb_debug);
                pbf_move_left_joystick(context, 0, 0, TICKS_PER_SECOND*5, 100);  // move hand to screen edge
                context.wait_for_all_requests();
            }
        #endif

        #if 0
            // to intentionally trigger failures in hand detection, for testing recovery
            // move hand to edge of screen, while still holding A
            if (SandwichHandType::GRABBING == hand_type){
                pbf_controller_state(context, BUTTON_A, DPAD_NONE, 0, 0, 128, 128, TICKS_PER_SECOND*5);
                pbf_press_button(context, BUTTON_A, 200, 0);
                // pbf_controller_state(context, BUTTON_A, DPAD_NONE, 128, 128, 128, 128, 100);
            }
        #endif
    }

    while(true){
        int ret = wait_until(console, context, std::chrono::seconds(5), {hand_watcher});
        if (ret < 0){
            // - the sandwich hand might be at the edge of the screen, so move it to the middle
            // and try searching the entire screen again
            // - move hand to bottom-right, then to the middle
            console.log(
                "Failed to detect sandwich hand. It may be at the screen's edge. " 
                "Try moving the hand to the middle of the screen and try searching again.");

            if(move_then_recover_sandwich_hand_position(info, console, context, hand_type, hand_watcher, move_session)){
                continue;
            }

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
            console.log(SANDWICH_HAND_TYPE_NAMES(hand_type) + " hand reached target.");
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
        }else{
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
    console.log("Finished eating sandwich. Back at picnic.");
    context.wait_for(std::chrono::seconds(1));
}

namespace{

void repeat_press_until(
    const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    std::function<void()> button_press,
    const std::vector<PeriodicInferenceCallback>& callbacks,
    const std::string &error_name, const std::string &error_message,
    std::chrono::milliseconds detection_timeout = std::chrono::seconds(5),
    size_t max_presses = 10,
    std::chrono::milliseconds default_video_period = std::chrono::milliseconds(50),
    std::chrono::milliseconds default_audio_period = std::chrono::milliseconds(20)
){
    button_press();
    for(size_t i_try = 0; i_try < max_presses; i_try++){
        context.wait_for_all_requests();
        const int ret = wait_until(console, context, detection_timeout, callbacks);
        if (ret >= 0){
            return;
        }
        button_press();
    }

    dump_image_and_throw_recoverable_exception(info, console, "IngredientListNotDetected",
        "enter_custom_sandwich_mode(): cannot detect ingredient list after 50 seconds.");
}

void repeat_button_press_until(
    const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    uint16_t button, uint16_t hold_ticks, uint16_t release_ticks,
    const std::vector<PeriodicInferenceCallback>& callbacks,
    const std::string &error_name, const std::string &error_message,
    std::chrono::milliseconds iteration_length = std::chrono::seconds(5),
    size_t max_presses = 10,
    std::chrono::milliseconds default_video_period = std::chrono::milliseconds(50),
    std::chrono::milliseconds default_audio_period = std::chrono::milliseconds(20)
){
    const std::chrono::milliseconds button_time = std::chrono::milliseconds((hold_ticks + release_ticks) * (1000 / TICKS_PER_SECOND));
    repeat_press_until(info, console, context,
        [&](){ pbf_press_button(context, button, hold_ticks, release_ticks); },
        callbacks, error_name, error_message, iteration_length - button_time, max_presses,
        default_video_period, default_audio_period
    );
}

void repeat_dpad_press_until(
    const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    uint8_t dpad_position, uint16_t hold_ticks, uint16_t release_ticks,
    const std::vector<PeriodicInferenceCallback>& callbacks,
    const std::string &error_name, const std::string &error_message,
    std::chrono::milliseconds iteration_length = std::chrono::seconds(5),
    size_t max_presses = 10,
    std::chrono::milliseconds default_video_period = std::chrono::milliseconds(50),
    std::chrono::milliseconds default_audio_period = std::chrono::milliseconds(20)
){
    const std::chrono::milliseconds button_time = std::chrono::milliseconds((hold_ticks + release_ticks) * (1000 / TICKS_PER_SECOND));
    repeat_press_until(info, console, context,
        [&](){ pbf_press_dpad(context, dpad_position, hold_ticks, release_ticks); },
        callbacks, error_name, error_message, iteration_length - button_time, max_presses, 
        default_video_period, default_audio_period
    );
}


} // anonymous namespace


void enter_custom_sandwich_mode(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    context.wait_for_all_requests();
    console.log("Entering custom sandwich mode.");
    console.overlay().add_log("Custom sandwich", COLOR_WHITE);

    SandwichIngredientArrowWatcher ingredient_selection_arrow(0, COLOR_YELLOW);
    repeat_button_press_until(
        info, console, context, BUTTON_X, 40, 80, {ingredient_selection_arrow},
        "IngredientListNotDetected", "enter_custom_sandwich_mode(): cannot detect ingredient list after 50 seconds."
    );
}

namespace{

void finish_two_herbs_sandwich(
    const ProgramInfo& info, AsyncDispatcher& dispatcher, ConsoleHandle& console, BotBaseContext& context
){
    console.log("Finish determining ingredients for two-sweet-herb sandwich.");
    console.overlay().add_log("Finish picking ingredients", COLOR_WHITE);

    wait_for_initial_hand(info, console, context);

    console.overlay().add_log("Start making sandwich", COLOR_WHITE);
    move_sandwich_hand(info, dispatcher, console, context, SandwichHandType::FREE, false, HAND_INITIAL_BOX, INGREDIENT_BOX);
    // Mash button A to pick and drop ingredients, upper bread and pick.
    // Egg Power 3 is applied with only two sweet herb condiments!
    pbf_mash_button(context, BUTTON_A, 8 * TICKS_PER_SECOND);
    context.wait_for_all_requests();
    console.overlay().add_log("Built sandwich", COLOR_WHITE);
}

} // anonymous namesapce

void make_two_herbs_sandwich(
    const ProgramInfo& info, AsyncDispatcher& dispatcher, ConsoleHandle& console, BotBaseContext& context,
    EggSandwichType sandwich_type, size_t sweet_herb_index_last, size_t salty_herb_index_last, size_t bitter_herb_index_last
){
    // The game has at most 5 herbs, in the order of sweet, salty, sour, bitter, spicy:
    if (sweet_herb_index_last >= 5){ // sweet index can only be: 0, 1, 2, 3, 4
        throw InternalProgramError(&console.logger(), PA_CURRENT_FUNCTION,
            "Invalid sweet herb index: " + std::to_string(sweet_herb_index_last));
    }
    if (salty_herb_index_last >= 4){ // 0, 1, 2, 3
        throw InternalProgramError(&console.logger(), PA_CURRENT_FUNCTION,
            "Invalid salty herb index: " + std::to_string(salty_herb_index_last));
    }
    if (bitter_herb_index_last >= 2){ // 0, 1
        throw InternalProgramError(&console.logger(), PA_CURRENT_FUNCTION,
            "Invalid bitter herb index: " + std::to_string(bitter_herb_index_last));
    }

    if (sandwich_type == EggSandwichType::SALTY_SWEET_HERBS && salty_herb_index_last >= sweet_herb_index_last){
        throw InternalProgramError(&console.logger(), PA_CURRENT_FUNCTION,
            "Invalid salty and sweet herb indices: " + std::to_string(salty_herb_index_last) + ", " + std::to_string(sweet_herb_index_last));
    }
    if (sandwich_type == EggSandwichType::BITTER_SWEET_HERBS && bitter_herb_index_last >= sweet_herb_index_last){
        throw InternalProgramError(&console.logger(), PA_CURRENT_FUNCTION,
            "Invalid bitter and sweet herb indices: " + std::to_string(bitter_herb_index_last) + ", " + std::to_string(sweet_herb_index_last));
    }

    {
        // Press button A to add first filling, assumed to be lettuce
        DeterminedSandwichIngredientWatcher filling_watcher(SandwichIngredientType::FILLING, 0);
        repeat_button_press_until(
            info, console, context, BUTTON_A, 40, 50, {filling_watcher},
            "DeterminedIngredientNotDetected", "make_two_herbs_sandwich(): cannot detect determined lettuce after 50 seconds."
        );
    }

    {
        // Press button + to go to condiments page
        SandwichCondimentsPageWatcher condiments_page_watcher;
        repeat_button_press_until(
            info, console, context, BUTTON_PLUS, 40, 60, {condiments_page_watcher},
            "CondimentsPageNotDetected", "make_two_herbs_sandwich(): cannot detect condiments page after 50 seconds."
        );
    }

    size_t first_herb_index_last = 0;
    switch(sandwich_type){
    case EggSandwichType::TWO_SWEET_HERBS:
        first_herb_index_last = sweet_herb_index_last;
        break;
    case EggSandwichType::SALTY_SWEET_HERBS:
        first_herb_index_last = salty_herb_index_last;
        break;
    case EggSandwichType::BITTER_SWEET_HERBS:
        first_herb_index_last = bitter_herb_index_last;
        break;
    default:
        throw InternalProgramError(&console.logger(), PA_CURRENT_FUNCTION,
            "Invalid EggSandwichType for make_two_herbs_sandwich()");
    }

    auto move_one_up_to_row = [&](size_t row){
        console.log("Move arrow to row " + std::to_string(row));
        SandwichIngredientArrowWatcher arrow(row);
        repeat_dpad_press_until(info, console, context, DPAD_UP, 10, 30, {arrow}, "IngredientArrowNotDetected",
            "make_two_herbs_sandwich(): cannot detect ingredient selection arrow at row " + std::to_string(row) + " after 50 seconds."
        );
    };

    auto press_a_to_determine_herb = [&](size_t herb_index){
        DeterminedSandwichIngredientWatcher herb_watcher(SandwichIngredientType::CONDIMENT, herb_index);
        repeat_button_press_until(
            info, console, context, BUTTON_A, 40, 60, {herb_watcher}, "CondimentsPageNotDetected",
            "make_two_herbs_sandwich(): cannot detect detemined herb at cell " + std::to_string(herb_index) + " after 50 seconds."
        );
    };

    // Press DPAD_UP multiple times to move to the first herb row
    for(size_t i = 0; i < first_herb_index_last+1; i++){
        move_one_up_to_row(9 - i);
    }
    press_a_to_determine_herb(0); // Press A to detemine one herb
    // Press DPAD_UP againt to move to the second herb row
    for(size_t i = first_herb_index_last+1; i < sweet_herb_index_last+1; i++){
        move_one_up_to_row(9 - i);
    }
    press_a_to_determine_herb(1); // Press A to detemine the second herb

    {
        // Press button + to go to picks page
        SandwichPicksPageWatcher picks_page_watcher;
        repeat_button_press_until(
            info, console, context, BUTTON_PLUS, 40, 60, {picks_page_watcher},
            "CondimentsPageNotDetected", "make_two_herbs_sandwich(): cannot detect picks page after 50 seconds."
        );
    }

    // Mesh button A to select the first pick
    pbf_mash_button(context, BUTTON_A, 80);
    context.wait_for_all_requests();

    finish_two_herbs_sandwich(info, dispatcher, console, context);
}

void make_two_herbs_sandwich(
    const ProgramInfo& info, AsyncDispatcher& dispatcher, ConsoleHandle& console, BotBaseContext& context,
    EggSandwichType sandwich_type, Language language
){
    std::map<std::string, uint8_t> fillings = {{"lettuce", (uint8_t)1}};
    std::map<std::string, uint8_t> condiments = {{"sweet-herba-mystica", (uint8_t)1}};
    switch(sandwich_type){
    case EggSandwichType::TWO_SWEET_HERBS:
        condiments["sweet-herba-mystica"] = 2;
        break;
    case EggSandwichType::SALTY_SWEET_HERBS:
        condiments["salty-herba-mystica"] = 1;
        break;
    case EggSandwichType::BITTER_SWEET_HERBS:
        condiments["bitter-herba-mystica"] = 1;
        break;
    default:
        throw InternalProgramError(&console.logger(), PA_CURRENT_FUNCTION,
            "Invalid EggSandwichType for make_two_herbs_sandwich()");
    }
    add_sandwich_ingredients(dispatcher, console, context, language, std::move(fillings), std::move(condiments));

    finish_two_herbs_sandwich(info, dispatcher, console, context);
}

void make_sandwich_option(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context, SandwichMakerOption& SANDWICH_OPTIONS){
    const Language language = SANDWICH_OPTIONS.LANGUAGE;
    if (language == Language::None){
        throw UserSetupError(console.logger(), "Must set game language option to read ingredient lists.");
    }

    int num_fillings = 0;
    int num_condiments = 0;
    std::map<std::string, uint8_t> fillings;
    std::map<std::string, uint8_t> condiments;

    //Add the selected ingredients to the maps if set to custom
    if (SANDWICH_OPTIONS.BASE_RECIPE == BaseRecipe::custom){
        console.log("Custom sandwich selected. Validating ingredients.", COLOR_BLACK);
        console.overlay().add_log("Custom sandwich selected.");

        std::vector<std::unique_ptr<SandwichIngredientsTableRow>> table = SANDWICH_OPTIONS.SANDWICH_INGREDIENTS.copy_snapshot();

        for (const std::unique_ptr<SandwichIngredientsTableRow>& row : table){
            const std::string& table_item = row->item.slug();
            if (!(table_item == "baguette")) { //ignore baguette
                if (std::find(ALL_SANDWICH_FILLINGS_SLUGS().begin(), ALL_SANDWICH_FILLINGS_SLUGS().end(), table_item) != ALL_SANDWICH_FILLINGS_SLUGS().end()){
                    fillings[table_item]++;
                    num_fillings++;
                }else{
                    condiments[table_item]++;
                    num_condiments++;
                }
            }else{
                console.log("Skipping baguette as it is unobtainable.");
                console.overlay().add_log("Skipping baguette as it is unobtainable.", COLOR_WHITE);
            }
        }

        if (num_fillings == 0 || num_condiments == 0){
            throw UserSetupError(console.logger(), "Must have at least one filling and at least one condiment.");
        }

        if (num_fillings > 6 || num_condiments > 4){
            throw UserSetupError(console.logger(), "Number of fillings exceed 6 and/or number of condiments exceed 4.");
        }
        console.log("Ingredients validated.", COLOR_BLACK);
        console.overlay().add_log("Ingredients validated.", COLOR_WHITE);
    }
    else if(SANDWICH_OPTIONS.BASE_RECIPE == BaseRecipe::non_shiny){
        console.log("Preset sandwich selected.", COLOR_BLACK);
        console.overlay().add_log("Preset sandwich selected.");

        // std::vector<std::string> table = SANDWICH_OPTIONS.get_premade_ingredients(
        //     SANDWICH_OPTIONS.get_premade_sandwich_recipe(SANDWICH_OPTIONS.BASE_RECIPE, SANDWICH_OPTIONS.TYPE, SANDWICH_OPTIONS.PARADOX));

        // The only non-shiny sandwich added at this time is Normal Encounter.
        std::vector<std::string> table = SANDWICH_OPTIONS.get_premade_ingredients(SandwichRecipe::non_shiny_normal);

        for (auto&& s : table){
            if (std::find(ALL_SANDWICH_FILLINGS_SLUGS().begin(), ALL_SANDWICH_FILLINGS_SLUGS().end(), s) != ALL_SANDWICH_FILLINGS_SLUGS().end()){
                fillings[s]++;
                num_fillings++;
            }else{
                condiments[s]++;
                num_condiments++;
            }
        }
    }
    //Otherwise get the preset ingredients
    else{
        console.log("Preset sandwich selected.", COLOR_BLACK);
        console.overlay().add_log("Preset sandwich selected.");

        std::vector<std::string> table = SANDWICH_OPTIONS.get_premade_ingredients(
            SANDWICH_OPTIONS.get_premade_sandwich_recipe(SANDWICH_OPTIONS.BASE_RECIPE, SANDWICH_OPTIONS.TYPE, SANDWICH_OPTIONS.PARADOX));

        for (auto&& s : table){
            if (std::find(ALL_SANDWICH_FILLINGS_SLUGS().begin(), ALL_SANDWICH_FILLINGS_SLUGS().end(), s) != ALL_SANDWICH_FILLINGS_SLUGS().end()){
                fillings[s]++;
                num_fillings++;
            }else{
                condiments[s]++;
                num_condiments++;
            }
        }
        //Insert Herba Mystica if required
        if (SandwichMakerOption::two_herba_required(SANDWICH_OPTIONS.BASE_RECIPE)){
            if (SANDWICH_OPTIONS.HERBA_ONE == SANDWICH_OPTIONS.HERBA_TWO){
                condiments.insert(std::make_pair(SANDWICH_OPTIONS.herba_to_string(SANDWICH_OPTIONS.HERBA_ONE), (uint8_t)2));
            }else{
                condiments.insert(std::make_pair(SANDWICH_OPTIONS.herba_to_string(SANDWICH_OPTIONS.HERBA_ONE), (uint8_t)1));
                condiments.insert(std::make_pair(SANDWICH_OPTIONS.herba_to_string(SANDWICH_OPTIONS.HERBA_TWO), (uint8_t)1));
            }
            num_condiments++;
            num_condiments++;
        }
    }

    /*
    //Print ingredients
    cout << "Fillings:" << endl;
    for (const auto& [key, value] : fillings){
        std::cout << key << ": " << (int)value << endl;
    }
    cout << "Condiments:" << endl;
    for (const auto& [key, value] : condiments){
        std::cout << key << ": " << (int)value << endl;
    }
    */

    make_sandwich_preset(env, console, context, SANDWICH_OPTIONS.LANGUAGE, fillings, condiments);
}

void make_sandwich_preset(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context, Language language, std::map<std::string, uint8_t>& fillings, std::map<std::string, uint8_t>& condiments){
    //Sort the fillings by priority for building (ex. large items on bottom, cherry tomatoes on top)
    //std::vector<std::string> fillings_game_order = {"lettuce", "tomato", "cherry-tomatoes", "cucumber", "pickle", "onion", "red-onion", "green-bell-pepper", "red-bell-pepper",
    //    "yellow-bell-pepper", "avocado", "bacon", "ham", "prosciutto", "chorizo", "herbed-sausage", "hamburger", "klawf-stick", "smoked-fillet", "fried-fillet", "egg", "potato-tortilla",
    //    "tofu", "rice", "noodles", "potato-salad", "cheese", "banana", "strawberry", "apple", "kiwi", "pineapple", "jalapeno", "watercress", "basil"};
    std::vector<std::string> fillings_game_order = { "hamburger", "rice", "noodles", "smoked-fillet", "fried-fillet", "cucumber", "pickle", "tofu",
        "chorizo", "herbed-sausage", "potato-tortilla", "klawf-stick", "lettuce", "tomato", "onion", "red-onion", "green-bell-pepper", "red-bell-pepper", "yellow-bell-pepper", "avocado",
        "bacon", "ham", "prosciutto", "cheese", "banana", "strawberry", "apple", "kiwi", "pineapple", "jalape\xc3\xb1o", "watercress", "potato-salad", "egg", "basil", "cherry-tomatoes" };

    //Add keys to new vector and sort
    std::vector<std::string> fillings_sorted;
    for (auto i = fillings.begin(); i != fillings.end(); i++){
        fillings_sorted.push_back(i->first);
    }
    std::unordered_map<std::string, int> temp_map;
    for (auto i = 0; i < (int)fillings_game_order.size(); i++){
        temp_map[fillings_game_order[i]] = i;
    }
    auto compare = [&temp_map](const std::string& s, const std::string& s1){
        return temp_map[s] < temp_map[s1];
    };
    std::sort(fillings_sorted.begin(), fillings_sorted.end(), compare);

    /*
    //Print sorted fillings
    cout << "Sorted fillings:" << endl;
    for (auto i : fillings_sorted){
        cout << i << endl;
    }
    */

    //Calculate number of plates there will be on the build screen
    //Get each ingredient in list order, then get the number of times it appears from the map
    //Also store how many of each ingredient is in each plate (ex. 6 onion in first plate and then 3 onion in the next)
    int plates = 0;
    std::vector<int> plate_amounts;

    for (const std::string& i : fillings_sorted){
        //Add "full" plates
        int plate_calcs = (int)(fillings[i] / FillingsCoordinates::instance().get_filling_information(i).servingsPerPlate);
        if (plate_calcs != 0){
            plates += plate_calcs;
            for (int j = 0; j < plate_calcs; j++){
                plate_amounts.push_back(
                    (int)(FillingsCoordinates::instance().get_filling_information(i).servingsPerPlate)
                    *(int)(FillingsCoordinates::instance().get_filling_information(i).piecesPerServing)
                );
            }
        }

        //Add plates for remaining servings
        int plate_remaining = ((int)(fillings[i] % FillingsCoordinates::instance().get_filling_information(i).servingsPerPlate));
        if (plate_remaining != 0){
            plates++;
            plate_amounts.push_back(plate_remaining * (int)(FillingsCoordinates::instance().get_filling_information(i).piecesPerServing));
        }
    }
    //cout << "Number of plates: " << plates << endl;
    int plates_string = plates;
    console.log("Number of plates: " + std::to_string(plates_string), COLOR_BLACK);
    console.overlay().add_log("Number of plates: " + std::to_string(plates_string), COLOR_WHITE);
    for (const auto& filling: fillings){
        env.log("Require filling " + filling.first + " x" + std::to_string(int(filling.second)));
    }
    for (const auto& condiment: condiments){
        env.log("Require condiment " + condiment.first + " x" + std::to_string(int(condiment.second)));
    }

    //Player must be on default sandwich menu
    std::map<std::string, uint8_t> fillings_copy(fillings); //Making a copy as we need the map for later
    enter_custom_sandwich_mode(env.program_info(), console, context);
    add_sandwich_ingredients(
        env.realtime_dispatcher(),
        console, context,
        language,
        std::move(fillings_copy),
        std::move(condiments)
    );

    run_sandwich_maker(env, console, context, language, fillings, fillings_sorted, plates);
}

void run_sandwich_maker(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context, Language language, std::map<std::string, uint8_t>& fillings, std::vector<std::string>& fillings_sorted, int& plates){

    wait_for_initial_hand(env.program_info(), console, context);

    //Wait for labels to appear
    console.log("Waiting for labels to appear.", COLOR_BLACK);
    console.overlay().add_log("Waiting for labels to appear.", COLOR_WHITE);
    pbf_wait(context, 300);
    context.wait_for_all_requests();

    //Now read in plate labels and store which plate has what
    console.log("Reading plate labels.", COLOR_BLACK);
    console.overlay().add_log("Reading plate labels.", COLOR_WHITE);

    std::vector<std::string> plate_order;

    SandwichPlateDetector left_plate_detector(console.logger(), COLOR_RED, language, SandwichPlateDetector::Side::LEFT);
    SandwichPlateDetector middle_plate_detector(console.logger(), COLOR_RED, language, SandwichPlateDetector::Side::MIDDLE);
    SandwichPlateDetector right_plate_detector(console.logger(), COLOR_RED, language, SandwichPlateDetector::Side::RIGHT);

    {
        VideoSnapshot screen = console.video().snapshot();

        const int max_read_label_tries = 4;
        for (int read_label_try_count = 0; read_label_try_count < max_read_label_tries; ++read_label_try_count){
            std::string center_filling = middle_plate_detector.detect_filling_name(screen);
            if (center_filling.empty()){
                if (read_label_try_count + 1 < max_read_label_tries){
                    // Wait more time
                    pbf_wait(context, TICKS_PER_SECOND * 2);
                    context.wait_for_all_requests();
                    screen = console.video().snapshot();
                    continue;
                }else{
                    console.log("Read nothing on center plate label.");
                    OperationFailedException::fire(
                        console, ErrorReport::SEND_ERROR_REPORT,
                        "No ingredient found on center plate label.",
                        std::move(screen)
                    );
                }
            }
            console.log("Read center plate label: " + center_filling);
            console.overlay().add_log("Center plate: " + center_filling);
            plate_order.push_back(center_filling);
            break;
        }

        //Get left (2nd) ingredient
        std::string left_filling = left_plate_detector.detect_filling_name(screen);
        if (left_filling.empty()){
            console.log("No ingredient found on left label.");
            console.overlay().add_log("No left plate");
        }else{
            console.log("Read left plate label: " + left_filling);
            console.overlay().add_log("Left plate: " + left_filling);
            plate_order.push_back(left_filling);
        }

        //Get right (3rd) ingredient
        std::string right_filling = right_plate_detector.detect_filling_name(screen);
        if (right_filling.empty()){
            console.log("No ingredient found on right label.");
            console.overlay().add_log("No right plate");
        }else{
            console.log("Read right plate label: " + right_filling);
            console.overlay().add_log("Right plate: " + right_filling);
            plate_order.push_back(right_filling);
        }

        //Get remaining ingredients if any
        //center 1, left 2, right 3, far left 4, far far left/right 5, right 6
        //this differs from the game layout: far right is 5 and far far left/right is 6 in game
        //however as long as we stay internally consistent with this numbering it will work
        for (int i = 0; i < (plates - 3); i++){
            pbf_press_button(context, BUTTON_R, 20, 180);
            context.wait_for_all_requests();

            screen = console.video().snapshot();
            left_filling = left_plate_detector.detect_filling_name(screen);

            if (left_filling.empty()){
                OperationFailedException::fire(
                    console, ErrorReport::SEND_ERROR_REPORT,
                    "No ingredient label found on remaining plate " + std::to_string(i) + ".",
                    std::move(screen)
                );
            }
            console.log("Read remaining plate " + std::to_string(i) + " label: " + left_filling);
            console.overlay().add_log("Remaining plate " + std::to_string(i) + ": " + left_filling);
            plate_order.push_back(left_filling);
        }

        //Now re-center plates
        console.log("Re-centering plates if needed.");
        console.overlay().add_log("Re-centering plates if needed.");
        for (int i = 0; i < (plates - 3); i++){
            pbf_press_button(context, BUTTON_L, 20, 80);
        }

        //If a label fails to read it'll cause issues down the line
        if ((int)plate_order.size() != plates){
            env.log("Found # plate labels " + std::to_string(plate_order.size()) + ", not same as desired # plates " + std::to_string(plates));
            OperationFailedException::fire(
                console, ErrorReport::SEND_ERROR_REPORT,
                "Number of plate labels did not match number of plates.",
                std::move(screen)
            );
        }
    }

    //Finally.
    console.log("Start making sandwich", COLOR_BLACK);
    console.overlay().add_log("Start making sandwich.", COLOR_WHITE);

    const ImageFloatBox center_plate{ 0.455, 0.130, 0.090, 0.030 };
    const ImageFloatBox left_plate{ 0.190, 0.136, 0.096, 0.031 };
    const ImageFloatBox right_plate{ 0.715, 0.140, 0.108, 0.033 };

    ImageFloatBox target_plate = center_plate;
    //Initial position handling
    auto end_box = move_sandwich_hand(env.program_info(), env.realtime_dispatcher(), console, context, SandwichHandType::FREE, false, HAND_INITIAL_BOX, HAND_INITIAL_BOX);
    move_sandwich_hand(env.program_info(), env.realtime_dispatcher(), console, context, SandwichHandType::GRABBING, true, { 0, 0, 1.0, 1.0 }, HAND_INITIAL_BOX);
    context.wait_for_all_requests();

    //Find fillings and add them in order
    for (const std::string& i : fillings_sorted){
        //cout << "Placing " << i << endl;
        console.overlay().add_log("Placing " + i, COLOR_WHITE);

        int times_to_place = (int)(FillingsCoordinates::instance().get_filling_information(i).piecesPerServing) * (fillings.find(i)->second);
        int placement_number = 0;

        //cout << "Times to place: " << times_to_place << endl;
        console.overlay().add_log("Times to place: " + std::to_string(times_to_place), COLOR_WHITE);

        std::vector<int> plate_index;
        //Get the plates we want to go to
        for (int j = 0; j < (int)plate_order.size(); j++){
            if (i == plate_order.at(j)){
                plate_index.push_back(j);
            }
        }

        //Target the correct filling plate and place until it is empty
        for (int j = 0; j < (int)plate_index.size(); j++){
            //Navigate to plate and set target plate
            //cout << "Target plate: " << plate_index.at(j) << endl;
            console.overlay().add_log("Target plate: " + std::to_string(plate_index.at(j)), COLOR_WHITE);
            switch (plate_index.at(j)){
            case 0:
                target_plate = center_plate;
                break;
            case 1:
                target_plate = left_plate;
                break;
            case 2:
                target_plate = right_plate;
                break;
            case 3: case 4: case 5: case 6:
                //Press R the appropriate number of times
                for (int k = 2; k < plate_index.at(j); k++){
                    pbf_press_button(context, BUTTON_R, 20, 80);
                }
                target_plate = left_plate;
                break;
            default:
                break;
            }

            //Place the fillings until label does not light up yellow on grab/the piece count is not hit
            while (true){
                //Break out after placing all pieces of the filling
                if (placement_number == times_to_place){
                    break;
                }

                end_box = move_sandwich_hand(env.program_info(), env.realtime_dispatcher(), console, context, SandwichHandType::FREE,
                    false, { 0, 0, 1.0, 1.0 }, target_plate);
                context.wait_for_all_requests();

                //Get placement location
                ImageFloatBox placement_target = FillingsCoordinates::instance().get_filling_information(i).placementCoordinates.at(
                    (int)fillings.find(i)->second).at(placement_number);

                end_box = move_sandwich_hand(env.program_info(), env.realtime_dispatcher(), console, context, SandwichHandType::GRABBING,
                    true, expand_box(end_box), placement_target);
                context.wait_for_all_requests();

                //If any of the labels are yellow continue. Otherwise assume plate is empty move on to the next.
                auto screen = console.video().snapshot();

                //The label check is needed for ingredients with multiple plates as we don't know which plate has what amount
                if (!left_plate_detector.is_label_yellow(screen) && !middle_plate_detector.is_label_yellow(screen)
                    && !right_plate_detector.is_label_yellow(screen)){
                    context.wait_for_all_requests();
                    break;
                }

                //If the plate is empty the increment is skipped using the above break
                placement_number++;
            }

            //Reset plate positions
            for (int k = 2; k < plate_index.at(j); k++){
                pbf_press_button(context, BUTTON_L, 20, 80);
            }
        }
    }
    // Handle top slice by tossing it away
    SandwichHandWatcher grabbing_hand(SandwichHandType::FREE, { 0, 0, 1.0, 1.0 });
    int ret = wait_until(console, context, std::chrono::seconds(30), { grabbing_hand });
    if (ret < 0){
        OperationFailedException::fire(
            console, ErrorReport::SEND_ERROR_REPORT,
            "SandwichMaker: Cannot detect grabing hand when waiting for upper bread.",
            grabbing_hand.last_snapshot()
        );
    }

    auto hand_box = hand_location_to_box(grabbing_hand.location());

    end_box = move_sandwich_hand(env.program_info(), env.realtime_dispatcher(), console, context, SandwichHandType::GRABBING, false, expand_box(hand_box), center_plate);
    pbf_mash_button(context, BUTTON_A, 125 * 5);

    env.log("Hand end box " + box_to_string(end_box));
    env.log("Built sandwich", COLOR_BLACK);
    // env.console.overlay().add_log("Hand end box " + box_to_string(end_box), COLOR_WHITE);
    console.overlay().add_log("Built sandwich.", COLOR_WHITE);
    context.wait_for_all_requests();

    finish_sandwich_eating(env.program_info(), console, context);
}

}
}
}
