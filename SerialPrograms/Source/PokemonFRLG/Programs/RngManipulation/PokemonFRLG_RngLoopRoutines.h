/*  RNG Loop Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_RngLoopRoutines_H
#define PokemonAutomation_PokemonFRLG_RngLoopRoutines_H

#include <atomic>
#include <functional>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include "CommonFramework/Language.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Pokemon_AdvRng.h"
#include "PokemonFRLG_RngCalibration.h"
#include "PokemonFRLG_RngStatsDatabase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

using namespace Pokemon;


struct EggCalibrationHistories{
    const RngCalibrations initial_calibrations;
    RngCalibrations calibrations;
    RngUncertainHistory wild_uncertain;
    RngUncertainHistory egg_uncertain;
    RngCalibrationHistory wild;
    RngCalibrationHistory held;
    RngCalibrationHistory pickup;

    EggCalibrationHistories(RngCalibrations calibrations)
    : initial_calibrations(calibrations)
    {};

    RngCalibrationHistory& frame(bool pickup_frame){ return pickup_frame ? pickup : held; }

    void clear_uncertain(){
        wild_uncertain.results.clear();
        wild_uncertain.calibrations.clear();
        egg_uncertain.results.clear();
        egg_uncertain.calibrations.clear();
    }
};

// Per-frame seed configuration: the nearby-seed list, the target's position in it, and the seed delay.
struct EggFrameTarget{
    uint64_t seed_delay;
    std::vector<uint16_t> seed_values;
    int16_t seed_position;
};
struct EggFrameTargets{
    EggFrameTarget held;
    EggFrameTarget pickup;

    const EggFrameTarget& frame(bool pickup_frame) const{ return pickup_frame ? pickup : held; }
};

// result of catching a wild encounter to identify the current seed
enum class WildCatchOutcome{
    shiny,
    failed,
    caught
};


// Adjust the calibrations for the upcoming reset for Egg RNG
void select_calibrations_for_frame(
    SingleSwitchProgramEnvironment& env,
    EggCalibrationHistories& hist,
    const std::vector<uint16_t>& seed_values,
    const int16_t& seed_position,
    uint64_t advances,
    bool pickup_frame,
    bool previously_hit_held_frame,
    uint16_t times_not_held
);

// Catch a wild encounter at the daycare pond and read it, to identify which seed was hit.
WildCatchOutcome catch_wild_for_seed_id(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    std::atomic<uint64_t>& errors,
    EventNotificationOption& error_notification,
    Language language,
    const WallClock& timestamp,
    uint64_t advances,
    int sweet_scent_from_last,
    uint64_t& balls_left,
    const std::set<std::string>& species_list,
    const RngStatsDatabase& stats_data,
    AdvObservedPokemon& pokemon,
    RngStats& species_stats,
    AdvRngFilters& filters,
    uint64_t& wild_advances_estimate,
    const std::function<void(const AdvRngFilters&)>& publish_filters
);

// Returns whether a calibration result confirms the target frame was hit. 
// When confirmed before locking in by saving, this prunes calibration_history 
// down to only the entries that hit the target.
bool confirm_held_frame_hit(
    RngCalibrationHistory& calibration_history,
    const RngUncertainHistory& egg_uncertain_history,
    bool locked_in,
    uint16_t target_seed,
    uint64_t target_advances
);

// Shared Rare Candy refinement loop used by RNG programs
std::vector<AdvRngState> refine_calibration_with_rare_candy(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    Language language,
    AdvObservedPokemon& pokemon,
    AdvRngFilters& filters,
    const BaseStats& base_stats,
    RngUncertainHistory& uncertain_history,
    RngCalibrationHistory& calibration_history,
    const RngCalibrations& calibrations,
    uint16_t max_history_length,
    uint64_t max_rare_candies,
    AdvRngMethod method,
    bool safari_zone,
    std::atomic<uint64_t>& errors,
    EventNotificationOption& error_notification,
    const std::function<std::vector<AdvRngState>(AdvRngFilters&)>& run_search,
    const std::function<void(const std::vector<AdvRngState>&)>& publish_hits,
    const std::function<void(const AdvRngFilters&)>& publish_filters,
    const std::function<bool(const std::vector<AdvRngState>&)>& result_determined
);

// Options describing the small differences between EggRng's held-frame and pickup-frame loops.
struct EggRefineOptions{
    bool take_first;
    uint32_t advance_radius;
    int from_last;
};

// Searches with the egg searcher (pairs), extracts one side of each pair, 
// and levels with Rare Candy until the result is determined (or can't be further narrowed down). 
// Returns a vector of possible hits after the search completes.
std::vector<AdvRngState> refine_egg_calibration(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    Language language,
    AdvObservedPokemon& observed_egg,
    AdvRngFilters& filters,
    const BaseStats& base_stats,
    RngUncertainHistory& uncertain_history,
    RngCalibrationHistory& calibration_history,
    const RngCalibrations& calibrations,
    uint16_t max_history_length,
    uint64_t candies_left,
    const EggRefineOptions& options,
    std::atomic<uint64_t>& errors,
    EventNotificationOption& error_notification,
    const std::function<std::vector<std::pair<AdvRngState, AdvRngState>>(AdvRngFilters&)>& run_search,
    const std::function<void(const std::vector<AdvRngState>&)>& publish_hits,
    const std::function<void(const AdvRngFilters&)>& publish_filters,
    const std::function<bool(const std::vector<std::pair<AdvRngState, AdvRngState>>&)>& indistinguishable
);

}
}
}
#endif
