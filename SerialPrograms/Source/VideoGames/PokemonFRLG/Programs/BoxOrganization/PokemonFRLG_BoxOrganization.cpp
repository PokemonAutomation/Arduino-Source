/*  Box Organization Helpers
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <sstream>
#include <utility>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/FrozenImageDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Resources/Pokemon_PokemonSlugs.h"
#include "VideoGames/PokemonFRLG/Inference/Box/PokemonFRLG_BoxCellOccupiedDetector.h"
#include "VideoGames/PokemonFRLG/Inference/Box/PokemonFRLG_BoxCellReader.h"
#include "VideoGames/PokemonFRLG/Inference/Box/PokemonFRLG_BoxGenderReader.h"
#include "VideoGames/PokemonFRLG/Inference/Box/PokemonFRLG_BoxSelectionModeDetector.h"
#include "VideoGames/PokemonFRLG/Inference/Menus/PokemonFRLG_SummaryDetector.h"
#include "VideoGames/PokemonFRLG/Inference/PokemonFRLG_SelectionArrowDetector.h"
#include "VideoGames/PokemonFRLG/Inference/PokemonFRLG_ShinySymbolDetector.h"
#include "VideoGames/PokemonFRLG/Inference/PokemonFRLG_StatsReader.h"
#include "VideoGames/PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_BoxOrganization.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

using namespace Pokemon;
using namespace std::chrono_literals;

namespace{

template <typename T>
std::string level_str(const T& level){
    if constexpr (requires { level.has_value(); }){
        return level.has_value() ? std::to_string(*level) : "?";
    }else{
        return std::to_string(level);
    }
}


void move_cursor_to(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    const BoxSelectionModeDetector& detector, BoxCursor& cursor, const BoxCursor& destination,
    BoxSelectionMode mode
){
    std::ostringstream ss;
    ss << "Moving cursor from " << cursor << " to " << destination;
    env.log(ss.str());

    while (cursor.box != destination.box){
        const bool forward = cursor.box < destination.box;
        pbf_press_button(context, forward ? BUTTON_R : BUTTON_L, 80ms, 600ms);
        context.wait_for_all_requests();
        if (forward){
            cursor.box++;
        }else{
            cursor.box--;
        }
    }

    // Blindly move the cursor to the destination cell. Verify destination, and retry if necessary.
    for (size_t attempts = 0; attempts < 5; attempts++){
        const bool down = cursor.row < destination.row;
        const size_t row_steps = down ? destination.row - cursor.row : cursor.row - destination.row;
        for (size_t step = 0; step < row_steps; step++){
            pbf_press_dpad(context, down ? DPAD_DOWN : DPAD_UP, 80ms, 240ms);
        }
        const size_t right_steps = (destination.column + BOX_COLS - cursor.column) % BOX_COLS;
        const size_t left_steps = (cursor.column + BOX_COLS - destination.column) % BOX_COLS;
        const bool right = right_steps <= left_steps;
        const size_t column_steps = right ? right_steps : left_steps;
        for (size_t step = 0; step < column_steps; step++){
            pbf_press_dpad(context, right ? DPAD_RIGHT : DPAD_LEFT, 80ms, 240ms);
        }

        pbf_wait(context, 200ms);
        context.wait_for_all_requests();
        const auto selection = detector.detect(env.console.video().snapshot());
        if (!selection || selection->location != BoxSelectionLocation::Grid || selection->mode != mode){
            OperationFailedExceptionWithScreenshot::fire(
                ErrorReportMode::SEND_ERROR_REPORT,
                "Box Sorter: Unable to detect the expected selection hand in the box grid.", env.console
            );
        }
        cursor.row = selection->row;
        cursor.column = selection->column;
        if (cursor.row == destination.row && cursor.column == destination.column){
            return;
        }
    }
    OperationFailedExceptionWithScreenshot::fire(
        ErrorReportMode::SEND_ERROR_REPORT, "Box Sorter: Unable to reach the requested box cell.", env.console
    );
}

BoxSelection wait_for_normal_selection(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context
){
    BoxSelectionModeWatcher selection(COLOR_RED, BoxSelectionMode::Normal, BoxSelectionLocation::Grid);
    if (wait_until(env.console, context, 5s, {selection}) < 0){
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            "Box Sorter: Unable to detect the regular selection hand in the box grid.", env.console
        );
    }
    return *selection.last_detected();
}

// Wait for the summary animation to finish and verify summary page detected.
void wait_for_summary_animation(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    FrozenImageDetector still(COLOR_CYAN, GameSettings::instance().GAME_BOX, 500ms, 2.0);
    if (wait_until(env.console, context, 10s, {still}) < 0){
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            "Box Sorter: Summary animation did not finish.", env.console
        );
    }
    SummaryDetector summary(COLOR_RED);
    if (!summary.detect(env.console.video().snapshot())){
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            "Box Sorter: Expected the first summary page.", env.console
        );
    }
}

void scan_box_shininess(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    const BoxSelectionModeDetector& selection_detector, BoxCursor& cursor,
    std::vector<std::optional<CollectedPokemonInfo>>& boxes_data
){
    const size_t box_start = cursor.box * BOX_ROWS * BOX_COLS;
    std::vector<size_t> occupied;
    for (size_t index = box_start; index < boxes_data.size(); index++){
        if (boxes_data[index]){
            occupied.push_back(index);
        }
    }
    if (occupied.empty()){
        return;
    }

    env.log("Reading shininess in box " + std::to_string(cursor.box + 1));
    move_cursor_to(env, context, selection_detector, cursor, BoxCursor(occupied.back()), BoxSelectionMode::Normal);
    SelectionArrowWatcher move_arrow(COLOR_RED, &env.console.overlay(), {0.6259, 0.1621, 0.0373, 0.0906});
    SelectionArrowWatcher summary_arrow(COLOR_RED, &env.console.overlay(), {0.6267, 0.2635, 0.0373, 0.0799});
    pbf_press_button(context, BUTTON_A, 80ms, 200ms);
    context.wait_for_all_requests();
    if (wait_until(env.console, context, 5s, {move_arrow}) < 0){
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT, "Box Sorter: Move menu arrow was not detected.", env.console
        );
    }
    pbf_press_dpad(context, DPAD_DOWN, 80ms, 200ms);
    context.wait_for_all_requests();
    if (wait_until(env.console, context, 5s, {summary_arrow}) < 0){
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT, "Box Sorter: Summary menu arrow was not detected.", env.console
        );
    }
    pbf_press_button(context, BUTTON_A, 80ms, 300ms);
    context.wait_for_all_requests();
    SummaryWatcher summary(COLOR_RED);
    if (wait_until(env.console, context, 5s, {summary}) < 0){
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT, "Box Sorter: Unable to open the summary screen.", env.console
        );
    }
    wait_for_summary_animation(env, context);

    StatsReader reader;
    ShinySymbolDetector shiny_detector(COLOR_YELLOW);
    VideoOverlaySet overlays(env.console.overlay());
    shiny_detector.make_overlays(overlays);
    for (size_t remaining = occupied.size(); remaining > 0; remaining--){
        const size_t index = occupied[remaining - 1];
        auto& pokemon = *boxes_data[index];
        const VideoSnapshot screen = env.console.video().snapshot();
        PokemonFRLG_Stats current_stats;
        reader.read_page1(env.logger(), Language::English, screen, current_stats);
        if (current_stats.name != pokemon.name_slug || current_stats.level != pokemon.level){
            std::ostringstream ss;
            ss << "Box Sorter: Summary species mismatch at " << BoxCursor(index)
               << " (expected: " << pokemon.name_slug << " (level " << level_str(pokemon.level)
                              << "), read: " << current_stats.name << " (level " << level_str(current_stats.level) << ")).";
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT, ss.str(), env.console);
        }
        pokemon.shiny = shiny_detector.read(env.logger(), screen);
        pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
        context.wait_for_all_requests();
        wait_for_summary_animation(env, context);
    }
    pbf_press_button(context, BUTTON_B, 80ms, 300ms);
    context.wait_for_all_requests();
    const BoxSelection selection = wait_for_normal_selection(env, context);
    cursor.row = selection.row;
    cursor.column = selection.column;
    const BoxCursor first(occupied.front());
    if (cursor.row != first.row || cursor.column != first.column){
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            "Box Sorter: Summary scan did not return to the first occupied cell.", env.console
        );
    }
}

}

BoxOrganizationStats::BoxOrganizationStats()
    : pokemon(m_stats["Pokemon"])
    , empty(m_stats["Empty Slots"])
    , compares(m_stats["Compares"])
    , swaps(m_stats["Swaps"])
{
    m_display_order.emplace_back("Pokemon");
    m_display_order.emplace_back("Empty Slots");
    m_display_order.emplace_back("Compares");
    m_display_order.emplace_back("Swaps");
}

BoxSortingPlan make_box_sorting_plan(
    const std::vector<std::optional<CollectedPokemonInfo>>& boxes_data,
    const std::vector<std::optional<CollectedPokemonInfo>>& boxes_sorted
){
    const size_t count = boxes_data.size();
    if (boxes_sorted.size() != count){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Box catalogue sizes do not match.");
    }
    BoxSortingPlan plan;
    // Give each Pokemon a unique source index, including identical duplicates.
    std::vector<size_t> destinations(count, count);
    std::vector<bool> claimed(count, false);
    std::vector<std::optional<size_t>> slots(count);
    for (size_t source = 0; source < count; source++){
        if (!boxes_data[source]){
            continue;
        }
        slots[source] = source;
        plan.compares++;
        if (boxes_data[source] == boxes_sorted[source]){
            // Leave Pokemon already in a valid final slot untouched.
            destinations[source] = source;
            claimed[source] = true;
        }
    }
    for (size_t source = 0; source < count; source++){
        if (!boxes_data[source] || destinations[source] != count){
            continue;
        }
        for (size_t destination = 0; destination < count; destination++){
            if (claimed[destination]){
                continue;
            }
            plan.compares++;
            if (boxes_data[source] == boxes_sorted[destination]){
                destinations[source] = destination;
                claimed[destination] = true;
                break;
            }
        }
        if (destinations[source] == count){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Pokemon missing from sorted box catalogue.");
        }
    }
    for (size_t destination = 0; destination < count; destination++){
        if (boxes_sorted[destination] && !claimed[destination]){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Sorted Pokemon missing from box catalogue.");
        }
    }

    // Start open chains at slots that must end empty, then handle closed cycles.
    // This avoids splitting a chain into several separate pickups.
    for (size_t pass = 0; pass < 2; pass++){
        for (size_t source = 0; source < count; source++){
            if ((pass == 0 && boxes_sorted[source]) || !slots[source] || destinations[*slots[source]] == source){
                continue;
            }
            std::vector<size_t> chain{source};
            auto held = slots[source];
            slots[source].reset();
            while (held){
                const size_t destination = destinations[*held];
                chain.push_back(destination);
                // Place the held Pokemon in its final slot and follow the displaced
                // Pokemon to its own destination until reaching an empty cell.
                std::swap(held, slots[destination]);
            }
            plan.chains.emplace_back(std::move(chain));
        }
    }
    return plan;
}


BoxOrganizationScan scan_box_data(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    const std::vector<SortingRule>& preferences, size_t num_boxes, bool read_shininess
){
    read_shininess = read_shininess || std::any_of(preferences.begin(), preferences.end(), [](const SortingRule& rule){
        return rule.sort_type == SortingRuleType::Shiny;
    });
    auto& stats = env.current_stats<BoxOrganizationStats>();
    const size_t slot_count = num_boxes * BOX_ROWS * BOX_COLS;
    BoxSelectionModeDetector selection_detector;
    BoxCellOccupiedDetector occupied_detector(COLOR_BLUE);
    BoxCellReader cell_reader;
    BoxGenderReader gender_reader;
    VideoOverlaySet overlays(env.console.overlay());
    selection_detector.make_overlays(overlays);
    occupied_detector.make_overlays(overlays);
    cell_reader.make_overlays(overlays);
    gender_reader.make_overlays(overlays);

    context.wait_for_all_requests();

    // Get selection hand into starting position
    auto selection = selection_detector.detect(env.console.video().snapshot());
    if (!selection){
        throw UserSetupError(env.console, "Unable to detect the selection hand. Start in Move Pokemon with an empty hand.");
    }
    if (selection->closed){
        throw UserSetupError(env.console, "Cannot start while holding a Pokemon. Put it down before starting the program.");
    }
    if (selection->location != BoxSelectionLocation::Grid){
        // X moves the hand to the box title from anywhere.
        pbf_press_button(context, BUTTON_X, 80ms, 400ms);
        context.wait_for_all_requests();
        selection = selection_detector.detect(env.console.video().snapshot());
        if (!selection || selection->location != BoxSelectionLocation::BoxTile){
            throw UserSetupError(env.console, "Unable to reach the box title. Start in Move Pokemon with an empty hand.");
        }
        pbf_press_dpad(context, DPAD_DOWN, 80ms, 400ms);
        context.wait_for_all_requests();
        selection = selection_detector.detect(env.console.video().snapshot());
    }
    if (!selection || selection->location != BoxSelectionLocation::Grid){
        throw UserSetupError(env.console, "Unable to enter the box grid from the box title.");
    }
    BoxCursor cursor(0, selection->row, selection->column);
    if (read_shininess && selection->mode != BoxSelectionMode::Normal){
        pbf_press_button(context, BUTTON_MINUS, 80ms, 300ms);
        context.wait_for_all_requests();
        selection = wait_for_normal_selection(env, context);
        cursor.row = selection->row;
        cursor.column = selection->column;
    }
    const BoxSelectionMode scan_mode = selection->mode;
    move_cursor_to(env, context, selection_detector, cursor, BoxCursor(0, 0, 0), scan_mode);

    std::vector<std::optional<CollectedPokemonInfo>> boxes_data;
    boxes_data.reserve(slot_count);

    const size_t slots_per_box = BOX_ROWS * BOX_COLS;
    for (size_t box = 0; box < num_boxes; box++){
        const std::string message = "Checking box " + std::to_string(box + 1)
            + "/" + std::to_string(num_boxes);
        env.log(message);
        env.console.overlay().add_log(message);
        for (size_t slot = 0; slot < slots_per_box; slot++){
            const BoxCursor destination(box * slots_per_box + slot);
            move_cursor_to(env, context, selection_detector, cursor, destination, scan_mode);
            pbf_wait(context, 200ms);
            context.wait_for_all_requests();
            const VideoSnapshot screen = env.console.video().snapshot();
            if (!occupied_detector.detect(screen)){
                boxes_data.emplace_back(std::nullopt);
                stats.empty++;
                env.update_stats();
                continue;
            }

            const std::string name = cell_reader.read_name(env.logger(), Language::English, screen);
            const int level = cell_reader.read_level(env.logger(), screen);
            const auto& dex_numbers = SLUGS_TO_NATIONAL_DEX();
            const auto dex = dex_numbers.find(name);
            if (dex == dex_numbers.end() || dex->second == 0 || dex->second > 386 || level < 1 || level > 100){
                std::ostringstream ss;
                ss << "Box Sorter: Unable to read species/level at " << cursor
                   << " (species: " << name << ", level: " << level << ").";
                OperationFailedExceptionWithScreenshot::fire(
                    ErrorReportMode::SEND_ERROR_REPORT, ss.str(), env.console
                );
            }

            CollectedPokemonInfo pokemon;
            pokemon.preferences = &preferences;
            pokemon.dex_number = (uint16_t)dex->second;
            pokemon.name_slug = name;
            pokemon.level = (uint8_t)level;
            pokemon.gender = gender_reader.read(screen);
            boxes_data.emplace_back(std::move(pokemon));
            env.console.overlay().add_log(create_overlay_info(*boxes_data.back()) + " Lv. " + std::to_string(level));
            stats.pokemon++;
            env.update_stats();
        }
        if (read_shininess){
            scan_box_shininess(env, context, selection_detector, cursor, boxes_data);
        }
    }
    return {std::move(boxes_data), cursor, scan_mode};
}

void execute_box_sorting_plan(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    const BoxSortingPlan& plan, BoxCursor& cursor, BoxSelectionMode scan_mode
){
    auto& stats = env.current_stats<BoxOrganizationStats>();
    BoxSelectionModeDetector selection_detector;
    VideoOverlaySet overlays(env.console.overlay());
    selection_detector.make_overlays(overlays);
    if (scan_mode != BoxSelectionMode::Relocate){
        pbf_press_button(context, BUTTON_MINUS, 80ms, 300ms);
        context.wait_for_all_requests();
    }

    for (const auto& chain : plan.chains){
        for (size_t step = 0; step < chain.size(); step++){
            const BoxCursor destination(chain[step]);
            std::ostringstream ss;
            ss << (step == 0 ? "Picking up Pokemon at " : "Placing Pokemon at ") << destination;
            env.log(ss.str());
            move_cursor_to(env, context, selection_detector, cursor, destination, BoxSelectionMode::Relocate);
            pbf_press_button(context, BUTTON_A, 80ms, 400ms);
            context.wait_for_all_requests();
            if (step != 0){
                stats.swaps++;
                env.update_stats();
            }
        }
    }

}


}
}
}
