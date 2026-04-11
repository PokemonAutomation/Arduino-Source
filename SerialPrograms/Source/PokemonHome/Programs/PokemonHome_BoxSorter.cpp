/*  Home Box Sorter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

/* TODO ideas
read pokemon name and store the slug (easier to detect missread than reading a number)
Optimise the swapping algo
Add enum for ball ? Also, BDSP is reading from swsh data. Worth refactoring ?

ideas for more checks :
ability
nature
type
original game
OT
moves
stats
level
surname
language
"stamps"
*/

#include <array>
#include <map>
#include <optional>
#include <sstream>
#include <algorithm>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/FrozenImageDetector.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "Pokemon/Resources/Pokemon_PokemonSlugs.h"
#include "Pokemon/Pokemon_BoxCursor.h"
#include "Pokemon/Pokemon_CollectedPokemonInfo.h"
#include "PokemonHome/Inference/PokemonHome_ButtonDetector.h"
#include "PokemonHome/Inference/PokemonHome_BoxGenderDetector.h"
#include "PokemonHome/Inference/PokemonHome_BallReader.h"
#include "PokemonHome_BoxSorter.h"
#include "PokemonHome_BoxNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{
using namespace Pokemon;


const size_t MAX_BOXES = 200;


BoxSorter_Descriptor::BoxSorter_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonHome:BoxSorter",
        STRING_POKEMON + " Home", "Box Sorter",
        "Programs/PokemonHome/BoxSorter.html",
        "Order boxes of " + STRING_POKEMON + ".",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct BoxSorter_Descriptor::Stats : public StatsTracker{
    Stats()
        : pkmn(m_stats["Pokemon"])
        , empty(m_stats["Empty Slots"])
        , compare(m_stats["Compares"])
        , swaps(m_stats["Swaps"])
    {
        m_display_order.emplace_back(Stat("Pokemon"));
        m_display_order.emplace_back(Stat("Empty Slots"));
        m_display_order.emplace_back(Stat("Compares"));
        m_display_order.emplace_back(Stat("Swaps"));
    }
    std::atomic<uint64_t>& pkmn;
    std::atomic<uint64_t>& empty;
    std::atomic<uint64_t>& compare;
    std::atomic<uint64_t>& swaps;
};
std::unique_ptr<StatsTracker> BoxSorter_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

