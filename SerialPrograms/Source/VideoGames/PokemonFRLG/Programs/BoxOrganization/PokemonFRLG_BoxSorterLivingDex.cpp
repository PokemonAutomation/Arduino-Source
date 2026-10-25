/*  Box Sorter Living Dex
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Filesystem/FileIO.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "Pokemon/Resources/Pokemon_PokemonSlugs.h"
#include "PokemonFRLG_BoxOrganization.h"
#include "PokemonFRLG_BoxSorterLivingDex.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

using namespace Pokemon;

BoxSorterLivingDex_Descriptor::BoxSorterLivingDex_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:BoxSorterLivingDex",
        STRING_POKEMON + " FRLG", "Box Sorter Living Dex",
        "",
        "Arrange boxes of " + STRING_POKEMON + " into living dex order. Start in Move " + STRING_POKEMON +
        " with an empty hand in the first box to organize. Uses English species names.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

std::unique_ptr<StatsTracker> BoxSorterLivingDex_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

BoxSorterLivingDex::BoxSorterLivingDex()
    : NUM_BOXES(
        "<b>Number of Boxes to Organize:</b><br>Starting with the current box and continuing to the right. "
        "Include room after the template for duplicates and other Pokemon.",
        LockMode::LOCK_WHILE_RUNNING, 8, 1, 14
    )
    , LIVING_DEX_TEMPLATE(
        "<b>Living Dex Template:</b><br>Placement begins in the first slot of the current box. "
        "Missing species leave empty slots; extras follow the template. Forms are ignored.",
        {{LivingDexTemplate::Condensed, "frlg_living_dex_condensed", "FRLG Living Dex (Condensed)"}},
        LockMode::LOCK_WHILE_RUNNING, LivingDexTemplate::Condensed
    )
    , SHINY_DEX(
        "<b>Shiny Dex:</b><br>Only shiny Pokemon fill the living dex template. Non-shiny Pokemon follow the template with the extras.",
        LockMode::LOCK_WHILE_RUNNING, false
    )
    , DRY_RUN(
        "<b>Dry Run:</b><br>Scan boxes and write the current and organized box catalogues and "
        "frlg-living-dex-missing-pokemon.txt without moving any " + STRING_POKEMON + ".",
        LockMode::LOCK_WHILE_RUNNING, false
    )
    , NOTIFICATIONS({&NOTIFICATION_PROGRAM_FINISH, &NOTIFICATION_ERROR_FATAL})
{
    PA_ADD_OPTION(NUM_BOXES);
    PA_ADD_OPTION(LIVING_DEX_TEMPLATE);
    PA_ADD_OPTION(SHINY_DEX);
    PA_ADD_OPTION(DRY_RUN);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void BoxSorterLivingDex::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StartProgramChecks::check_performance_class_wired_or_wireless(context);

    std::vector<LivingDexEntry> living_dex_order;
    bool obey_gender_differences;
    switch (LIVING_DEX_TEMPLATE){
    case LivingDexTemplate::Condensed:
        living_dex_order = load_living_dex_template(
            RESOURCE_PATH() + "PokemonFRLG/Box/DexTemplates/frlg_living_dex_condensed.json"
        );
        obey_gender_differences = false;
        break;
    default:
        throw InternalProgramError(&env.logger(), PA_CURRENT_FUNCTION, "Invalid living dex template.");
    }
    const size_t slot_count = (size_t)NUM_BOXES * BOX_ROWS * BOX_COLS;
    if (living_dex_order.size() > slot_count){
        const size_t boxes_required = (living_dex_order.size() + BOX_ROWS * BOX_COLS - 1) / (BOX_ROWS * BOX_COLS);
        throw UserSetupError(env.console,
            "The living dex template requires at least " + std::to_string(boxes_required) + " boxes."
        );
    }

    const std::vector<SortingRule> preferences{{SortingRuleType::DexNo, false}};
    auto& stats = env.current_stats<BoxSorterLivingDex_Descriptor::Stats>();
    const bool shiny_dex = SHINY_DEX;
    BoxOrganizationScan scan = scan_box_data(env, context, preferences, (uint16_t)NUM_BOXES, shiny_dex);
    const auto boxes_sorted = make_living_dex_order(
        env.logger(), scan.boxes_data, living_dex_order, obey_gender_differences, shiny_dex
    );
    const BoxSortingPlan plan = make_box_sorting_plan(scan.boxes_data, boxes_sorted);
    stats.compares += plan.compares;
    env.update_stats();
    save_boxes_data_to_json(scan.boxes_data, "frlg-living-dex-box-order.json");
    save_boxes_data_to_json(boxes_sorted, "frlg-living-dex-box-order-sorted.json");
    env.log("Saved current and organized box catalogues.");

    if (DRY_RUN){
        env.log("Dry run complete.");
    }else{
        env.log("Start living dex organization...");
        execute_box_sorting_plan(env, context, plan, scan.cursor, scan.mode);
    }

    const std::string missing_path = "frlg-living-dex-missing-pokemon.txt";
    string_to_file(missing_path, make_missing_living_dex_report(boxes_sorted, living_dex_order));
    env.log("Saved missing Pokemon list to " + missing_path + ".");
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

std::string make_missing_living_dex_report(
    const std::vector<std::optional<CollectedPokemonInfo>>& boxes_sorted,
    const std::vector<LivingDexEntry>& living_dex_order
){
    if (living_dex_order.size() > boxes_sorted.size()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Living dex template exceeds organized box slots.");
    }
    std::vector<uint16_t> missing;
    for (size_t i = 0; i < living_dex_order.size(); i++){
        if (!boxes_sorted[i]){
            missing.push_back(living_dex_order[i].nat_id);
        }
    }
    std::sort(missing.begin(), missing.end());
    std::string report;
    for (uint16_t dex_number : missing){
        const std::string& slug = NATIONAL_DEX_SLUGS().at(dex_number - 1);
        report += std::to_string(dex_number) + " " + get_pokemon_name(slug).display_name() + "\n";
    }
    return report;
}

std::vector<LivingDexEntry> load_living_dex_template(const std::string& path){
    const JsonValue json = load_json_file(path);
    const JsonArray& entries = json.to_array_throw(path);
    std::vector<LivingDexEntry> order;
    order.reserve(entries.size());
    for (const JsonValue& value : entries){
        const JsonObject& entry = value.to_object_throw(path);
        const int64_t nat_id = entry.get_integer_throw("natID", path);
        if (nat_id < 1 || nat_id > 386){
            throw FileException(nullptr, PA_CURRENT_FUNCTION, "Invalid FRLG National Dex number.", path);
        }
        order.push_back({
            (uint16_t)nat_id,
            entry.get_string_throw("slug", path),
            entry.get_boolean_throw("has_gender_difference", path),
        });
    }
    if (order.empty()){
        throw FileException(nullptr, PA_CURRENT_FUNCTION, "Living dex template is empty.", path);
    }
    return order;
}

bool is_viable_for_dex(
    const LivingDexEntry& entry, const CollectedPokemonInfo& pokemon,
    bool obey_gender_differences, bool shiny_dex
){
    if (entry.nat_id != pokemon.dex_number || (shiny_dex && !pokemon.shiny)){
        return false;
    }
    if (obey_gender_differences && entry.has_gender_difference){
        if (entry.slug.ends_with("-female") && pokemon.gender != StatsHuntGenderFilter::Female){
            return false;
        }
        if (entry.slug.ends_with("-male") && pokemon.gender != StatsHuntGenderFilter::Male){
            return false;
        }
    }
    return true;
}

std::vector<std::optional<CollectedPokemonInfo>> make_living_dex_order(
    Logger& logger,
    const std::vector<std::optional<CollectedPokemonInfo>>& boxes_data,
    const std::vector<LivingDexEntry>& living_dex_order,
    bool obey_gender_differences, bool shiny_dex
){
    if (living_dex_order.size() > boxes_data.size()){
        throw UserSetupError(logger, "Select enough boxes to hold the living dex template.");
    }
    std::vector<std::optional<CollectedPokemonInfo>> ordered(boxes_data.size());
    std::vector<bool> used(boxes_data.size(), false);
    // Reserve all viable Pokemon already in place before considering duplicates.
    for (size_t i = 0; i < living_dex_order.size(); i++){
        if (boxes_data[i] && is_viable_for_dex(living_dex_order[i], *boxes_data[i], obey_gender_differences, shiny_dex)){
            ordered[i] = boxes_data[i];
            used[i] = true;
        }
    }
    for (size_t i = 0; i < living_dex_order.size(); i++){
        if (ordered[i]){
            continue;
        }
        for (size_t source = 0; source < boxes_data.size(); source++){
            if (!used[source] && boxes_data[source] &&
                is_viable_for_dex(living_dex_order[i], *boxes_data[source], obey_gender_differences, shiny_dex)
            ){
                ordered[i] = boxes_data[source];
                used[source] = true;
                break;
            }
        }
    }
    size_t destination = living_dex_order.size();
    for (size_t source = 0; source < boxes_data.size(); source++){
        if (used[source] || !boxes_data[source]){
            continue;
        }
        if (destination == ordered.size()){
            throw UserSetupError(logger,
                "Not enough space after the living dex template for duplicates and other Pokemon. "
                "Increase the number of boxes or remove extra Pokemon."
            );
        }
        ordered[destination++] = boxes_data[source];
    }
    return ordered;
}


}
}
}
