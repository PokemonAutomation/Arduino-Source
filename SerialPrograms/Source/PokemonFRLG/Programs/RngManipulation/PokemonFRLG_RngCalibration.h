/*  RNG Calibration
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_RngCalibration_H
#define PokemonAutomation_PokemonFRLG_RngCalibration_H

#include <string>
#include <algorithm>
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "Pokemon/Pokemon_AdvRng.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

using namespace Pokemon;


struct RngAdvanceHistory{
    std::vector<double> seed_calibrations;
    std::vector<std::vector<AdvRngState>> results;
};

struct RngCalibrationHistory{
    std::vector<double> seed_calibrations;
    std::vector<double> advance_calibrations;
    std::vector<double> continue_screen_adjustments;
    std::vector<AdvRngState> results;
};


void check_seed_validity(ConsoleHandle& console, std::string seed_string);
uint16_t parse_seed(ConsoleHandle& console, std::string seed_string);
std::vector<uint16_t> parse_seed_list(ConsoleHandle& console, std::string seed_list_string);
int16_t seed_position_in_list(uint16_t seed, std::vector<uint16_t> list);

Pokemon::AdvNature string_to_nature(std::string nature_string);

// get search hits for any of the provided seed values and advances range
std::vector<AdvRngState> get_search_results(
    ConsoleHandle& console,
    AdvRngSearcher& searcher, 
    AdvRngFilters& filters,
    const std::vector<uint16_t>& SEED_VALUES,
    const uint64_t& ADVANCES, 
    const uint64_t& advances_radius,
    int16_t gender_threshold = 126,
    uint16_t tid_xor_sid = 0
);

std::vector<AdvRngState> get_wild_search_results(
    ConsoleHandle& console,
    AdvRngWildSearcher& searcher, 
    AdvRngFilters& filters,
    const std::vector<uint16_t>& SEED_VALUES,
    const uint64_t& ADVANCES, 
    const uint64_t& advances_radius,
    int16_t gender_threshold = 126,
    bool super_rod = false,
    uint16_t tid_xor_sid = 0
);

// update IV ranges for RNG search filters with new stats/EVs after leveling up
void update_filters(
    AdvRngFilters& filters, 
    AdvObservedPokemon& pokemon, 
    const StatReads& stats, 
    const EVs& evyield, 
    const BaseStats& BASE_STATS
);

// get seed calibration based on average offset in the RNG calibration history
double get_seed_calibration_frames(
    const RngCalibrationHistory& HISTORY, 
    const std::vector<uint16_t>& SEED_VALUES, 
    const int16_t& SEED_POSITION
);

// get advances calibration based on average offset in the RNG calibration history
double get_advances_calibration_frames(const RngCalibrationHistory& CALIBRATION_HISTORY, const uint64_t& ADVANCES);

// infer hit seeds/advances, update the calibration history, and return whether or not the search is finished
bool update_history(
    ConsoleHandle& console,
    RngAdvanceHistory& ADVANCE_HISTORY,
    RngCalibrationHistory& CALIBRATION_HISTORY, 
    const uint16_t& MAX_HISTORY_LENGTH,
    const double& SEED_CALIBRATION_FRAMES,
    const double& ADVANCES_CALIBRATION,
    const double& CONTINUE_SCREEN_ADJUSTMENT,
    const std::vector<AdvRngState>& search_hits,
    uint32_t max_advance_possibilities = 1,
    uint32_t advance_radius = 2,
    bool force_finish = false
);

bool all_indistinguishable(std::vector<AdvRngState> hits, AdvRngSearcher& searcher);
bool all_indistinguishable(std::vector<AdvRngState> hits, AdvRngWildSearcher& searcher, bool super_rod);



}
}
}
#endif
