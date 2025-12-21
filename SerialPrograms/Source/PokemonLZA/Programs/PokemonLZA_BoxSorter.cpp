/*  LZA Box Sorter
 *
 *  From: https://github.com/PokemonAutomation/
 *
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
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "Pokemon/Pokemon_BoxCursor.h"
#include "Pokemon/Pokemon_CollectedPokemonInfo.h"
#include "PokemonHome/Inference/PokemonHome_BoxGenderDetector.h"
#include "PokemonHome/Inference/PokemonHome_BallReader.h"
#include "PokemonLZA/Inference/Boxes/PokemonLZA_BoxDetection.h"
#include "PokemonLZA/Inference/Boxes/PokemonLZA_BoxInfoDetector.h"
#include "PokemonLZA/Resources/PokemonLZA_AvailablePokemon.h"
#include "PokemonLZA_BoxSorter.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{
using namespace Pokemon;



const size_t MAX_BOXES = 32;

BoxSorter_Descriptor::BoxSorter_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:BoxSorter",
        STRING_POKEMON + " Legends: Z-A", "Box Sorter",
        "Programs/PokemonLZA/BoxSorter.html",
        "Order boxes of " + STRING_POKEMON + ".",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {}
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
    : NUM_BOXES(
        "<b>Number of Boxes to Sort:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        1, 1, MAX_BOXES
        )
    , SORT_TABLE(
          "<b>Sort Order Rules:</b><br>Sort order rules will be applied top to bottom.",
          {SortingRuleType::DexNo, SortingRuleType::Shiny, SortingRuleType::Alpha}
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
    PA_ADD_OPTION(NUM_BOXES); //number of boxes to check and sort
    PA_ADD_OPTION(SORT_TABLE);
    PA_ADD_OPTION(OUTPUT_FILE);
    PA_ADD_OPTION(DRY_RUN);
    PA_ADD_OPTION(NOTIFICATIONS);
}


//Move the cursor to the given coordinates, knowing current pos via the cursor struct
[[nodiscard]] BoxCursor move_cursor_to(SingleSwitchProgramEnvironment& env, ProControllerContext& context, const BoxCursor& cur_cursor, const BoxCursor& dest_cursor, bool holding_pokemon = false){

    std::ostringstream ss;
    ss << "Moving cursor from " << cur_cursor << " to " << dest_cursor;
    env.console.log(ss.str());

    uint16_t GAME_DELAY = 30;
    // TODO: shortest path movement though pages, boxes
    for (size_t i = cur_cursor.box; i < dest_cursor.box; ++i){
        pbf_press_button(context, BUTTON_R, 10, GAME_DELAY+30);
    }
    for (size_t i = dest_cursor.box; i < cur_cursor.box; ++i){
        pbf_press_button(context, BUTTON_L, 10, GAME_DELAY+30);
    }

    BoxDetector box_detector(COLOR_RED, &env.console.overlay());
    box_detector.move_cursor(
        env.program_info(),
        env.console,
        context,
        (uint8_t)dest_cursor.row + 1,
        (uint8_t)dest_cursor.column,
        holding_pokemon
    );

    return dest_cursor;
}

void print_boxes_data(const std::vector<std::optional<CollectedPokemonInfo>>& boxes_data, SingleSwitchProgramEnvironment& env){
    std::ostringstream ss;
    for (const std::optional<CollectedPokemonInfo>& pokemon : boxes_data){
        ss << pokemon << "\n";
    }
    env.console.log(ss.str());
}

std::string create_overlay_info(const CollectedPokemonInfo& pokemon, const BoxDexNumberDetector& dex_number_detector){
    const std::string& display_name = get_pokemon_name(pokemon.name_slug).display_name();
    
    std::string overlay_log = dex_number_detector.dex_type() == DexType::HYPERSPACE ? "H" : "L";
    
    char dex_str[6];
    snprintf(dex_str, sizeof(dex_str), "%03d", dex_number_detector.dex_number());
    overlay_log += std::string(dex_str) + " " + display_name;
    if(pokemon.gender == StatsHuntGenderFilter::Male){
        overlay_log += " " + UNICODE_MALE;
    } else if (pokemon.gender == StatsHuntGenderFilter::Female){
        overlay_log += " " + UNICODE_FEMALE;
    }
    if (pokemon.shiny){
        overlay_log += " *";
    }
    if (pokemon.alpha){
        overlay_log += " " + UNICODE_ALPHA;
    }
    return overlay_log;
}

void sort(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    std::vector<std::optional<CollectedPokemonInfo>> boxes_data,
    std::vector<std::optional<CollectedPokemonInfo>> boxes_sorted,
    BoxSorter_Descriptor::Stats& stats,
    BoxCursor& cur_cursor
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
                bool holding_pokemon = false;
                cur_cursor = move_cursor_to(env, context, cur_cursor, cursor, holding_pokemon);
                pbf_press_button(context, BUTTON_Y, 10, 60);
                context.wait_for_all_requests();

                //moving to destination to place it or swap it
                holding_pokemon = true;
                cur_cursor = move_cursor_to(env, context, cur_cursor, cursor_s, holding_pokemon);
                pbf_press_button(context, BUTTON_Y, 10, 60);
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

    std::vector<SortingRule> sort_preferences = SORT_TABLE.preferences();
    if (sort_preferences.empty()){
        throw UserSetupError(env.console, "At least one sorting method selection needs to be made!");
    }

    BoxSorter_Descriptor::Stats& stats = env.current_stats<BoxSorter_Descriptor::Stats>();

    // vector that will store data for each slot
    std::vector<std::optional<CollectedPokemonInfo>> boxes_data;

    BoxCursor cur_cursor{static_cast<uint16_t>(NUM_BOXES-1), 0, 0};

    VideoOverlaySet video_overlay_set(env.console);

    std::ostringstream ss;

    uint16_t VIDEO_DELAY = 50;

    BoxDetector box_detector(COLOR_RED, &env.console.overlay());
    // BoxPageInfoWatcher info_watcher(&env.console.overlay());
    // SomethingInBoxCellWatcher non_empty_watcher(COLOR_BLUE, &env.console.overlay());
    BoxShinyDetector shiny_detector(COLOR_RED, &env.console.overlay());
    BoxAlphaDetector alpha_detector(COLOR_RED, &env.console.overlay());
    SomethingInBoxCellDetector non_empty_detector(COLOR_BLUE, &env.console.overlay());
    BoxDexNumberDetector dex_number_detector(env.console);

    box_detector.make_overlays(video_overlay_set);
    shiny_detector.make_overlays(video_overlay_set);
    alpha_detector.make_overlays(video_overlay_set);
    non_empty_detector.make_overlays(video_overlay_set);
    dex_number_detector.make_overlays(video_overlay_set);

    //cycle through each box
    for (size_t box_idx = 0; box_idx < NUM_BOXES; box_idx++){
        if(box_idx != 0){
            // Press button R to move to next box
            pbf_press_button(context, BUTTON_R, 10, VIDEO_DELAY+100);
        }else{
            // Moving the cursor to the first slot in the box
            box_detector.move_cursor(env.program_info(), env.console, context, 1, 0);
        }

        context.wait_for_all_requests();
        const std::string log_msg = "Checking box " + std::to_string(box_idx+1) + "/" + std::to_string(NUM_BOXES);
        env.log(log_msg);
        env.console.overlay().add_log(log_msg);

        // Box grid to find empty slots (red boxes) and fill boxes_data with value to check or not for pokemon dex number

        ss << "\n";

        int num_empty_slots = 0;
        for (size_t row = 0; row < BOX_ROWS; row++){
            for (size_t column = 0; column < BOX_COLS; column++){
                box_detector.move_cursor(
                    env.program_info(),
                    env.console,
                    context,
                    (uint8_t)row + 1,
                    (uint8_t)column
                );
                
                pbf_wait(context, 100ms); // wait some time for the pokemon info to be updated
                VideoSnapshot screen = env.console.video().snapshot();
                
                if(non_empty_detector.detect(screen)){
                    stats.pkmn++;
                    env.update_stats();

                    bool dex_number_detected = dex_number_detector.detect(screen);
                    if (!dex_number_detected){
                        OperationFailedException::fire(
                            ErrorReport::SEND_ERROR_REPORT,
                            "BoxSorting Check Summary: Unable to read a correct dex number, found: " + std::to_string(dex_number_detector.dex_number_when_error()),
                            env.console
                        );
                    }

                    // XXX TODO: change code to use regional dex, both Lumiose and Hyperspace
                    uint16_t dex_number = dex_number_detector.dex_number();
                    std::string name_slug;
                    // env.add_overlay_log(std::to_string(dex_number_detector.dex_type_color_ratio()));
                    if (dex_number_detector.dex_type() == DexType::HYPERSPACE){
                        name_slug = HYPERSPACE_DEX_SLUGS()[dex_number-1];
                        dex_number += (uint16_t)LUMIOSE_DEX_SLUGS().size();
                    } else{
                        name_slug = LUMIOSE_DEX_SLUGS()[dex_number-1];
                    }
                    
                    boxes_data.push_back(
                        CollectedPokemonInfo{
                            .preferences = &sort_preferences,
                            .dex_number = static_cast<uint16_t>(dex_number),
                            .name_slug = name_slug,
                            .shiny = shiny_detector.detect(screen),
                            .alpha = alpha_detector.detect(screen),
                        }
                        );
                    ss << "\u2705 " ;    //  checkbox
                    env.add_overlay_log(create_overlay_info(*boxes_data.back(), dex_number_detector));
                }else{
                    stats.empty++;
                    num_empty_slots++;
                    env.add_overlay_log("Empty Slot");
                    env.update_stats();
                    boxes_data.push_back(std::nullopt); //empty optional to make sorting easier later
                    ss << "\u274c " ;    //  "X"
                }
            }
            ss << "\n";
        }
        env.console.log(ss.str());
        env.add_overlay_log("Empty: " + std::to_string(num_empty_slots) + "/30");
        ss.str("");
    } // end box_idx
        
    // copy boxes data to sort
    std::vector<std::optional<CollectedPokemonInfo>> boxes_sorted = boxes_data;

    // sorting copy of boxes_data
    std::sort(boxes_sorted.begin(), boxes_sorted.end());

    env.console.log("Current boxes data :");
    const std::string json_path_basename = OUTPUT_FILE;
    save_boxes_data_to_json(boxes_data, json_path_basename + ".json");

    env.log("Sorted boxes data :");
    print_boxes_data(boxes_sorted, env);
    save_boxes_data_to_json(boxes_sorted, json_path_basename + "-sorted.json");

    if (!DRY_RUN){
        sort(env, context, boxes_data, boxes_sorted, stats, cur_cursor);
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);

}

}
}
}
