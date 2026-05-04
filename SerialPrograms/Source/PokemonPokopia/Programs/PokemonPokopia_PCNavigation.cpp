/*  PC Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "PokemonPokopia/Inference/PokemonPokopia_ButtonDetector.h"
#include "PokemonPokopia/Inference/PokemonPokopia_SelectionArrowDetector.h"
#include "PokemonPokopia/Programs/PokemonPokopia_PCNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonPokopia{


namespace{

// Get the box for each PC menu item
// Palette Town has an extra "Appraise" box which skews the layout of the bottom row
ImageFloatBox get_pc_menu_option_box(PCMenuOption option, bool is_palette_town=false){
    switch (option){
    case PCMenuOption::CHALLENGES:
        return PC_CHALLENGES_BOX;
    case PCMenuOption::SHOP:
        return PC_SHOP_BOX;
    case PCMenuOption::LINK_PLAY:
        return is_palette_town ? PC_PALETTE_LINK_PLAY_BOX : PC_LINK_PLAY_BOX;
    case PCMenuOption::GET_ITEMS:
        return is_palette_town ? PC_PALETTE_GET_ITEMS_BOX : PC_GET_ITEMS_BOX;
    case PCMenuOption::STAMP_CARD:
        return is_palette_town ? PC_PALETTE_STAMP_CARD_BOX : PC_STAMP_CARD_BOX;
    case PCMenuOption::APPRAISE:
        if (!is_palette_town){
            throw InternalProgramError(
                nullptr, PA_CURRENT_FUNCTION,
                "APPRAISE option is only present in Palette Town"
            );
        }
        return PC_PALETTE_APPRAISE_BOX;
    }
    throw InternalProgramError(
        nullptr, PA_CURRENT_FUNCTION,
        "Invalid PCMenuOption enum value"
    );
}

// Get the box location for the daily item type (paintable, recipe, none) in the shop menu based on the index (0-4)
ImageFloatBox get_shop_daily_item_type_box(int item_index){
    return ImageFloatBox(
        SHOP_DAILY_ITEM_TYPE_BOX_1.x + item_index * SHOP_DAILY_ITEM_OFFSET.first,
        SHOP_DAILY_ITEM_TYPE_BOX_1.y + item_index * SHOP_DAILY_ITEM_OFFSET.second,
        SHOP_DAILY_ITEM_TYPE_BOX_1.width,
        SHOP_DAILY_ITEM_TYPE_BOX_1.height
    );
}

// Get the box location for the daily item coin icon in the shop menu based on the index (0-4)
ImageFloatBox get_shop_daily_item_coin_icon_box(int item_index){
    return ImageFloatBox(
        SHOP_DAILY_ITEM_COIN_ICON_BOX_1.x + item_index * SHOP_DAILY_ITEM_OFFSET.first,
        SHOP_DAILY_ITEM_COIN_ICON_BOX_1.y + item_index * SHOP_DAILY_ITEM_OFFSET.second,
        SHOP_DAILY_ITEM_COIN_ICON_BOX_1.width,
        SHOP_DAILY_ITEM_COIN_ICON_BOX_1.height
    );
}

std::vector<ImageFloatBox> get_shop_daily_item_selection_boxes(){
    std::vector<ImageFloatBox> boxes;
    for (size_t i = 0; i < 5; i++){
        boxes.emplace_back(
            SHOP_DAILY_ITEM_SELECTOR_BOX_1.x + i * SHOP_DAILY_ITEM_OFFSET.first,
            SHOP_DAILY_ITEM_SELECTOR_BOX_1.y + i * SHOP_DAILY_ITEM_OFFSET.second,
            SHOP_DAILY_ITEM_SELECTOR_BOX_1.width,
            SHOP_DAILY_ITEM_SELECTOR_BOX_1.height
        );
    }
    return boxes;
}

// Interact when the A button prompt appears
// Watches for the A button prompt and wait for it to clear
bool interact_button_a_prompt(ConsoleHandle& console, ProControllerContext& context){
    ButtonWatcher button_a(
        COLOR_GREEN, ButtonType::ButtonA,
        {0.250000, 0.000000, 0.500000, 1.000000},
        &console.overlay()
    );
    int ret = wait_until(
        console, context,
        30s,
        {
            button_a,
        }
    );
    if (ret != 0){
        return false;
    }

    WallClock deadline = current_time() + 60s;
    while (current_time() < deadline){
        context.wait_for_all_requests();
        pbf_press_button(context, BUTTON_A, 160ms, 240ms);
        context.wait_for(500ms);

        VideoSnapshot screen = console.video().snapshot();
        if (!button_a.detect(screen)){
            console.log("Detected interaction complete");
            return true;
        }
    }
    console.log("Failed to detect interaction complete");
    return false;
}

// Wait for PC to be open by watching for selectors
// Return status depending on what menu is first opened
PCOpenStatus wait_for_pc_open(ConsoleHandle& console, ProControllerContext& context){
    SelectionArrowWatcher challenges_selector(
        COLOR_YELLOW, &console.overlay(),
        SelectionArrowType::DOWN,
        get_pc_menu_option_box(PCMenuOption::CHALLENGES)
    );
    SelectionArrowWatcher add_stamp_selector(
        COLOR_YELLOW, &console.overlay(),
        SelectionArrowType::RIGHT,
        ADD_STAMP_BOX
    );
    // TODO: Handle environment change menu

    int ret = wait_until(
        console, context,
        30s,
        {
            challenges_selector,
            add_stamp_selector
        }
    );
    switch (ret){
        case 0:
            console.log("Detected main PC menu");
            return PCOpenStatus::MAIN;
        case 1:
            console.log("Detected stamp card open");
            return PCOpenStatus::STAMP_CARD;
        default:
            console.log("Failed to detect PC menu open");
            return PCOpenStatus::FAILURE;
    }
}

// Check if the row the selector is currently on is the same as the target row
// The Challenges and Shop menu options are fixed on the top row so check those
bool is_current_selector_on_target_row(ConsoleHandle& console, bool target_is_on_top_row){
    SelectionArrowDetector challenges_selector(
        COLOR_YELLOW, &console.overlay(),
        SelectionArrowType::DOWN,
        get_pc_menu_option_box(PCMenuOption::CHALLENGES)
    );
    SelectionArrowDetector shop_selector(
        COLOR_YELLOW, &console.overlay(),
        SelectionArrowType::DOWN,
        get_pc_menu_option_box(PCMenuOption::SHOP)
    );

    VideoSnapshot screen = console.video().snapshot();
    bool current_is_on_top_row = challenges_selector.detect(screen) || shop_selector.detect(screen);
    if ((target_is_on_top_row && current_is_on_top_row) || (!target_is_on_top_row && !current_is_on_top_row)){
        console.log("current selector is on target row");
        return true;
    }
    return false;
}

// Move the selector to the right row if it isn't already there
bool set_selector_on_target_row(ConsoleHandle& console, ProControllerContext& context, bool target_is_on_top_row){
    WallClock deadline = current_time() + 30s;
    while (current_time() < deadline){
        context.wait_for_all_requests();
        if (is_current_selector_on_target_row(console, target_is_on_top_row)){
            // context.wait_for(1000ms); // Confirmation in case video lags
            // if (is_current_selector_on_target_row(console, target_is_on_top_row)){
                console.log("Selector set on target row");
                return true;
            // }
        }
        pbf_press_dpad(context, DPAD_DOWN, 160ms, 240ms);
    }
    console.log("Failed to set selector on target row");
    return false;
}

// Helper to move the selector within the current row until the target is hovered
// Check for both possible locations for palette town and everywhere else
bool set_selector_on_target_within_row_routine(ConsoleHandle& console, ProControllerContext& context, PCMenuOption target){
    SelectionArrowWatcher target_selector(
        COLOR_YELLOW, &console.overlay(),
        SelectionArrowType::DOWN,
        get_pc_menu_option_box(target)
    );
    SelectionArrowWatcher target_selector_pallete(
        COLOR_YELLOW, &console.overlay(),
        SelectionArrowType::DOWN,
        get_pc_menu_option_box(target, true)
    );

    WallClock deadline = current_time() + 30s;
    while (current_time() < deadline){
        context.wait_for_all_requests();
        int ret = wait_until(
            console, context,
            1s,
            {
                target_selector,
                target_selector_pallete
            }
        );
        if (ret == 0 || ret == 1){
            console.log("Selector on target");
            return true;
        }
        pbf_press_dpad(context, DPAD_RIGHT, 160ms, 240ms);
    }
    console.log("Failed to navigate to the correct menu option within row");
    return false;
}

// Move the selector within the current row until the target is hovered
bool set_selector_on_target_within_row(ConsoleHandle& console, ProControllerContext& context, PCMenuOption target){
    WallClock deadline = current_time() + 30s;
    while (current_time() < deadline){
        if (set_selector_on_target_within_row_routine(console, context, target)){
            // context.wait_for(1000ms); // Confirmation in case video lags
            // if (set_selector_on_target_within_row_routine(console, context, target_selector)){
                console.log("Selector confirmed on target");
                return true;
            // }
        }
    }
    console.log("Failed to navigate and confirm the correct menu option within row");
    return false;
}

// Move the selector to hover the menu option specified by the target_selector
// Move vertically to the right row, then horizontally to the right option within the row
bool set_menu_option(ConsoleHandle& console, ProControllerContext& context, PCMenuOption option){
    bool target_is_on_top_row = (option == PCMenuOption::CHALLENGES || option == PCMenuOption::SHOP);

    if (!set_selector_on_target_row(console, context, target_is_on_top_row)){
        console.log("Failed to navigate to the correct row");
        return false;
    }
    if (set_selector_on_target_within_row(console, context, option)){
        console.log("Successfully navigated to the correct menu option");
        return true;
    }
    console.log("Failed to navigate to the correct menu option");
    return false;
}

// Get the index of the selector in a list of image box locations
int get_current_selector_index(ConsoleHandle& console, const std::vector<ImageFloatBox>& option_boxes, SelectionArrowType arrow_type){
    std::vector<std::unique_ptr<SelectionArrowDetector>> selectors;
    for (const ImageFloatBox& box : option_boxes){
        selectors.emplace_back(
            new SelectionArrowDetector(
                COLOR_YELLOW, &console.overlay(),
                arrow_type,
                box
            )
        );
    }
    WallClock deadline = current_time() + 30s;
    while (current_time() < deadline){
        VideoSnapshot screen = console.video().snapshot();
        for (size_t i = 0; i < selectors.size(); i++){
            if (selectors[i]->detect(screen)){
                return int(i);
            }
        }
    }
    return -1;
}

// Move the selector to the target index in a list of options
bool generic_navigate_to_target_routine(
    ConsoleHandle& console, 
    ProControllerContext& context, 
    const std::vector<ImageFloatBox>& option_boxes,
    int target_index,
    SelectionArrowType arrow_type
) {
    WallClock deadline = current_time() + 30s;
    while (current_time() < deadline){
        int current_index = get_current_selector_index(console, option_boxes, arrow_type);
        if (current_index == -1){
            return false;
        }
        if (current_index == target_index){
            console.log("Selector on target");
            return true;
        } else {
            int item_count = int(option_boxes.size());
            int delta = target_index - current_index;
            if (std::abs(delta) > item_count / 2){
                if (delta > 0){
                    delta = delta - item_count;
                } else {
                    delta = delta + item_count;
                }
            }
            if (delta > 0) {
                for (int i = 0; i < delta; i++) {
                    pbf_press_dpad(context, (arrow_type == SelectionArrowType::DOWN) ? DPAD_RIGHT : DPAD_DOWN, 160ms, 240ms);
                }
            } else {
                for (int i = 0; i < -delta; i++) {
                    pbf_press_dpad(context, (arrow_type == SelectionArrowType::DOWN) ? DPAD_LEFT : DPAD_UP, 160ms, 240ms);
                }
            }
            context.wait_for_all_requests();
        }
    }
    console.log("Failed to detect selector position");
    return false;
}

// Move the selector to the target index in a list of options and confirm it is there
bool generic_navigate_to_target(
    ConsoleHandle& console, 
    ProControllerContext& context, 
    const std::vector<ImageFloatBox>& option_boxes,
    int target_index,
    SelectionArrowType arrow_type
) {
    WallClock deadline = current_time() + 30s;
    while (current_time() < deadline){
        if (generic_navigate_to_target_routine(console, context, option_boxes, target_index, arrow_type)){
            context.wait_for_all_requests();
            context.wait_for(1s);
            int current_index = get_current_selector_index(console, option_boxes, arrow_type);
            if (current_index == target_index){
                console.log("Successfully navigated and confirmed the target");
                return true;
            }
        }
    }
    console.log("Failed to navigate and confirm the target");
    return false;
}

} // Namespace

// Top-level functions

void wait_for_overworld(ConsoleHandle& console, ProControllerContext& context){
    OverworldWatcher overworld_watcher(
        COLOR_GREEN, &console.overlay()
    );
    int ret = wait_until(
        console, context,
        120s,
        {overworld_watcher}
    );
    if (ret != 0){
        console.log("Failed to detect overworld");
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "wait_for_overworld() failed to detect overworld",
            console
        );
    }
    console.log("Detected overworld");
}

std::vector<ImageFloatBox> get_generic_options_boxes(size_t option_count){
    std::vector<ImageFloatBox> boxes;
    for (size_t i = 0; i < option_count; i++){
        boxes.emplace_back(
            ImageFloatBox(
                PC_GENERIC_TOP_BOX.x,
                PC_GENERIC_TOP_BOX.y + i * 0.100000,
                PC_GENERIC_TOP_BOX.width,
                PC_GENERIC_TOP_BOX.height
            )
        );
    }
    return boxes;
}

void access_pc_from_overworld(ConsoleHandle& console, ProControllerContext& context, bool stop_on_stamp_card){
    wait_for_overworld(console, context);

    // Need to physically face PC for PC open prompt
    ButtonWatcher button_a_watcher(
        COLOR_RED, ButtonType::ButtonA, 
        {0.250000, 0.000000, 0.500000, 1.000000}, // Button A can fall in a fairly large area
        &console.overlay()
    );
    SelectionArrowWatcher challenges_watcher(
        COLOR_YELLOW, &console.overlay(),
        SelectionArrowType::DOWN,
        get_pc_menu_option_box(PCMenuOption::CHALLENGES)
    );

    // Wandering Pokemon can trigger the same prompt, retry a few times
    for (int i = 0; i < 5; i++){
        int ret = run_until<ProControllerContext>(
            console, context,
            [&](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +0.8}, 25ms, 1000ms);
                pbf_move_left_joystick(context, {0, -0.8}, 25ms, 240ms); // return position
                pbf_move_left_joystick(context, {+0.8, 0}, 25ms, 1000ms);
                pbf_move_left_joystick(context, {-0.8, 0}, 25ms, 240ms); // return position
                pbf_move_left_joystick(context, {-0.8, 0}, 25ms, 1000ms);
                pbf_move_left_joystick(context, {+0.8, 0}, 25ms, 240ms); // return position
            },
            {button_a_watcher}
        );
        switch (ret){
        case 0:
            console.log("Detected A button prompt");
            if (interact_button_a_prompt(console, context)){
                PCOpenStatus status = wait_for_pc_open(console, context);
                switch (status){
                case PCOpenStatus::MAIN:
                    console.log("Successfully opened PC main menu");
                    return;
                case PCOpenStatus::STAMP_CARD:
                    console.log("Opened stamp card instead of main PC menu");
                    if (stop_on_stamp_card){
                        return;
                    }
                    // Go one level up until challenges menu on main menu is seen
                    ret = run_until<ProControllerContext>(
                        console, context,
                        [&](ProControllerContext& context){
                            for (int i = 0; i < 5; i++){
                                pbf_press_button(context, BUTTON_B, 160ms, 2000ms);
                            }
                        },
                        {challenges_watcher}
                    );
                    if (ret == 0){
                        console.log("Successfully navigated up to main PC menu from stamp card");
                        return;
                    } else {
                        console.log("Failed to navigate up to main PC menu from stamp card");
                        pbf_mash_button(context, BUTTON_B, 10s);
                        break;
                    }
                // Not sure how to handle this case, can't test it. Attempt to back out and try again for now
                // Currently not being detected
                case PCOpenStatus::ENVIRONMENT_CHANGE:
                    console.log("Environment change detected");
                    pbf_mash_button(context, BUTTON_B, 15s);
                    break;
                case PCOpenStatus::FAILURE:
                    console.log("Failed to open PC"); // Likely stuck talking to a wandering Pokemon
                    pbf_mash_button(context, BUTTON_B, 15s);
                    continue;
                default:
                    throw InternalProgramError(
                        nullptr, PA_CURRENT_FUNCTION,
                        "Invalid PCOpenStatus enum value"
                    );
                }
            }
        default:
            console.log("Failed to detect A button prompt, attempting to reposition and retry... (attempt " + std::to_string(i+1) + ")");
        }
    }
    OperationFailedException(
        ErrorReport::SEND_ERROR_REPORT,
        "access_pc_from_overworld() failed to open PC",
        console
    );
}

void exit_pc(ConsoleHandle& console, ProControllerContext& context){
    ButtonWatcher button_a(
        COLOR_GREEN, ButtonType::ButtonA,
        {0.250000, 0.000000, 0.500000, 1.000000},
        &console.overlay()
    );
    int ret = run_until<ProControllerContext>(
        console, context,
        [&](ProControllerContext& context){
            for (int i = 0; i < 20; i++){
                pbf_press_button(context, BUTTON_B, 160ms, 1000ms);
            }
        },
        {button_a}
    );
    if (ret != 0){
        console.log("Failed to detect return to overworld");
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "exit_pc() failed to detect return to overworld",
            console
        );
    }
    console.log("Exited PC and returned to overworld");
}

void open_menu_option(ConsoleHandle& console, ProControllerContext& context, PCMenuOption option){
    SelectionArrowDetector target_selector(
        COLOR_YELLOW, &console.overlay(),
        SelectionArrowType::DOWN,
        {0.150000, 0.200000, 0.700000, 0.500000}
    );
    if (!set_menu_option(console, context, option)){
        console.log("Failed to set menu option");
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "open_menu_option() failed to set menu option",
            console
        );
    }
    WallClock deadline = current_time() + 30s;
    while (current_time() < deadline){
        context.wait_for_all_requests();
        VideoSnapshot screen = console.video().snapshot();
        ImageFloatBox last_detected_box;
        if (target_selector.detect(screen)){
            last_detected_box = target_selector.last_detected();
        }
        else {
            context.wait_for(500ms);
            continue;
        }
        pbf_press_button(context, BUTTON_A, 160ms, 240ms);
        context.wait_for(500ms);
        screen = console.video().snapshot();
        ImageFloatBox current_detected_box;
        if (target_selector.detect(screen)){
            current_detected_box = target_selector.last_detected();
            if (std::abs(current_detected_box.x - last_detected_box.x) > 0.01 && std::abs(current_detected_box.y - last_detected_box.y) > 0.01){
                console.log("PC Menu option opened");
                return;
            }
        }
        else {
            console.log("PC Menu option opened"); // For subsequent menus without selectors
            return;
        }
    }
    console.log("Failed to open PC menu option");
    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "open_menu_option() failed to open PC menu option",
        console
    );
}

void generic_select_and_open(
    ConsoleHandle& console, 
    ProControllerContext& context, 
    const std::vector<ImageFloatBox>& option_boxes,
    int target_index,
    SelectionArrowType arrow_type,
    bool move_parallel
) {
    SelectionArrowWatcher target_selector(
        COLOR_YELLOW, &console.overlay(),
        arrow_type,
        option_boxes[target_index]
    );

    if (!generic_navigate_to_target(console, context, option_boxes, target_index, arrow_type)){
        console.log("Failed to navigate and confirm the target");
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "generic_select_and_open() failed to navigate and confirm the target",
            console
        );
    }
    WallClock deadline = current_time() + 30s;
    while (current_time() < deadline){
        context.wait_for_all_requests();
        pbf_press_button(context, BUTTON_A, 160ms, 240ms);
        context.wait_for(500ms);
        VideoSnapshot screen = console.video().snapshot();
        if (!target_selector.detect(screen)){
            console.log("Target option opened");
            return;
        }
    }
    console.log("Failed to open target option");
    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "generic_select_and_open() failed to open target option",
        console
    );
}

void yes_no_select_and_confirm(
    ConsoleHandle& console, 
    ProControllerContext& context, 
    bool select_yes
){
    generic_select_and_open(console, context, CONFIRM_YES_NO, (select_yes) ? 0 : 1, SelectionArrowType::RIGHT);
}

void continue_until_prompt(
    ConsoleHandle& console, 
    ProControllerContext& context, 
    const ImageFloatBox prompt_box
){
    SelectionArrowWatcher prompt_watcher(
        COLOR_YELLOW, &console.overlay(),
        SelectionArrowType::RIGHT,
        prompt_box
    );
    int ret = run_until<ProControllerContext>(
        console, context,
        [&](ProControllerContext& context){
            for (int i = 0; i < 20; i++){
                pbf_press_button(context, BUTTON_A, 160ms, 2000ms);
            }
        },
        {prompt_watcher}
    );
    if (ret != 0){
        console.log("Failed to detect prompt");
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "continue_until_prompt() failed to detect prompt",
            console
        );
    }
}

RecipeType item_is_recipe(ConsoleHandle& console, ProControllerContext& context, int item_index){
    RecipeIconDetector recipe_icon_detector(
        COLOR_YELLOW,
        &console.overlay(),
        get_shop_daily_item_type_box(item_index)
    );
    VideoSnapshot screen = console.video().snapshot();
    if (recipe_icon_detector.detect(screen)){
        console.log("Detected recipe icon for item " + std::to_string(item_index));
        return recipe_icon_detector.recipe_type();
    }
    else{
        console.log("No recipe icon detected for item " + std::to_string(item_index));
        return RecipeType::NOT_RECIPE;
    }
}

bool item_is_available(ConsoleHandle& console, ProControllerContext& context, int item_index){
    CoinIconDetector coin_icon_detector(
        COLOR_YELLOW,
        &console.overlay(),
        get_shop_daily_item_coin_icon_box(item_index)
    );
    VideoSnapshot screen = console.video().snapshot();
    if (coin_icon_detector.detect(screen)){
        console.log("Detected coin icon for item " + std::to_string(item_index) + ", item is available");
        return true;
    }
    else{
        console.log("No coin icon detected for item " + std::to_string(item_index) + ", item is not available");
        return false;
    }
}

void buy_item(ConsoleHandle& console, ProControllerContext& context, int item_index){
    std::vector<ImageFloatBox> selection_boxes = get_shop_daily_item_selection_boxes();
    SelectionArrowWatcher item_selector(
        COLOR_YELLOW, &console.overlay(),
        SelectionArrowType::DOWN,
        selection_boxes[item_index]
    );
    generic_select_and_open(console, context, selection_boxes, item_index, SelectionArrowType::DOWN);

    int ret = run_until<ProControllerContext>(
        console, context,
        [&](ProControllerContext& context){
            for (int i = 0; i < 20; i++){
                pbf_press_button(context, BUTTON_A, 160ms, 1500ms);
            }
        },
        {item_selector}
    );
    if (ret != 0){
        console.log("Failed to detect shop after purchase");
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "buy_item() failed to detect shop after purchase",
            console
        );
    }
}

Stamp get_stamp(
    ConsoleHandle& console, 
    ProControllerContext& context, 
    ImageFloatBox stamp_box
){
    StampDetector mew_stamp_detector(
        COLOR_YELLOW,
        Stamp::MEW,
        stamp_box,
        &console.overlay()
    );
    for (int i = 0; i < 5; i++){ // Take 5 samples to account for glint
        VideoSnapshot screen = console.video().snapshot();
        if (mew_stamp_detector.detect(screen)){
            console.log("Detected Mew Stamp");
            return Stamp::MEW;
        }
        context.wait_for(500ms);
    }
    console.log("Detected other stamp or no stamp");
    return Stamp::OTHER;
}

Stamp get_stamp_from_selection(
    ConsoleHandle& console, 
    ProControllerContext& context, 
    ImageFloatBox selection_arrow_box
){
    ImageFloatBox stamp_box = {
        selection_arrow_box.x + SELECTION_BOX_TO_STAMP_BOX_OFFSET.first,
        selection_arrow_box.y + SELECTION_BOX_TO_STAMP_BOX_OFFSET.second,
        TODAYS_STAMP_BOX.width, TODAYS_STAMP_BOX.height
    };
    return get_stamp(console, context, stamp_box);
}

bool add_stamp(ConsoleHandle& console, ProControllerContext& context, SelectionArrowWatcher& replace_stamp_selection_watcher){
    SelectionArrowWatcher add_stamp_selector(
        COLOR_YELLOW, &console.overlay(),
        SelectionArrowType::RIGHT,
        ADD_STAMP_BOX
    );
    WallClock deadline = current_time() + 30s;
    while(current_time() < deadline){
        context.wait_for_all_requests();
        pbf_press_button(context, BUTTON_A, 160ms, 240ms);
        context.wait_for(1000ms);

        int ret = wait_until(
            console, context,
            2s,
            {replace_stamp_selection_watcher}
        );
        if (ret == 0){
            console.log("Stamp replacement needed");
            return false;
        }
        ret = wait_until(
            console, context,
            2s,
            {add_stamp_selector}
        );
        if (ret != 0){
            console.log("Stamp added without replacement");
            return true;
        }
    }
    OperationFailedException(
        ErrorReport::SEND_ERROR_REPORT,
        "add_stamp() failed to add stamp",
        console
    );
    return false;
}

void replace_stamp(
    ConsoleHandle& console, 
    ProControllerContext& context, 
    SelectionArrowWatcher& replace_stamp_selection_watcher
){
    WallClock deadline = current_time() + 30s;
    while (current_time() < deadline){
        pbf_press_button(context, BUTTON_A, 160ms, 240ms);
        int ret = wait_until(
            console, context,
            2s,
            {replace_stamp_selection_watcher}
        );
        if (ret != 0){
            console.log("Detected stamp replaced");
            return;
        }
    }
    console.log("Failed to replace stamp");
    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "replace_stamp() failed to replace stamp",
        console
    );
}

void move_to_next_stamp(ConsoleHandle& console, ProControllerContext& context, SelectionArrowWatcher& replace_stamp_selection_watcher){
    ImageFloatBox current_selector_location = replace_stamp_selection_watcher.last_detected();

    WallClock deadline = current_time() + 30s;
    while (current_time() < deadline){
        pbf_press_button(context, BUTTON_DOWN, 160ms, 240ms);
        context.wait_for(500ms);
        int ret = wait_until(
            console, context,
            2s,
            {replace_stamp_selection_watcher}
        );
        if (ret != 0) {
            console.log("Failed to detect selector after moving to next stamp");
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "move_to_next_stamp() failed to detect selector after moving to next stamp",
                console
            );
        }
        ImageFloatBox new_selector_location = replace_stamp_selection_watcher.last_detected();
        if (std::abs(new_selector_location.x - current_selector_location.x) > 0.01 && std::abs(new_selector_location.y - current_selector_location.y) > 0.01) {
            console.log("Selector moved to next stamp");
            return;
        }
    }
    console.log("Failed to move selector to next stamp");
    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "move_to_next_stamp() failed to move selector to next stamp",
        console
    );
}



}
}
}
