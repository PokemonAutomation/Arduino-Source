/*  Box Sorter Living Dex
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Filesystem/FileIO.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonFRLG_BoxOrganization.h"
#include "PokemonFRLG_BoxSorterLivingDex.h"
#include "VideoGames/PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_BoxSorterLivingDex.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

using namespace Pokemon;

BoxSorterLivingDex_Descriptor::BoxSorterLivingDex_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:BoxSorterLivingDex",
        STRING_POKEMON + " FRLG", "Box Sorter Living Dex",
        "Programs/PokemonFRLG/BoxSorterLivingDex.html",
        "Arrange boxes of " + STRING_POKEMON + " into living dex order. Start in Move " + STRING_POKEMON +
        " with an empty hand in the first box to organize. Uses English species names.",
        PanelQualityStanding::OFFICIAL_BETA,
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
    home_black_border_check(env.console, context);

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



}
}
}
