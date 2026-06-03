/*  RNG Calibration
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_RngCalibration_H
#define PokemonAutomation_PokemonFRLG_RngCalibration_H

#include <string>
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "Pokemon/Pokemon_AdvRng.h"
#include "PokemonFRLG/Programs/RngManipulation/PokemonFRLG_BlindNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

using namespace Pokemon;


static const double FRLG_FRAMERATE = 59.999977; // FPS
static const double FRLG_FRAME_DURATION = 1000.0 / FRLG_FRAMERATE;

struct RngCalibrations{
    double seed_offset;
    double csf_offset;
    double ingame_offset;

    bool operator==(const RngCalibrations& other) const {
        return (
               seed_offset   == other.seed_offset
            && csf_offset    == other.csf_offset
            && ingame_offset == other.ingame_offset
        );
    }
};

struct RngUncertainHistory{
    std::vector<RngCalibrations> calibrations;
    std::vector<std::vector<AdvRngState>> results;
};

struct RngCalibrationHistory{
    std::vector<RngCalibrations> calibrations;
    std::vector<AdvRngState> results;
};


void check_seed_validity(ConsoleHandle& console, std::string seed_string);
uint16_t parse_seed(ConsoleHandle& console, std::string seed_string);
std::vector<uint16_t> parse_seed_list(ConsoleHandle& console, std::string seed_list_string);
int16_t seed_position_in_list(uint16_t seed, std::vector<uint16_t> list);

std::string to_hex_string(const uint16_t& val);
std::string to_hex_string(const uint32_t& val);

RngTimings prepare_timings(
    ConsoleHandle& console,
    PokemonFRLG_RngTarget target,
    const uint64_t& SEED_DELAY,
    const uint64_t& CONTINUE_SCREEN_FRAMES,
    const uint64_t& INGAME_ADVANCES,
    const bool& USE_TEACHY_TV,
    const RngCalibrations& calibrations,
    const int64_t& FIXED_SEED_OFFSET, 
    const int64_t& FIXED_ADVANCES_OFFSET
);

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

std::vector<std::pair<AdvRngState,AdvRngState>> get_egg_search_results(
    ConsoleHandle& console,
    AdvRngEggSearcher& searcher, 
    AdvRngFilters& filters,
    const std::vector<uint16_t>& HELD_SEED_VALUES,
    const std::vector<uint16_t>& PICKUP_SEED_VALUES,
    const uint64_t& HELD_ADVANCES, 
    const uint64_t& held_advances_radius,
    const uint64_t& PICKUP_ADVANCES, 
    const uint64_t& pickup_advances_radius,
    AdvIVs& parentA,
    AdvIVs& parentB,
    AdvEggCompatibility compatibility,
    int16_t gender_threshold,
    uint16_t tid_xor_sid
);

// update IV ranges for RNG search filters with new stats/EVs after leveling up
void update_filters(
    AdvRngFilters& filters, 
    AdvObservedPokemon& pokemon, 
    const StatReads& stats, 
    const EVs& evyield, 
    const BaseStats& BASE_STATS,
    AdvRngMethod method = AdvRngMethod::Method1
);

// get the search radius across advances based on the length of the RNG calibration history
uint64_t get_advances_radius(
    ConsoleHandle& console, 
    const RngCalibrationHistory& calibration_history,
    const uint64_t& initial_radius
);

// get seed calibration based on average offset in the RNG calibration history
double get_seed_calibration_frames(
    const RngCalibrationHistory& history, 
    const std::vector<uint16_t>& seed_values, 
    const int16_t& seed_position
);

// get advances calibration based on average offset in the RNG calibration history
double get_advances_calibration_frames(const RngCalibrationHistory& calibration_history, const uint64_t& advances);

// get RngCalibrations from the RNG calibration history
RngCalibrations get_calibrations(
    ConsoleHandle& console,
    const RngCalibrationHistory& history,
    const std::vector<uint16_t>& seed_values,
    const int16_t& seed_position,
    const uint64_t& advances,
    bool csf_first = false
);

// infer hit seeds/advances, update the calibration history, and return whether or not the search is finished
bool update_history(
    ConsoleHandle& console,
    RngUncertainHistory& uncertain_history,
    RngCalibrationHistory& calibration_history, 
    const uint16_t& max_history_length,
    const RngCalibrations calibrations,
    const std::vector<AdvRngState>& search_hits,
    uint32_t max_advance_possibilities = 1,
    uint32_t advance_radius = 2,
    bool force_finish = false
);

bool all_indistinguishable(const std::vector<AdvRngState>& hits, AdvRngSearcher& searcher, const int16_t& gender_threshold);
bool all_indistinguishable(const std::vector<AdvRngState>& hits, AdvRngWildSearcher& searcher, const int16_t& gender_threshold, bool super_rod);
bool all_indistinguishable(
    const std::vector<std::pair<AdvRngState, AdvRngState>>& hits, 
    AdvRngEggSearcher& searcher,
    const int16_t& gender_threshold,
    AdvIVs& parentA_ivs, AdvIVs& parentB_ivs
);

bool same_seeds(std::vector<AdvRngState> hits);


}
}
}
#endif