BoxSorter::BoxSorter()
    : BOX_NUMBER(
        "<b>Number of Boxes to Sort:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        1, 1, MAX_BOXES
    )
    , VIDEO_DELAY(
          "<b>Capture Card Delay:</b>",
          LockMode::LOCK_WHILE_RUNNING,
          "400 ms"
    )
    , GAME_DELAY(
          "<b>" + STRING_POKEMON + " Home App Delay:</b>",
          LockMode::LOCK_WHILE_RUNNING,
          "240 ms"
    )
    , SORT_TABLE(
          "<b>Sort Order Rules:</b><br>Sort order rules will be applied top to bottom."
    )
    , OUTPUT_FILE(
          false,
          "<b>Output File:</b><br>JSON file basename to catalogue box info found by the program.",
          LockMode::LOCK_WHILE_RUNNING,
          "box_order",
          "box_order"
    )
    , DRY_RUN(
          "<b>Dry Run:</b><br>Catalogue and make sorting plan without execution. Check output at <output_file>.json and <output_file>-sorted.json)",
          LockMode::LOCK_WHILE_RUNNING,
          false
    )
    , NOTIFICATIONS({
          &NOTIFICATION_PROGRAM_FINISH
    })
{
    PA_ADD_OPTION(BOX_NUMBER); //number of box to check and sort
    PA_ADD_OPTION(VIDEO_DELAY); //delay for every input that need video feedback, user will be able to modify this to enhance capture card delay compatibility
    PA_ADD_OPTION(GAME_DELAY);  //delay for non video feedback, this way I can go as fast as pokemon home can handle movement when needed
    PA_ADD_OPTION(SORT_TABLE);
    PA_ADD_OPTION(OUTPUT_FILE);
    PA_ADD_OPTION(DRY_RUN);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void sort(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    std::vector<std::optional<CollectedPokemonInfo>> boxes_data,
    std::vector<std::optional<CollectedPokemonInfo>> boxes_sorted,
    BoxSorter_Descriptor::Stats& stats,
    BoxCursor& cur_cursor,
    Milliseconds GAME_DELAY
){
    env.log("Start sorting...");
    env.add_overlay_log("Start Sorting...");

    std::ostringstream ss;
    // this need to be separated into functions when I will redo the whole thing but I just wanted it to work

    // going thru the sorted list one by one and for each one go through the current pokemon layout to find the closest possible match to fill the slot
    for (size_t poke_nb_s = 0; poke_nb_s < boxes_sorted.size(); poke_nb_s++){
        if (boxes_sorted[poke_nb_s] == std::nullopt){ // we've hit the end of the sorted list.
            break;
        }
        for (size_t poke_nb = poke_nb_s; poke_nb < boxes_data.size(); poke_nb++){
            BoxCursor cursor_s(poke_nb_s);
            BoxCursor cursor(poke_nb);

            // ss << "Comparing " << boxes_data[poke_nb] << " at " << cursor << " to " << boxes_sorted[poke_nb_s] << " at " << cursor_s;
            // env.console.log(ss.str());
            // ss.str("");

            //check for a match and also check if the pokemon is not already in the slot
            stats.compare++;
            env.update_stats();
            if(boxes_sorted[poke_nb_s] == boxes_data[poke_nb] && poke_nb_s == poke_nb){ // Same spot no need to move.
                break;
            }
            if(boxes_sorted[poke_nb_s] == boxes_data[poke_nb]){
                ss << "Swapping " << boxes_data[poke_nb] << " at " << cursor << " and " << boxes_sorted[poke_nb_s] << " at " << cursor_s;
                env.console.log(ss.str());
                ss.str("");

                //moving cursor to the pokemon to pick it up
                cur_cursor = move_cursor_to(env, context, cur_cursor, cursor, GAME_DELAY);
                pbf_press_button(context, BUTTON_Y, 80ms, GAME_DELAY + 240ms);

                //moving to destination to place it or swap it
                cur_cursor = move_cursor_to(env, context, cur_cursor, cursor_s, GAME_DELAY);
                pbf_press_button(context, BUTTON_Y, 80ms, GAME_DELAY + 240ms);

                context.wait_for_all_requests();

                std::swap(boxes_data[poke_nb_s], boxes_data[poke_nb]);
                stats.swaps++;
                env.update_stats();

                break;
            }
        }
    }
}

void BoxSorter::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StartProgramChecks::check_performance_class_wired_or_wireless(context);
    
    const std::vector<SortingRule> sort_preferences = SORT_TABLE.preferences();
    if (sort_preferences.empty()){
        throw UserSetupError(env.console, "At least one sorting method selection needs to be made!");
    }

    BoxSorter_Descriptor::Stats& stats = env.current_stats< BoxSorter_Descriptor::Stats>();

    // vector that will store data for each slot
    std::vector<std::optional<CollectedPokemonInfo>> boxes_data;

    BoxCursor cur_cursor{static_cast<uint16_t>(BOX_NUMBER-1), 0, 0};

    VideoOverlaySet video_overlay_set(env.console);

    BoxCursor dest_cursor;
    BoxCursor nav_cursor = {0, 0, 0};

	exit_menus(env, context, VIDEO_DELAY.get());
    
    BoxViewWatcher box_view_watcher(&env.console.overlay());
    SummaryScreenWatcher summary_screen_watcher(&env.console.overlay());

    //cycle through each box
    for (size_t box_idx = 0; box_idx < BOX_NUMBER; box_idx++){
        if(box_idx != 0){
            // Press button R to move to next box
            pbf_press_button(context, BUTTON_R, 80ms, VIDEO_DELAY.get() + 800ms);
        }else{
            // Moving the cursor until it goes to the first slot
            if(!go_to_first_slot(env, context, VIDEO_DELAY)){
                throw UserSetupError(
                    env.logger(),
                    "ERROR: Could not move cursor to the first slot, please consider adjusting delay."
                );
            }
        }

        context.wait_for_all_requests();
        const std::string log_msg = "Checking box " + std::to_string(box_idx+1) + "/" + std::to_string(BOX_NUMBER);
        env.log(log_msg);
        env.console.overlay().add_log(log_msg);


        // Check box grid color stddev to find occupied slots and fill boxes_data with placeholder pokemon info.
        const std::array<size_t, 2> first_pokemon_slot = find_occupied_slots_in_box(env, context, boxes_data, sort_preferences);

        //enter the summary screen to read pokemon info
        if (first_pokemon_slot[0] != SIZE_MAX){
            // moving cursor to the first pokemon slot
            dest_cursor = {0, first_pokemon_slot[0], first_pokemon_slot[1]};
            nav_cursor = move_cursor_to(env, context, nav_cursor, dest_cursor, GAME_DELAY);

            env.add_overlay_log("Checking Summary...");

            pbf_press_button(context, BUTTON_A, 80ms, GAME_DELAY);
            context.wait_for_all_requests();
            video_overlay_set.clear();
            pbf_press_dpad(context, DPAD_DOWN, 80ms, GAME_DELAY);
            pbf_press_button(context, BUTTON_A, 80ms, 100ms);
            context.wait_for_all_requests();
            int ret = wait_until(env.console, context, Seconds(5), {summary_screen_watcher});
            if (ret != 0){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT, "HomeBoxSorter(): does not find summary screen after 5 sec", env.console
                );
            }


            // cycle through each summary of the current box and fill pokemon information
            for (size_t row = 0; row < BOX_ROWS; row++){
                for (size_t column = 0; column < BOX_COLS; column++){
                    const size_t global_idx = to_global_index(box_idx, row, column);
                    if (!boxes_data[global_idx].has_value()){
                        continue;
                    }

                    // Read the summary screen and assign data to boxes_data[global_idx]
                    read_summary_screen(env, context, boxes_data[global_idx].value());
                }
            }

            // log detailed pokemon infomation of this box
            std::ostringstream ss;
            ss << std::endl;
            for (size_t row = 0; row < BOX_ROWS; row++){
                for (size_t column = 0; column < BOX_COLS; column++){
                    ss << "[" << row << ", " << column << "]: " << boxes_data[to_global_index(box_idx, row, column)] << std::endl;
                }
            }
            env.log(ss.str());

            //get out of summary with a lot of delay because it's slow for some reasons
            pbf_press_button(context, BUTTON_B, 80ms, 100ms);
            context.wait_for_all_requests();
            ret = wait_until(env.console, context, Seconds(5), {box_view_watcher});
            if (ret != 0){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT, "HomeBoxSorter(): does not find box view after 5 sec", env.console
                );
            }
            video_overlay_set.clear();
        }

        video_overlay_set.clear();

        dest_cursor = {0, 0, 0};
        nav_cursor = move_cursor_to(env, context, nav_cursor, dest_cursor, GAME_DELAY);
        context.wait_for_all_requests();
    } // end if (find_first_poke){

    // copy boxes data to sort
    std::vector<std::optional<CollectedPokemonInfo>> boxes_sorted = boxes_data;

    // sorting copy of boxes_data
    std::sort(boxes_sorted.begin(), boxes_sorted.end());

    env.log("Current boxes data :");
    print_boxes_data(boxes_data, env);
    const std::string json_path_basename = OUTPUT_FILE;
    save_boxes_data_to_json(boxes_data, json_path_basename + ".json");

    env.log("Sorted boxes data :");
    print_boxes_data(boxes_sorted, env);
    save_boxes_data_to_json(boxes_sorted, json_path_basename + "-sorted.json");

    if (!DRY_RUN){
        sort(env, context, boxes_data, boxes_sorted, stats, cur_cursor, GAME_DELAY);
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);

}

}
}
}

