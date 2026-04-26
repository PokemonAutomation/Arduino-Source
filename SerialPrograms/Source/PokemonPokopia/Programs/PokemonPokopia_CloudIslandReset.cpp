/*  Cloud Island Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonPokopia/Inference/PokemonPokopia_ButtonDetector.h"
#include "PokemonPokopia/Inference/PokemonPokopia_MovesDetection.h"
#include "PokemonPokopia/Inference/PokemonPokopia_PCDetection.h"
#include "PokemonPokopia/Inference/PokemonPokopia_SettingsScreenDetector.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"

#include "PokemonPokopia/Programs/PokemonPokopia_PCNavigation.h"
#include "PokemonPokopia/Programs/PokemonPokopia_CloudIslandReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonPokopia{

using namespace Pokemon;


CloudIslandReset_Descriptor::CloudIslandReset_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonPokopia:CloudIslandReset",
        STRING_POKEMON + " Pokopia", "Cloud Island Reset",
        "Programs/PokemonPokopia/CloudIslandReset.html",
        "Reset the Cloud Island for Mew stamps and recipes.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
class CloudIslandReset_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : resets(m_stats["Resets"])
        , mew_stamps(m_stats["Mew Stamps"])
        , recipes_purchased(m_stats["Recipes Purchased"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Mew Stamps");
        m_display_order.emplace_back("Recipes Purchased");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& mew_stamps;
    std::atomic<uint64_t>& recipes_purchased;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> CloudIslandReset_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


CloudIslandReset::CloudIslandReset()
    : STOP_AFTER_CURRENT("Reset")
    , NUM_RESETS(
        "<b>Number of Resets to Run:</b><br>"
        "Zero will run until 'Stop after Current Reset' is pressed or the program is manually stopped.",
        LockMode::UNLOCK_WHILE_RUNNING,
        500,
        0
    )
    , COLLECT_MEW_STAMPS(
        "<b>Collect Mew Stamps:</b><br>"
        "Whether to collect Mew stamps from the Cloud Islands",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , BUY_RECIPES(
        "<b>Buy Recipes:</b><br>"
        "Whether to buy recipes from the Cloud Island shop.",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
    , SPEND_LIMIT(
        "<b>Spend Limit:</b><br>"
        "The limit for how many Life Coins should remain after buying recipes.",
        LockMode::UNLOCK_WHILE_RUNNING,
        5000,
        0
    )
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(STOP_AFTER_CURRENT);
    PA_ADD_OPTION(NUM_RESETS);
    PA_ADD_OPTION(COLLECT_MEW_STAMPS);
    PA_ADD_OPTION(BUY_RECIPES);
    PA_ADD_OPTION(SPEND_LIMIT);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void CloudIslandReset::delete_cloud_island_save(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    CloudIslandReset_Descriptor::Stats& stats = env.current_stats<CloudIslandReset_Descriptor::Stats>();

    access_pc_from_overworld(env.console, context);
    env.console.log("Opened PC menu");

    open_menu_option(env.console, context, PCMenuOption::LINK_PLAY);
    env.console.log("Opened Link Play menu");

    // Navigate top level Link Play menu and enter settings [invite, visit, play, settings])
    generic_select_and_open(env.console, context, LINK_PLAY_MENU, 3, SelectionArrowType::RIGHT);
    env.console.log("Opened Link Play Settings menu");

    // Navigate Link Play Settings menu and enter Cloud Island Settings [Palette Town, Cloud Island]
    std::vector<ImageFloatBox> settings_menu_boxes = get_generic_options_boxes(2);
    generic_select_and_open(env.console, context, settings_menu_boxes, 1, SelectionArrowType::RIGHT);
    env.console.log("Opened Cloud Island Settings menu");

    // Navigate to the delete option in settings
    SettingsScreenWatcher settings_loaded_watcher({0.342500, 0.170000, 0.050000, 0.143000}); // Top setting, used to detect page is loaded
    SettingsScreenWatcher delete_button_watcher({0.342500, 0.521500, 0.050000, 0.143000}); // Bottom setting
    int ret = wait_until(
        env.console, context,
        30s,
        {
            settings_loaded_watcher
        }
    );
    if (ret != 0){
        env.console.log("Failed to detect settings menu loaded");
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "delete_cloud_island_save() failed to detect settings menu loaded",
            env.console
        );
    }
    ret = run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            for (int i = 0; i < 5; i++){
                pbf_press_dpad(context, DPAD_UP, 160ms, 1500ms);
            }
        },
        {delete_button_watcher}
    );
    if (ret != 0){
        env.console.log("Failed to navigate to delete save option");
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "delete_cloud_island_save() failed to navigate to delete save option",
            env.console
         );
    }
    env.console.log("Navigated to delete save option");

    // Select delete
    continue_until_prompt(env.console, context, CONFIRM_NO_BOX);
    env.console.log("Navigated through delete confirmation");

    yes_no_select_and_confirm(env.console, context, true);
    env.console.log("Confirmed delete");

    exit_pc(env.console, context);
    env.console.log("Exited PC menu");
}

void CloudIslandReset::create_cloud_island_after_delete(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    CloudIslandReset_Descriptor::Stats& stats = env.current_stats<CloudIslandReset_Descriptor::Stats>();

    access_pc_from_overworld(env.console, context);
    env.console.log("Opened PC menu");

    open_menu_option(env.console, context, PCMenuOption::LINK_PLAY);
    env.console.log("Opened Link Play menu");

    // Navigate top level Link Play menu and enter play Cloud Island menu [invite, visit, play, settings])
    generic_select_and_open(env.console, context, LINK_PLAY_MENU, 2, SelectionArrowType::RIGHT);
    env.console.log("Opened Link Play Settings menu");

    // Navigate Cloud Island menu and create new Cloud Island [go to, search, create]
    std::vector<ImageFloatBox> cloud_island_menu_boxes = get_generic_options_boxes(3);
    generic_select_and_open(env.console, context, cloud_island_menu_boxes, 2, SelectionArrowType::RIGHT);
    env.console.log("Opened Cloud Island Settings menu");

    // Navigate Create Cloud Island menu and generate random [random, magic number]
    std::vector<ImageFloatBox> create_cloud_island_menu_boxes = get_generic_options_boxes(2);
    generic_select_and_open(env.console, context, create_cloud_island_menu_boxes, 0, SelectionArrowType::RIGHT);
    env.console.log("Opened Create Cloud Island Settings menu");

    // A hacky way to get through name entry and island settings, wait at final confirmation
    ImageFloatBox final_confirm_box{0.202000, 0.722000, 0.027500, 0.057500};
    SelectionArrowWatcher create_island_confirm_watcher(
        COLOR_YELLOW, &env.console.overlay(),
        SelectionArrowType::RIGHT,
        final_confirm_box
    );
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            for (int i = 0; i < 20; i++){
                pbf_press_button(context, BUTTON_A, 160ms, 1000ms);
                pbf_press_button(context, BUTTON_PLUS, 160ms, 500ms);
                pbf_press_button(context, BUTTON_PLUS, 160ms, 500ms);
                pbf_press_button(context, BUTTON_PLUS, 160ms, 500ms);
            }
        },
        {create_island_confirm_watcher}
    );
    if (ret != 0){
        env.console.log("Failed to navigate through create island menu");
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "create_cloud_island_after_delete() failed to navigate through create island menu",
            env.console
         );
    }
    env.console.log("Waiting at create island confirmation");

    generic_select_and_open(env.console, context, {final_confirm_box}, 0, SelectionArrowType::RIGHT);
    env.console.log("Confirmed create island");

    // Nearby Pokemon can randomly ask to tag along so mash B until black screen
    BlackScreenWatcher black_screen_watcher;
    ret = run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_B, 15s);
        },
        {black_screen_watcher}
    );
    if (ret != 0){
        env.console.log("Failed to detect travelling to new cloud island");
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "create_cloud_island_after_delete() failed to detect travelling to new cloud island",
            env.console
        );
    }
    env.console.log("Travelling to new cloud island");
}

void CloudIslandReset::open_cloud_island_pc(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    wait_for_overworld(env.console, context);
    
    access_pc_from_overworld(env.console, context, true);
    env.console.log("Opened Cloud Island PC menu");
}

bool CloudIslandReset::add_todays_stamp(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    CloudIslandReset_Descriptor::Stats& stats = env.current_stats<CloudIslandReset_Descriptor::Stats>();
    SelectionArrowWatcher replace_stamp_selection_watcher(
        COLOR_YELLOW, &env.console.overlay(),
        SelectionArrowType::DOWN,
        STAMP_CARD_BOX
    );

    Stamp todays_stamp = get_stamp(env.console, context, TODAYS_STAMP_BOX);
    if (todays_stamp != Stamp::MEW){
        env.console.log("Today's stamp is not Mew, skipping adding to collection");
        exit_pc(env.console, context);
        return false;
    }
    env.console.log("Today's stamp is Mew, attempting to add to collection");
    bool stamp_added = add_stamp(env.console, context, replace_stamp_selection_watcher);
    if (stamp_added){
        env.console.log("Added today's stamp to collection");
        if (todays_stamp == Stamp::MEW){
            stats.mew_stamps++;
            env.update_stats();
        }
        exit_pc(env.console, context);
        return false;
    }

    for (int i = 0; i < 5; i++){
        env.console.log("Check stamp " + std::to_string(i + 1) + " / 5");
        // last_detected set in navigation routines
        ImageFloatBox selection_arrow = replace_stamp_selection_watcher.last_detected();
        Stamp slot_stamp = get_stamp_from_selection(env.console, context, selection_arrow);
        if (slot_stamp != Stamp::MEW){
            env.console.log("Stamp in slot " + std::to_string(i + 1) + " is not Mew, replacing with today's stamp");
            replace_stamp(env.console, context, replace_stamp_selection_watcher);
            if (todays_stamp == Stamp::MEW){
                stats.mew_stamps++;
                env.update_stats();
            }
            exit_pc(env.console, context);
            if (i == 4){ // replaced the last slot, so all stamps should be mew
                return true;
            }
            return false;
        }
        move_to_next_stamp(env.console, context, replace_stamp_selection_watcher);
    }
    env.console.log("All stamps in collection are Mew, failed to add today's stamp");
    return true;
}

bool CloudIslandReset::buy_recipes(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    CloudIslandReset_Descriptor::Stats& stats = env.current_stats<CloudIslandReset_Descriptor::Stats>();
    CoinCountWatcher coin_watcher(env.console);
    int ret;

    open_menu_option(env.console, context, PCMenuOption::SHOP);
    env.console.log("Opened shop menu");

    for (int i = 0; i < 5; i++){
        if (!item_is_available(env.console, context, i)){
            env.console.log("Recipe in index " + std::to_string(i) + " is not available for purchase, skipping");
            continue;
        }
        RecipeType recipe_type = item_is_recipe(env.console, context, i);
        // recipe_type = RecipeType::SINGLE; // Bypass for testing
        // if (i != 1) { continue; } // Bypass for testing
        switch (recipe_type){
        // For now, treat all recipes types with the same priority
        // TODO: Recipes costs are assumed to be 400 here but recipes can be on sale and cheaper ones can be prioritized
        case RecipeType::SINGLE:
        case RecipeType::DOUBLE:
        case RecipeType::TRIPLE:
        case RecipeType::QUAD:
            env.console.log("Recipe is available in index " + std::to_string(i));
            ret = wait_until(
                env.console, context,
                30s,
                {coin_watcher}
            );
            if (ret != 0){
                env.console.log("Failed to read coin count in shop menu");
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "buy_recipes() failed to read coin count in shop menu",
                    env.console
                );
            }
            env.console.log("Detected coin count: " + std::to_string(coin_watcher.coin_count()));
            // For now, assume all recipes are the full 400 coins
            if (coin_watcher.coin_count() - 400 < SPEND_LIMIT){
                env.console.log("Spend limit reached, not buying more recipes");
                return true;
            }
            buy_item(env.console, context, i);
            stats.recipes_purchased++;
            env.update_stats();
            break;
        case RecipeType::NOT_RECIPE:
            env.console.log("Index " + std::to_string(i) + " does not contain a recipe, skipping");
            continue;
        default:
            throw InternalProgramError(
                nullptr, PA_CURRENT_FUNCTION,
                "Unexpected recipe type detected in buy_recipes(): " + std::to_string((int)recipe_type)
            );
        }
    }
    exit_pc(env.console, context);
    return false;
}

void CloudIslandReset::leave_cloud_island(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    CloudIslandReset_Descriptor::Stats& stats = env.current_stats<CloudIslandReset_Descriptor::Stats>();

    access_pc_from_overworld(env.console, context);
    env.console.log("Opened PC menu");

    open_menu_option(env.console, context, PCMenuOption::LINK_PLAY);
    env.console.log("Opened Link Play menu");

    continue_until_prompt(env.console, context, CONFIRM_NO_BOX);
    env.console.log("Navigated to leave cloud island confirmation");

    yes_no_select_and_confirm(env.console, context, true);
    env.console.log("Leaving cloud island");

    BlackScreenWatcher black_screen_watcher;
    int ret = wait_until(
        env.console, context,
        60s,
        {black_screen_watcher}
    );
    if (ret != 0){
        env.console.log("Failed to detect leaving cloud island");
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "leave_cloud_island() failed to detect leaving cloud island",
            env.console
        );
    }
}

void CloudIslandReset::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    CloudIslandReset_Descriptor::Stats& stats = env.current_stats<CloudIslandReset_Descriptor::Stats>();

    DeferredStopButtonOption::ResetOnExit reset_on_exit(STOP_AFTER_CURRENT);

    bool all_stamps_mew = false;
    bool spend_limit_reached = false;

    while (NUM_RESETS != 0 && stats.resets < NUM_RESETS){
        delete_cloud_island_save(env, context);
        create_cloud_island_after_delete(env, context);
        stats.resets++;

        open_cloud_island_pc(env, context);
        if (COLLECT_MEW_STAMPS && !all_stamps_mew){
            all_stamps_mew = add_todays_stamp(env, context);
        }
        else {
            exit_pc(env.console, context); // Exit PC since first access always force opens stamp card
        }

        if (BUY_RECIPES && !spend_limit_reached){
            open_cloud_island_pc(env, context);
            spend_limit_reached = buy_recipes(env, context);
        }

        leave_cloud_island(env, context);
        bool collect_mew_stamps_done = !COLLECT_MEW_STAMPS || (COLLECT_MEW_STAMPS && all_stamps_mew);
        bool buy_recipes_done = !BUY_RECIPES || (BUY_RECIPES && spend_limit_reached);

        if (STOP_AFTER_CURRENT.should_stop() || (collect_mew_stamps_done && buy_recipes_done)){
            break;
        }
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}



}
}
}
