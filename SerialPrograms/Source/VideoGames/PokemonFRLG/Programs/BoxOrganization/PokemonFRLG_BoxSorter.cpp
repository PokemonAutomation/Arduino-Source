/*  Box Sorter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonFRLG_BoxOrganization.h"
#include "PokemonFRLG_BoxSorter.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

using namespace Pokemon;

namespace{

constexpr uint16_t MAX_BOXES = 14;

}

BoxSorter_Descriptor::BoxSorter_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:BoxSorter",
        Pokemon::STRING_POKEMON + " FRLG", "Box Sorter",
        "",
        "Sort boxes of " + STRING_POKEMON + ". Start in Move " + STRING_POKEMON +
        " with an empty hand in the first box to sort. Uses English species names.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}


std::unique_ptr<StatsTracker> BoxSorter_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

BoxSorter::BoxSorter()
    : NUM_BOXES(
        "<b>Number of Boxes to Sort:</b><br>Starting with the current box and continuing to the right.",
        LockMode::LOCK_WHILE_RUNNING, 1, 1, MAX_BOXES
    )
    , SORT_TABLE(
        "<b>Sort Order Rules:</b><br>Sort order rules will be applied top to bottom.",
        {SortingRuleType::DexNo, SortingRuleType::Level, SortingRuleType::Gender}
    )
    , OUTPUT_FILE(
        false,
        "<b>Output File:</b><br>JSON file basename for the scanned and sorted box catalogues.",
        LockMode::LOCK_WHILE_RUNNING, "box_order", "box_order"
    )
    , DRY_RUN(
        "<b>Dry Run:</b><br>Scan boxes and write &lt;output_file&gt;.json and "
        "&lt;output_file&gt;-sorted.json without moving any " + STRING_POKEMON + ".",
        LockMode::LOCK_WHILE_RUNNING, false
    )
    , NOTIFICATIONS({&NOTIFICATION_PROGRAM_FINISH, &NOTIFICATION_ERROR_FATAL})
{
    PA_ADD_OPTION(NUM_BOXES);
    PA_ADD_OPTION(SORT_TABLE);
    PA_ADD_OPTION(OUTPUT_FILE);
    PA_ADD_OPTION(DRY_RUN);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void BoxSorter::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StartProgramChecks::check_performance_class_wired_or_wireless(context);

    const std::vector<SortingRule> preferences = SORT_TABLE.preferences();
    if (preferences.empty()){
        throw UserSetupError(env.console, "Select at least one sorting rule.");
    }
    const std::string output_file = OUTPUT_FILE;
    if (output_file.empty()){
        throw UserSetupError(env.console, "Specify an output file basename.");
    }

    auto& stats = env.current_stats<BoxSorter_Descriptor::Stats>();
    BoxOrganizationScan scan = scan_box_data(env, context, preferences, (uint16_t)NUM_BOXES);
    const auto& boxes_data = scan.boxes_data;

    // Stable ordering avoids unnecessary moves for Pokemon with identical sort keys.
    auto boxes_sorted = boxes_data;
    std::stable_sort(boxes_sorted.begin(), boxes_sorted.end());
    const BoxSortingPlan plan = make_box_sorting_plan(boxes_data, boxes_sorted);
    stats.compares += plan.compares;
    env.update_stats();
    save_boxes_data_to_json(boxes_data, output_file + ".json");
    save_boxes_data_to_json(boxes_sorted, output_file + "-sorted.json");
    env.log("Saved current and sorted box catalogues.");

    if (DRY_RUN){
        env.log("Dry run complete.");
        send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
        return;
    }

    env.log("Start sorting...");
    execute_box_sorting_plan(env, context, plan, scan.cursor, scan.mode);

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}
}
}
