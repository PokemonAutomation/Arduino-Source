/*  Box Organization Helpers
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_BoxOrganization_H
#define PokemonAutomation_PokemonFRLG_BoxOrganization_H

#include <atomic>
#include <cstdint>
#include <optional>
#include <vector>
#include "CommonFramework/Language.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Pokemon_BoxCursor.h"
#include "Pokemon/Pokemon_CollectedPokemonInfo.h"
#include "VideoGames/PokemonFRLG/Inference/Box/PokemonFRLG_BoxSelectionModeDetector.h"

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

struct LivingDexEntry{
    uint16_t nat_id;
    std::string slug;
    bool has_gender_difference;
};

bool is_viable_for_dex(
    const LivingDexEntry& entry, const Pokemon::CollectedPokemonInfo& pokemon,
    bool obey_gender_differences, bool shiny_dex = false
);

std::vector<std::optional<Pokemon::CollectedPokemonInfo>> make_living_dex_order(
    Logger& logger,
    const std::vector<std::optional<Pokemon::CollectedPokemonInfo>>& boxes_data,
    const std::vector<LivingDexEntry>& living_dex_order,
    bool obey_gender_differences, bool shiny_dex = false
);

// List empty template slots in National Dex order, with one number and English name per line.
std::string make_missing_living_dex_report(
    const std::vector<std::optional<Pokemon::CollectedPokemonInfo>>& boxes_sorted,
    const std::vector<LivingDexEntry>& living_dex_order
);

std::vector<LivingDexEntry> load_living_dex_template(const std::string& path);

struct BoxOrganizationScan{
    std::vector<std::optional<Pokemon::CollectedPokemonInfo>> boxes_data;
    Pokemon::BoxCursor cursor;
    BoxSelectionMode mode;
};

// Detecting Shiny requires reading the summary screen, this is much slower than just reading the box data.
BoxOrganizationScan scan_box_data(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    Language language, const std::vector<Pokemon::SortingRule>& preferences, size_t num_boxes,
    bool read_shininess = false
);

void execute_box_sorting_plan(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    const BoxSortingPlan& plan, Pokemon::BoxCursor& cursor, BoxSelectionMode scan_mode
);


}
}
}
#endif
