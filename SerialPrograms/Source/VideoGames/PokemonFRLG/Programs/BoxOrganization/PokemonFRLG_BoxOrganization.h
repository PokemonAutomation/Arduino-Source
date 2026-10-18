/*  Box Organization Helpers
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_BoxOrganization_H
#define PokemonAutomation_PokemonFRLG_BoxOrganization_H

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Pokemon_BoxCursor.h"
#include "Pokemon/Pokemon_CollectedPokemonInfo.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

struct BoxOrganizationStats : public StatsTracker{
    BoxOrganizationStats();

    std::atomic<uint64_t>& pokemon;
    std::atomic<uint64_t>& empty;
    std::atomic<uint64_t>& compares;
    std::atomic<uint64_t>& swaps;
};

struct BoxSortingPlan{
    // Pick up at the first slot, then place/swap at each remaining slot.
    // Every placement is final and each chain ends with an empty hand.
    std::vector<std::vector<size_t>> chains;
    uint64_t compares = 0;
};

BoxSortingPlan make_box_sorting_plan(
    const std::vector<std::optional<Pokemon::CollectedPokemonInfo>>& boxes_data,
    const std::vector<std::optional<Pokemon::CollectedPokemonInfo>>& boxes_sorted
);

enum class BoxSelectionMode;

struct BoxOrganizationScan{
    std::vector<std::optional<Pokemon::CollectedPokemonInfo>> boxes_data;
    Pokemon::BoxCursor cursor;
    BoxSelectionMode mode;
};

BoxOrganizationScan scan_box_data(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    const std::vector<Pokemon::SortingRule>& preferences, size_t num_boxes
);

void execute_box_sorting_plan(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    const BoxSortingPlan& plan, Pokemon::BoxCursor& cursor, BoxSelectionMode scan_mode
);


}
}
}
#endif
