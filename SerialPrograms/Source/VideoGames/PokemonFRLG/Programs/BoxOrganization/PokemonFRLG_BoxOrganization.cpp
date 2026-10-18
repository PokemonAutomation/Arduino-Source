/*  Box Organization Helpers
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <sstream>
#include <utility>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Resources/Pokemon_PokemonSlugs.h"
#include "VideoGames/PokemonFRLG/Inference/Box/PokemonFRLG_BoxCellOccupiedDetector.h"
#include "VideoGames/PokemonFRLG/Inference/Box/PokemonFRLG_BoxCellReader.h"
#include "VideoGames/PokemonFRLG/Inference/Box/PokemonFRLG_BoxGenderReader.h"
#include "VideoGames/PokemonFRLG/Inference/Box/PokemonFRLG_BoxSelectionModeDetector.h"
#include "PokemonFRLG_BoxOrganization.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

using namespace Pokemon;
using namespace std::chrono_literals;

namespace{

// The first scanned box is box 0, regardless of its in-game name/number.
// Keep all movement inside the grid so the party and box-title controls are avoided.
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
    const std::vector<SortingRule>& preferences, size_t num_boxes
){
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
    const BoxSelectionMode scan_mode = selection->mode;
    move_cursor_to(env, context, selection_detector, cursor, BoxCursor(0, 0, 0), scan_mode);

    std::vector<std::optional<CollectedPokemonInfo>> boxes_data;
    boxes_data.reserve(slot_count);

    for (size_t index = 0; index < slot_count; index++){
        const BoxCursor destination(index);
        if (destination.row == 0 && destination.column == 0){
            const std::string message = "Checking box " + std::to_string(destination.box + 1)
                + "/" + std::to_string(num_boxes);
            env.log(message);
            env.console.overlay().add_log(message);
        }
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
