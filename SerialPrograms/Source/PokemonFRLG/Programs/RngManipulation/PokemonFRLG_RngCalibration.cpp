/*  RNG Calibration
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include <utility>
#include <sstream>
#include <algorithm>
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "PokemonFRLG_BlindNavigation.h"
#include "PokemonFRLG_RngCalibration.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

void check_seed_validity(ConsoleHandle& console, std::string seed_string){
    static const std::map<char, char> MAP{
        {'1', '1'},
        {'2', '2'},
        {'3', '3'},
        {'4', '4'},
        {'5', '5'},
        {'6', '6'},
        {'7', '7'},
        {'8', '8'},
        {'9', '9'},
        {'0', '0'},
        {'A', 'A'}, {'a', 'A'},
        {'B', 'B'}, {'b', 'B'},
        {'C', 'C'}, {'c', 'C'},
        {'D', 'D'}, {'d', 'D'},
        {'E', 'E'}, {'e', 'E'},
        {'F', 'F'}, {'f', 'F'}
    };

    if (seed_string.size() != 4){
        throw UserSetupError(console, "Invalid seed length. Seeds should be 4 characters.");
    }

    for (char ch : seed_string){
        auto iter = MAP.find(ch);
        if (iter == MAP.end()){
            throw UserSetupError(console, "Invalid seed character. Seeds should be valid hex strings (valid characters are 0-9 and A-F");
        }
    }
}

uint16_t parse_seed(ConsoleHandle& console, std::string seed_string){
    check_seed_validity(console, seed_string);
    std::istringstream converter(seed_string);
    uint16_t value;
    converter >> std::hex >> value;
    return value;
}

std::vector<uint16_t> parse_seed_list(ConsoleHandle& console, std::string seed_list_string){
    std::vector<uint16_t> values;
    auto ss = std::stringstream{seed_list_string};
    for (std::string line; std::getline(ss, line, '\n');){
        values.push_back(parse_seed(console, line));
    }
    return values;    
}

int16_t seed_position_in_list(uint16_t seed, std::vector<uint16_t> list){
    for (size_t i=0; i<list.size(); i++){
        if (seed == list[i]) {
            return int16_t(i);
        }
    }
    return -1;
}

std::string to_hex_string(const uint16_t& val){
    std::ostringstream s;
    s << std::hex << val;
    return s.str();
}
std::string to_hex_string(const uint32_t& val){
    std::ostringstream s;
    s << std::hex << val;
    return s.str();
}

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
){
    double modified_ingame_advances = INGAME_ADVANCES + calibrations.ingame_offset + FIXED_ADVANCES_OFFSET;
    if (modified_ingame_advances < 0) {
        throw UserSetupError(console, "In-game advances cannot be negative. Check your in-game advances and calibration.");
    }

    bool safari_zone = (
        target == PokemonFRLG_RngTarget::safarizonecenter ||
        target == PokemonFRLG_RngTarget::safarizoneeast ||
        target == PokemonFRLG_RngTarget::safarizonenorth ||
        target == PokemonFRLG_RngTarget::safarizonewest ||
        target == PokemonFRLG_RngTarget::safarizonesurf ||
        target == PokemonFRLG_RngTarget::safarizonefish
    );

    uint64_t TEACHY_ADVANCES = 0;

    uint64_t TEACHY_TV_BUFFER = safari_zone ? 20000 : 12000; // Safari zone targets need extra time to walk to the right position

    bool should_use_teachy_tv = USE_TEACHY_TV && (modified_ingame_advances > TEACHY_TV_BUFFER); // don't use Teachy TV for short in-game advance targets
    if (should_use_teachy_tv) {
        TEACHY_ADVANCES = uint64_t((int)std::floor((modified_ingame_advances - TEACHY_TV_BUFFER + 7500) / 313) * 313);
    }

    console.log("Seed calibration (frames): " + std::to_string(calibrations.seed_offset));
    console.log("CSF calibration (frames): " + std::to_string(calibrations.csf_offset));
    console.log("In-game calibration (frames x2): " + std::to_string(calibrations.ingame_offset));

    double seed_delay = SEED_DELAY + (calibrations.seed_offset * FRLG_FRAME_DURATION) + FIXED_SEED_OFFSET;
    double csf_delay = (CONTINUE_SCREEN_FRAMES + calibrations.csf_offset) * FRLG_FRAME_DURATION;
    double teachy_delay = TEACHY_ADVANCES * FRLG_FRAME_DURATION / 313;
    double ingame_delay = (modified_ingame_advances - TEACHY_ADVANCES) * FRLG_FRAME_DURATION / 2 - (should_use_teachy_tv ? 14067 : 0);

    if (seed_delay < 0){
        throw UserSetupError(console, "prepare_timings(): seed delay cannot be negative. Check your calibration values.");
    }
        if (csf_delay < 0){
        throw UserSetupError(console, "prepare_timings(): CSF duration cannot be negative. Check your calibration values.");
    }
        if (teachy_delay < 0){
        throw UserSetupError(console, "prepare_timings(): Teachy TV duration cannot be negative. Check your calibration values.");
    }
    if (ingame_delay < 0){
        throw UserSetupError(console, "prepare_timings(): in-game duration cannot be negative. Check your calibration values.");
    }

    RngTimings timings;
    timings.seed_delay   = uint64_t(seed_delay);
    timings.csf_delay    = uint64_t(csf_delay);
    timings.teachy_delay = uint64_t(teachy_delay);
    timings.ingame_delay = uint64_t(ingame_delay);

    console.log("Seed delay: " + std::to_string(timings.seed_delay) + "ms");
    console.log("Continue Screen delay: " + std::to_string(timings.csf_delay) + "ms");
    console.log("Teachy TV delay: " + std::to_string(timings.teachy_delay) + "ms");
    console.log("In-game delay: " + std::to_string(timings.ingame_delay) + "ms");
    console.log("Total time: " + std::to_string(timings.seed_delay + timings.csf_delay + timings.teachy_delay + timings.ingame_delay) + "ms");

    check_timings(console, target, timings, safari_zone);

    return timings;
}


std::vector<AdvRngState> get_search_results(
    ConsoleHandle& console,
    AdvRngSearcher& searcher, 
    AdvRngFilters& filters,
    const std::vector<uint16_t>& SEED_VALUES,
    const uint64_t& ADVANCES, 
    const uint64_t& advances_radius,
    int16_t gender_threshold,
    uint16_t tid_xor_sid

){
    std::vector<AdvRngState> search_hits;
    for (int i=0; i<4; i++){
        uint64_t adv_radius = advances_radius * (uint64_t(1) << i);
        uint64_t min_adv = ADVANCES - std::min(uint64_t(ADVANCES), adv_radius);    
        uint64_t max_adv = ADVANCES + adv_radius;
        search_hits = searcher.search(filters, SEED_VALUES, min_adv, max_adv, gender_threshold, tid_xor_sid);
        if (search_hits.size() > 0){
            console.log("Number of search hits: " + std::to_string(search_hits.size()));
            return search_hits;
        }
    }
    console.log("Number of search hits: " + std::to_string(search_hits.size()));
    return search_hits;
}

std::vector<AdvRngState> get_wild_search_results(
    ConsoleHandle& console,
    AdvRngWildSearcher& searcher, 
    AdvRngFilters& filters,
    const std::vector<uint16_t>& SEED_VALUES,
    const uint64_t& ADVANCES, 
    const uint64_t& advances_radius,
    int16_t gender_threshold,
    bool super_rod,
    uint16_t tid_xor_sid

){
    std::vector<AdvRngState> search_hits;
    for (int i=0; i<4; i++){
        uint64_t adv_radius = advances_radius * (uint64_t(1) << i);
        uint64_t min_adv = ADVANCES - std::min(uint64_t(ADVANCES), adv_radius);    
        uint64_t max_adv = ADVANCES + adv_radius;
        search_hits = searcher.search(filters, SEED_VALUES, min_adv, max_adv, gender_threshold, super_rod, tid_xor_sid);
        if (search_hits.size() > 0){
            console.log("Number of search hits: " + std::to_string(search_hits.size()));
            return search_hits;
        }
    }
    console.log("Number of search hits: " + std::to_string(search_hits.size()));
    return search_hits;
}

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
){
    std::vector<std::pair<AdvRngState,AdvRngState>> search_hits;
    for (int i=0; i<1; i++){
        uint64_t held_adv_radius = held_advances_radius * (uint64_t(1) << i);
        uint64_t held_min_adv = HELD_ADVANCES - std::min(uint64_t(HELD_ADVANCES), held_adv_radius);    
        uint64_t held_max_adv = HELD_ADVANCES + held_adv_radius;
        uint64_t pickup_adv_radius = pickup_advances_radius * (uint64_t(1) << i);
        uint64_t pickup_min_adv = PICKUP_ADVANCES - std::min(uint64_t(PICKUP_ADVANCES), pickup_adv_radius);    
        uint64_t pickup_max_adv = PICKUP_ADVANCES + pickup_adv_radius;
        search_hits = searcher.search(
            filters, HELD_SEED_VALUES, 
            held_min_adv, held_max_adv, 
            PICKUP_SEED_VALUES, pickup_min_adv, pickup_max_adv, 
            parentA, parentB, compatibility,
            gender_threshold, tid_xor_sid
        );
        if (search_hits.size() > 0){
            console.log("Number of search hits: " + std::to_string(search_hits.size()));
            return search_hits;
        }
    }
    console.log("Number of search hits: " + std::to_string(search_hits.size()));
    return search_hits;
}

bool range_is_valid(IvRange iv){
    return (
        iv.low <= iv.high &&
        iv.low >= 0 &&
        iv.high >= 0
    );
}

bool ranges_are_valid(IvRanges ivs){
    return (
        range_is_valid(ivs.hp) &&
        range_is_valid(ivs.attack) &&
        range_is_valid(ivs.defense) &&
        range_is_valid(ivs.spatk) &&
        range_is_valid(ivs.spdef) &&
        range_is_valid(ivs.speed)
    );
}

void update_filters(
    AdvRngFilters& filters, 
    AdvObservedPokemon& pokemon, 
    const StatReads& stats, 
    const EVs& evyield, 
    const BaseStats& BASE_STATS,
    AdvRngMethod method
){
    level_up_observed_pokemon(pokemon, stats, evyield);

    while (true){
        // in the worst case (the new stats are the problem), start over
        if (pokemon.level.size() == 0){
            filters.ivs = { {0,31}, {0,31}, {0,31}, {0,31}, {0,31}, {0,31} };
            return;
        }

        AdvRngFilters new_filters = observation_to_filters(pokemon, BASE_STATS, method);

        if (!ranges_are_valid(new_filters.ivs)){
            IvRanges new_stat_ivs = calc_iv_ranges(BASE_STATS, pokemon.level.back(), pokemon.evs.back(), pokemon.stats.back(), nature_to_adjustment(pokemon.nature));
            if (!ranges_are_valid(new_stat_ivs)){
                // remove newest stats first if they aren't valid
                pokemon.level.erase(pokemon.level.begin());
                pokemon.stats.erase(pokemon.stats.begin());
                pokemon.evs.erase(pokemon.evs.begin());
            }else{
                // remove oldest stats first
                pokemon.level.pop_back();
                pokemon.stats.pop_back();
                pokemon.evs.pop_back();
            }
            continue;
        }

        filters.ivs = new_filters.ivs;
        return;
    }

}


uint64_t get_advances_radius(
    ConsoleHandle& console, 
    const RngCalibrationHistory& calibration_history,
    const uint64_t& initial_radius
){
    uint64_t advances_radius = initial_radius;
    for (size_t i=0; i<calibration_history.results.size(); i++){
        advances_radius = advances_radius / 2;
        if (advances_radius <= 4){
            advances_radius = 4;
            break;
        }
    }
    console.log("Advances search radius: " + std::to_string(advances_radius));

    return advances_radius;
}

double get_seed_calibration_frames(
    const RngCalibrationHistory& history, 
    const std::vector<uint16_t>& seed_values, 
    const int16_t& seed_position
){
    double sum = 0;
    uint16_t len = 0;
    for (size_t i=0; i<history.results.size(); i++){
        uint16_t seed = history.results[i].seed;
        int16_t position = -1;
        for (size_t j=0; j<seed_values.size(); j++){
            if (seed == seed_values[j]){
                position = int16_t(j);
                break;
            }
        }
        if (position < 0){
            continue;
        }
        double calibration = history.calibrations[i].seed_offset;
        double offset = seed_position - position + calibration;
        sum += offset;
        len++;
    }

    if (len == 0){
        return 0;
    }

    double average_offset = sum / len;
    return average_offset;
}

double get_advances_calibration(const RngCalibrationHistory& history, const uint64_t& advance_target){
    double sum = 0;
    uint16_t len = 0;
    for (size_t i=0; i<history.results.size(); i++){
        uint64_t advance = history.results[i].advance;
        double calibration = history.calibrations[i].ingame_offset + history.calibrations[i].csf_offset;
        double offset = calibration + advance_target - advance;
        sum += offset;
        len++;
    }

    if (len == 0){
        return 0;
    }

    double average_offset = sum / len;
    return average_offset;
}

RngCalibrations get_calibrations(
    ConsoleHandle& console,
    const RngCalibrationHistory& history,
    const std::vector<uint16_t>& seed_values,
    const int16_t& seed_position,
    const uint64_t& advances,
    bool csf_first
){
    RngCalibrations calibrations{};

    if (history.results.size() > 0){
        
        calibrations.seed_offset = get_seed_calibration_frames(history, seed_values, seed_position);

        AdvRngState prev_hit = history.results.back();
        double prev_csf_offset = history.calibrations.back().csf_offset;
        double prev_ingame_offset = history.calibrations.back().ingame_offset;
        int64_t prev_advance_miss = int64_t(prev_hit.advance) - int64_t(advances);

        if (prev_advance_miss == 0){
            // don't change anything if the advance target has been hit
            calibrations.csf_offset = prev_csf_offset;
            calibrations.ingame_offset = prev_ingame_offset;
        }else if (std::abs(prev_advance_miss) < 2){
            // when very close, take the previous calibration and bump the CSF in the right direction
            if (prev_advance_miss > 0){
                calibrations.csf_offset = prev_csf_offset - 0.5;
            }else{
                calibrations.csf_offset = prev_csf_offset + 0.5;
            }
            calibrations.csf_offset = fmod(calibrations.csf_offset, 2);
            calibrations.ingame_offset = prev_ingame_offset; // leave unchanged
        }else if(csf_first){
            // adjust the csf, putting anything beyond +/-2 frames into the in-game calibration
            double new_advances_calibration = get_advances_calibration(history, advances);
            double total_diff = new_advances_calibration - prev_ingame_offset - prev_csf_offset;
            calibrations.csf_offset = fmod(prev_csf_offset + total_diff, 2);
            double csf_diff = calibrations.csf_offset - prev_csf_offset;
            calibrations.ingame_offset = prev_ingame_offset + total_diff - csf_diff;
        }else{
            // only adjust the in-game offset
            calibrations.ingame_offset = get_advances_calibration(history, advances);
        }
    }

    return calibrations;
}


bool update_history(
    ConsoleHandle& console,
    RngUncertainHistory& uncertain_history,
    RngCalibrationHistory& calibration_history, 
    const uint16_t& max_history_length,
    const RngCalibrations calibrations,
    const std::vector<AdvRngState>& search_hits,
    uint32_t max_advance_possibilities,
    uint32_t advance_radius,
    bool force_finish
){
    if (search_hits.size() == 0){
        console.log("No matches found.");
        return true;
    }

    if (!force_finish && search_hits.size() > max_advance_possibilities){
        return false;
    }
    
    std::vector<int64_t> uncal_advances; // hit advance - calibrations
    std::vector<AdvRngState> hits;
    for(auto hit : search_hits) {
        uncal_advances.emplace_back(int64_t(std::round(hit.advance - calibrations.csf_offset - calibrations.ingame_offset)));
        hits.emplace_back(hit);
    }
    
    // get unique advances from new search results
    std::sort(uncal_advances.begin(), uncal_advances.end());
    std::vector<int64_t>::iterator iter;
    iter = std::unique(uncal_advances.begin(), uncal_advances.begin() + uncal_advances.size());
    uncal_advances.resize(std::distance(uncal_advances.begin(), iter));

    int64_t mode = 0;

    if (uncal_advances.size() == 1){
        console.log("Hit " + to_hex_string(search_hits[0].seed) + " / " + std::to_string(search_hits[0].advance));
        mode = uncal_advances[0];
    }else {
        uncertain_history.calibrations.emplace_back(calibrations);
        uncertain_history.results.emplace_back(hits);
        
        // check advance history for repeated values
        std::vector<uint64_t> counts;
        uint64_t best = 0;
        bool tie = false;
        for (int64_t& uadv : uncal_advances){
            uint64_t count = 0;
            for (size_t i=0; i<uncertain_history.results.size(); i++){
                auto& cals = uncertain_history.calibrations[i];
                auto& res = uncertain_history.results[i];
                for (auto& state : res){
                    if (std::abs(int64_t(std::round(state.advance - cals.csf_offset - cals.ingame_offset)) - uadv) <= advance_radius){
                        count++;
                        break; // only count one possible hit from each attempt
                    }
                }
            }
            if (count > best){
                mode = uadv;
                best = count;
                tie = false;
            }else if (count == best){
                tie = true;
            }
        }

        if (tie){
            console.log("More than 1 possible advances value hit.");
            return true;
        }
    }
    
    // add the closest possibility to the advances mode for each attempt to the calibration history
    console.log("Inferred hits from previous " + std::to_string(uncertain_history.results.size()) + " attempts: ");
    for (size_t i=0; i<uncertain_history.results.size(); i++){
        auto& cals = uncertain_history.calibrations[i];
        auto& res = uncertain_history.results[i];
        AdvRngState most_likely_hit;
        int64_t lowest_dist = INTMAX_MAX;
        for (auto& state : res){
            int64_t state_dist = std::abs(int64_t(state.advance - cals.csf_offset - cals.ingame_offset) - int64_t(mode));
            if (state_dist < lowest_dist){
                most_likely_hit = state;
                lowest_dist = state_dist;
            }
        }
        calibration_history.calibrations.emplace_back(cals);
        calibration_history.results.emplace_back(most_likely_hit);
        console.log("   " + to_hex_string(most_likely_hit.seed) + " / " + std::to_string(most_likely_hit.advance));
    }


    while (calibration_history.results.size() > max_history_length){
        calibration_history.calibrations.erase(calibration_history.calibrations.begin());
        calibration_history.results.erase(calibration_history.results.begin());
    }

    uncertain_history.results.clear();
    uncertain_history.calibrations.clear();

    return true;
}



bool are_indistinguishable(AdvPokemonResult res1, AdvPokemonResult res2, const int16_t& gender_threshold){
    return (
        res1.nature == res2.nature &&
        gender_from_gender_value(res1.gender, gender_threshold) == gender_from_gender_value(res2.gender, gender_threshold) &&
        // res1.ability == res2.ability &&
        res1.ivs.hp == res2.ivs.hp &&
        res1.ivs.attack == res2.ivs.attack &&
        res1.ivs.defense == res2.ivs.defense &&
        res1.ivs.spatk == res2.ivs.spatk &&
        res1.ivs.spdef == res2.ivs.spdef &&
        res1.ivs.speed == res2.ivs.speed
    );
}

bool are_indistinguishable(AdvWildPokemonResult res1, AdvWildPokemonResult res2, const int16_t& gender_threshold){
    return (
        res1.species == res2.species &&
        res1.level == res2.level &&
        res1.nature == res2.nature &&
        gender_from_gender_value(res1.gender, gender_threshold) == gender_from_gender_value(res2.gender, gender_threshold) &&        
        // res1.ability == res2.ability &&
        res1.ivs.hp == res2.ivs.hp &&
        res1.ivs.attack == res2.ivs.attack &&
        res1.ivs.defense == res2.ivs.defense &&
        res1.ivs.spatk == res2.ivs.spatk &&
        res1.ivs.spdef == res2.ivs.spdef &&
        res1.ivs.speed == res2.ivs.speed
    );
}

bool all_indistinguishable(const std::vector<AdvRngState>& hits, AdvRngSearcher& searcher, const int16_t& gender_threshold){
    if (hits.size() < 2){
        return true;
    }
    searcher.state = hits[0];
    AdvPokemonResult first_result = searcher.generate_pokemon();
    for (size_t i=1; i<hits.size(); i++){
        searcher.state = hits[i];
        AdvPokemonResult other_result = searcher.generate_pokemon();
        if (!are_indistinguishable(first_result, other_result, gender_threshold)){
            return false;
        }
    }
    return true;
}

bool all_indistinguishable(const std::vector<AdvRngState>& hits, AdvRngWildSearcher& searcher, const int16_t& gender_threshold, bool super_rod){
    if (hits.size() < 2){
        return true;
    }
    searcher.state = hits[0];
    AdvWildPokemonResult first_result = searcher.generate_pokemon(super_rod);

    for (size_t i=1; i<hits.size(); i++){
        searcher.state = hits[i];
        AdvWildPokemonResult other_result = searcher.generate_pokemon(super_rod);
        if (!are_indistinguishable(first_result, other_result, gender_threshold)){
            return false;
        }
    }
    return true;
}

bool all_indistinguishable(
    const std::vector<std::pair<AdvRngState, AdvRngState>>& hits, 
    AdvRngEggSearcher& searcher,
    const int16_t& gender_threshold,
    AdvIVs& parentA_ivs, AdvIVs& parentB_ivs
){
    if (hits.size() < 2){
        return true;
    }
    searcher.held_state = hits[0].first;
    searcher.pickup_state = hits[0].second;

    AdvPokemonResult first_result = searcher.generate_pokemon(parentA_ivs, parentB_ivs);

     for (size_t i=1; i<hits.size(); i++){
        searcher.held_state = hits[i].first;
        searcher.pickup_state = hits[i].second;
        AdvPokemonResult other_result = searcher.generate_pokemon(parentA_ivs, parentB_ivs);
        if (!are_indistinguishable(first_result, other_result, gender_threshold)){
            return false;
        }
    }
    return true;
}

bool same_seeds(std::vector<AdvRngState> hits){
    if (hits.size() < 2){
        return true;
    }
    uint16_t first_seed = hits[0].seed;

    for (size_t i=1; i<hits.size(); i++){
        if (hits[i].seed != first_seed){
            return false;
        }
    }
    return true;
}



}
}
}
