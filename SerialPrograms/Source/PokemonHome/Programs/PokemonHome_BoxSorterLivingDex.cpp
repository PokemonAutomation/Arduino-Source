/*  Home Box Sorter Living Dex
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <array>
#include <cmath>
#include <map>
#include <optional>
#include <sstream>
#include <vector>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Inference/Pokemon_TypeReader.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Pokemon_BoxCursor.h"
#include "Pokemon/Pokemon_CollectedPokemonInfo.h"
#include "PokemonHome/Inference/PokemonHome_ButtonDetector.h"
#include "PokemonHome_BoxNavigation.h"
#include "PokemonHome_BoxSorterLivingDex.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{
using namespace Pokemon;


const size_t MAX_HOME_BOXES = 200;


BoxSorterLivingDex_Descriptor::BoxSorterLivingDex_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonHome:BoxSorterLivingDex",
        STRING_POKEMON + " Home", "Box Sorter Living Dex",
        "Programs/PokemonHome/BoxSorterLivingDex.html",
        "Arrange boxes of " + STRING_POKEMON + " into living dex order.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct BoxSorterLivingDex_Descriptor::Stats : public StatsTracker{
    Stats()
        : pkmn(m_stats["Pokemon"])
        , empty(m_stats["Empty Slots"])
        , swaps(m_stats["Swaps"])
    {
        m_display_order.emplace_back(Stat("Pokemon"));
        m_display_order.emplace_back(Stat("Empty Slots"));
        m_display_order.emplace_back(Stat("Swaps"));
    }
    std::atomic<uint64_t>& pkmn;
    std::atomic<uint64_t>& empty;
    std::atomic<uint64_t>& swaps;
};

std::unique_ptr<StatsTracker> BoxSorterLivingDex_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

BoxSorterLivingDex::BoxSorterLivingDex()
    : LIVING_DEX_START_BOX(
        "<b>Living Dex Starting Box:</b><br>Box number where the living dex placement begins (1-indexed).",
        LockMode::LOCK_WHILE_RUNNING,
        1, 1, (uint16_t)MAX_HOME_BOXES
    )
    , REJECT_BOX_START(
        "<b>Reject Box Range Start:</b><br>First box of the range reserved for rejected or overflow " + STRING_POKEMON + ".",
        LockMode::LOCK_WHILE_RUNNING,
        1, 1, (uint16_t)MAX_HOME_BOXES
    )
    , REJECT_BOX_END(
        "<b>Reject Box Range End:</b><br>Last box of the range reserved for rejected or overflow " + STRING_POKEMON + ".",
        LockMode::LOCK_WHILE_RUNNING,
        1, 1, (uint16_t)MAX_HOME_BOXES
    )
    , SHINY_DEX(
        "<b>Shiny Dex:</b><br>Enable or disable shiny dex mode.",
        LockMode::LOCK_WHILE_RUNNING,
        false
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
    , OUTPUT_FILE(
          false,
          "<b>Output File:</b><br>JSON file basename to catalogue box info found by the program.",
          LockMode::LOCK_WHILE_RUNNING,
          "living_dex_order",
          "living_dex_order"
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
    PA_ADD_OPTION(LIVING_DEX_START_BOX);
    PA_ADD_OPTION(REJECT_BOX_START);
    PA_ADD_OPTION(REJECT_BOX_END);
    PA_ADD_OPTION(SHINY_DEX);
    PA_ADD_OPTION(VIDEO_DELAY);
    PA_ADD_OPTION(GAME_DELAY);
    PA_ADD_OPTION(OUTPUT_FILE);
    PA_ADD_OPTION(DRY_RUN);
    PA_ADD_OPTION(NOTIFICATIONS);
}

bool BoxSorterLivingDex::is_viable_for_dex(const LivingDexEntry& entry, const CollectedPokemonInfo& pokemonInfo) {
    if (SHINY_DEX != pokemonInfo.shiny){
        return false;
    }

    if (entry.nat_id != pokemonInfo.dex_number){
        return false;
    }

    if (entry.has_gender_difference) {
        if (entry.slug.ends_with("-female") && pokemonInfo.gender != StatsHuntGenderFilter::Female){
            return false;
        }

        if (entry.slug.ends_with("-male") && pokemonInfo.gender != StatsHuntGenderFilter::Male){
            return false;
        }
    }

    if (entry.primary_type != pokemonInfo.primaryType || entry.secondary_type != pokemonInfo.secondaryType){
        return false;
    }

    if (entry.form_slug == "gigantamax" && !pokemonInfo.gmax){
        return false;
    }

    return true;
}

[[nodiscard]] BoxCursor BoxSorterLivingDex::populate_box_data(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    std::vector<SortingRule> sort_preferences,
    std::vector<std::optional<CollectedPokemonInfo>>& boxes_data,
    size_t box_count,
    BoxCursor& nav_cursor
){
    BoxCursor dest_cursor;
    size_t starting_box(nav_cursor.box);
    VideoOverlaySet video_overlay_set(env.console);
    BoxViewWatcher box_view_watcher(&env.console.overlay());
    SummaryScreenWatcher summary_screen_watcher(&env.console.overlay());

    for (size_t box_idx = 0; box_idx < box_count; box_idx++){
        if (box_idx != 0){
            dest_cursor = BoxCursor(starting_box + box_idx, 0, 0);
            nav_cursor = move_cursor_to(env, context, nav_cursor, dest_cursor, GAME_DELAY);
        } else{
            env.log("Moving the cursor to the first slot on the page.");
            if (!go_to_first_slot(env, context, VIDEO_DELAY)){
                throw UserSetupError(
                    env.logger(),
                    "ERROR: Could not move cursor to the first slot, please consider adjusting delay."
                );
            }
        }

        context.wait_for_all_requests();
        const std::string log_msg = "Checking box " + std::to_string(box_idx + 1) + "/" + std::to_string(box_count);
        env.log(log_msg);
        env.console.overlay().add_log(log_msg);

        // Check box grid color stddev to find occupied slots and fill boxes_data with placeholder pokemon info.
        const std::array<size_t, 2> first_pokemon_slot = find_occupied_slots_in_box(env, context, boxes_data, sort_preferences);

        //enter the summary screen to read pokemon info
        if (first_pokemon_slot[0] != SIZE_MAX){
            // moving cursor to the first pokemon slot
            dest_cursor = { starting_box + box_idx, first_pokemon_slot[0], first_pokemon_slot[1] };
            nav_cursor = move_cursor_to(env, context, nav_cursor, dest_cursor, GAME_DELAY);

            env.add_overlay_log("Checking Summary...");

            pbf_press_button(context, BUTTON_A, 80ms, GAME_DELAY);
            context.wait_for_all_requests();
            video_overlay_set.clear();
            pbf_press_dpad(context, DPAD_DOWN, 80ms, GAME_DELAY);
            pbf_press_button(context, BUTTON_A, 80ms, 100ms);
            context.wait_for_all_requests();
            int ret = wait_until(env.console, context, Seconds(5), { summary_screen_watcher });
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
            ret = wait_until(env.console, context, Seconds(5), { box_view_watcher });
            if (ret != 0){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT, "BoxSorterLivingDex(): does not find box view after 5 sec", env.console
                );
            }
            video_overlay_set.clear();
        }
        video_overlay_set.clear();
    }

    return nav_cursor;
}

void BoxSorterLivingDex::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StartProgramChecks::check_performance_class_wired_or_wireless(context);

    std::string path = RESOURCE_PATH() + "PokemonHome/DexTemplates/living_form_dex_by_id_no_space.json";
    JsonValue json = load_json_file(path);
    const JsonArray& slugs = json.to_array_throw(path);

    std::vector<LivingDexEntry> living_dex_order;
    living_dex_order.reserve(slugs.size());
    for (const JsonValue& entry_value : slugs){
        const JsonObject& obj = entry_value.to_object_throw(path);
        LivingDexEntry entry;
        entry.species_slug = obj.get_string_throw("species_slug", path);
        entry.form_slug = obj.get_string_throw("form_slug", path);
        entry.nat_id = (uint16_t)obj.get_integer_throw("natID", path);
        entry.primary_type = POKEMON_TYPE_SLUGS().get_enum(obj.get_string_throw("primaryType", path), PokemonType::NONE);
        entry.secondary_type = POKEMON_TYPE_SLUGS().get_enum(obj.get_string_default("secondaryType", "none"), PokemonType::NONE);
        entry.slug = obj.get_string_throw("slug", path);
        entry.has_gender_difference = obj.get_boolean_throw("has_gender_difference", path);
        living_dex_order.emplace_back(std::move(entry));
    }

    // TODO allow users to have rules for the "best" version of a pokemon to be kept in the living dex box
    // It would be nice to add OT as most users that care about living dex also like to have it with their OT
    const std::vector<SortingRule> sort_preferences(
        {
            { SortingRuleType::DexNo, false } 
        });

    BoxSorterLivingDex_Descriptor::Stats& stats = env.current_stats<BoxSorterLivingDex_Descriptor::Stats>();

    // vector that will store data for each slot
    std::vector<std::optional<CollectedPokemonInfo>> living_dex_boxes_data;

    exit_menus(env, context, VIDEO_DELAY.get());

    BoxCursor dest_cursor;
    BoxCursor nav_cursor((LIVING_DEX_START_BOX - 1), 0, 0);

    size_t living_dex_box_count = (size_t)std::ceil((double)living_dex_order.size() / 30);

    nav_cursor = populate_box_data(env, context, sort_preferences, living_dex_boxes_data, living_dex_box_count, nav_cursor);

    dest_cursor = BoxCursor((REJECT_BOX_START - 1), 0, 0);
    nav_cursor = move_cursor_to(env, context, nav_cursor, dest_cursor, GAME_DELAY);
    context.wait_for_all_requests();

    std::vector<std::optional<CollectedPokemonInfo>> reject_boxes_data;
    size_t reject_box_count = REJECT_BOX_END - REJECT_BOX_START + 1;

    nav_cursor = populate_box_data(env, context, sort_preferences, reject_boxes_data, reject_box_count, nav_cursor);

    if (DRY_RUN){
        save_boxes_data_to_json(living_dex_boxes_data, "living_boxes_unsorted.json");
        save_boxes_data_to_json(reject_boxes_data, "reject_boxes_unsorted.json");
    }

    std::ostringstream ss;

    for (size_t i = 0; i < living_dex_order.size(); i++){
        bool found = false;
        LivingDexEntry entry = living_dex_order[i];
        env.log("Looking for " + entry.slug + " in the living dex boxes.");
        for(size_t j = i; j < living_dex_boxes_data.size(); j++){
            if (!living_dex_boxes_data[j].has_value()){
                continue;
            }

            if (!is_viable_for_dex(entry, *living_dex_boxes_data[j])){
                continue;
            }

            found = true;
            if (i == j){
                // already in the right spot, do nothing
                break;
            }
            else{

                BoxCursor pokemon_cursor = BoxCursor(LIVING_DEX_START_BOX - 1 + j / 30, (j / 6) % 5, j % 6);
                BoxCursor destination_cursor = BoxCursor(LIVING_DEX_START_BOX - 1 + i / 30, (i / 6) % 5, i % 6);

                if (!DRY_RUN){
                    ss << "Swapping " << living_dex_boxes_data[j] << " at " << pokemon_cursor << " and " << living_dex_boxes_data[i] << " at " << destination_cursor;
                    env.console.log(ss.str());
                    ss.str("");

                    nav_cursor = move_cursor_to(env, context, nav_cursor, pokemon_cursor, GAME_DELAY);
                    pbf_press_button(context, BUTTON_Y, 80ms, GAME_DELAY.get() + 240ms);

                    nav_cursor = move_cursor_to(env, context, nav_cursor, destination_cursor, GAME_DELAY);
                    pbf_press_button(context, BUTTON_Y, 80ms, GAME_DELAY.get() + 240ms);

                    context.wait_for_all_requests();

                    stats.swaps++;
                    env.update_stats();
                }

                std::swap(living_dex_boxes_data[j], living_dex_boxes_data[i]);  
            }

            break;
        }

        if (found){
            continue;
        }

        env.log("Could not find a pokemon for " + entry.slug + " in the living dex boxes, looking in reject boxes for a valid match.");

        for (size_t j = 0; j < reject_boxes_data.size(); j++){
            if (!reject_boxes_data[j].has_value()) {
                continue;
            }

            if (!is_viable_for_dex(entry, *reject_boxes_data[j])){
                continue;
            }

            found = true;

            BoxCursor pokemon_cursor = BoxCursor(REJECT_BOX_START - 1 + j / 30, (j / 6) % 5, j % 6);
            BoxCursor destination_cursor = BoxCursor(LIVING_DEX_START_BOX - 1 + i / 30, (i / 6) % 5, i % 6);
            
            if (!DRY_RUN){
                ss << "Swapping " << reject_boxes_data[j] << " at " << pokemon_cursor << " and " << living_dex_boxes_data[i] << " at " << destination_cursor;
                env.console.log(ss.str());
                ss.str("");
                nav_cursor = move_cursor_to(env, context, nav_cursor, pokemon_cursor, GAME_DELAY);
                pbf_press_button(context, BUTTON_Y, 80ms, GAME_DELAY.get() + 240ms);
                nav_cursor = move_cursor_to(env, context, nav_cursor, destination_cursor, GAME_DELAY);
                pbf_press_button(context, BUTTON_Y, 80ms, GAME_DELAY.get() + 240ms);
                context.wait_for_all_requests();
                stats.swaps++;
                env.update_stats();
            }

            std::swap(living_dex_boxes_data[i], reject_boxes_data[j]);

            break;
        }

        if (found){
            continue;
        }

        if (!living_dex_boxes_data[i].has_value()){
            continue;
        }

        // move the pokemon in the current living dex slot to the reject box or later in the living dex
        size_t empty_reject_slot = SIZE_MAX;
        size_t empty_living_dex_slot = SIZE_MAX;
        for (size_t j = 0; j < reject_boxes_data.size(); j++){
            if (!reject_boxes_data[j].has_value()){
                empty_reject_slot = j;
                break;
            }
        }
        if (empty_reject_slot == SIZE_MAX){
            env.log("No empty reject box slots available to move pokemon out of the way, placing in open slot in living dex for now.");

            for (size_t j = i + 1; j < living_dex_boxes_data.size(); j++){
                if (!living_dex_boxes_data[j].has_value()){
                    empty_living_dex_slot = j;
                    break;
                }
            }
        }
        BoxCursor pokemon_cursor = BoxCursor(LIVING_DEX_START_BOX - 1 + i / 30, (i / 6) % 5, i % 6);
        BoxCursor destination_cursor;
        
        if (empty_reject_slot != SIZE_MAX){
            destination_cursor = BoxCursor(REJECT_BOX_START - 1 + empty_reject_slot / 30, (empty_reject_slot / 6) % 5, empty_reject_slot % 6);
        }
        else if (empty_living_dex_slot != SIZE_MAX){
            destination_cursor = BoxCursor(LIVING_DEX_START_BOX - 1 + empty_living_dex_slot / 30, (empty_living_dex_slot / 6) % 5, empty_living_dex_slot % 6);
        }
        else{
            throw UserSetupError(
                env.logger(),
                "ERROR: No empty slots available to move pokemon out of the way, please consider removing some duplicates or increasing the reject box range."
            );
        }

        if (!DRY_RUN){
            ss << "Moving " << living_dex_boxes_data[i] << " at " << pokemon_cursor << " to " << destination_cursor;
            env.console.log(ss.str());
            ss.str("");
            nav_cursor = move_cursor_to(env, context, nav_cursor, pokemon_cursor, GAME_DELAY);
            pbf_press_button(context, BUTTON_Y, 80ms, GAME_DELAY.get() + 240ms);
            nav_cursor = move_cursor_to(env, context, nav_cursor, destination_cursor, GAME_DELAY);
            pbf_press_button(context, BUTTON_Y, 80ms, GAME_DELAY.get() + 240ms);
            context.wait_for_all_requests();
            stats.swaps++;
            env.update_stats();
        }

        if (empty_living_dex_slot != SIZE_MAX){
            std::swap(living_dex_boxes_data[i], living_dex_boxes_data[empty_living_dex_slot]);
        }
        else{
            std::swap(living_dex_boxes_data[i], reject_boxes_data[empty_reject_slot]);
        }
    }

    for (size_t i = living_dex_order.size(); i < living_dex_boxes_data.size(); i++){
        if (!living_dex_boxes_data[i].has_value()){
            continue;
        }

        size_t empty_reject_slot = SIZE_MAX;
        for (size_t j = 0; j < reject_boxes_data.size(); j++){
            if (!reject_boxes_data[j].has_value()){
                empty_reject_slot = j;
                break;
            }
        }

        if (empty_reject_slot == SIZE_MAX){
            break;
        }

        BoxCursor pokemon_cursor = BoxCursor(LIVING_DEX_START_BOX - 1 + i / 30, i % 5, i % 6);
        BoxCursor destination_cursor = BoxCursor(REJECT_BOX_START - 1 + empty_reject_slot / 30, (empty_reject_slot / 6) % 5, empty_reject_slot % 6);

        if (!DRY_RUN){
            ss << "Moving " << living_dex_boxes_data[i] << " at " << pokemon_cursor << " to " << destination_cursor;
            env.console.log(ss.str());
            ss.str("");
            nav_cursor = move_cursor_to(env, context, nav_cursor, pokemon_cursor, GAME_DELAY);
            pbf_press_button(context, BUTTON_Y, 80ms, GAME_DELAY.get() + 240ms);
            nav_cursor = move_cursor_to(env, context, nav_cursor, destination_cursor, GAME_DELAY);
            pbf_press_button(context, BUTTON_Y, 80ms, GAME_DELAY.get() + 240ms);
            context.wait_for_all_requests();
            stats.swaps++;
            env.update_stats();
        }
        
        std::swap(living_dex_boxes_data[i], reject_boxes_data[empty_reject_slot]);
        
    }

    if (DRY_RUN){
        save_boxes_data_to_json(living_dex_boxes_data, "living_boxes_sorted.json");
        save_boxes_data_to_json(reject_boxes_data, "reject_boxes_sorted.json");
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

}
}
}
