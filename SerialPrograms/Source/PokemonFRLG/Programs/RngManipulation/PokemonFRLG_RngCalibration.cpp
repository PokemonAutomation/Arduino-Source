/*  RNG Calibration
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <sstream>
#include "CommonFramework/Exceptions/OperationFailedException.h"
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
        OperationFailedException::fire(
            ErrorReport::NO_ERROR_REPORT,
            "GiftRng(): Invalid seed length. Seeds should be 4 characters.",
            console
        ); 
    }

    for (char ch : seed_string){
        auto iter = MAP.find(ch);
        if (iter == MAP.end()){
            OperationFailedException::fire(
                ErrorReport::NO_ERROR_REPORT,
                "GiftRng(): Invalid seed character. Seeds should be hex strings (valid characters are 0-9 and A-F).",
                console
            ); 
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

AdvNature string_to_nature(std::string nature_string){
    if (nature_string == "Hardy")   return AdvNature::Hardy;
    if (nature_string == "Lonely")  return AdvNature::Lonely;    
    if (nature_string == "Brave")   return AdvNature::Brave;  
    if (nature_string == "Adamant") return AdvNature::Adamant;  
    if (nature_string == "Naughty") return AdvNature::Naughty;  
    if (nature_string == "Bold")    return AdvNature::Bold;  
    if (nature_string == "Docile")  return AdvNature::Docile;  
    if (nature_string == "Relaxed") return AdvNature::Relaxed;  
    if (nature_string == "Impish")  return AdvNature::Impish;  
    if (nature_string == "Lax")     return AdvNature::Lax;  
    if (nature_string == "Timid")   return AdvNature::Timid;  
    if (nature_string == "Hasty")   return AdvNature::Hasty;  
    if (nature_string == "Serious") return AdvNature::Serious;  
    if (nature_string == "Jolly")   return AdvNature::Jolly;  
    if (nature_string == "Naive")   return AdvNature::Naive;  
    if (nature_string == "Modest")  return AdvNature::Modest;  
    if (nature_string == "Mild")    return AdvNature::Mild;  
    if (nature_string == "Quiet")   return AdvNature::Quiet;  
    if (nature_string == "Bashful") return AdvNature::Bashful;  
    if (nature_string == "Rash")    return AdvNature::Rash;  
    if (nature_string == "Calm")    return AdvNature::Calm;  
    if (nature_string == "Gentle")  return AdvNature::Gentle;  
    if (nature_string == "Sassy")   return AdvNature::Sassy;  
    if (nature_string == "Careful") return AdvNature::Careful;  
    if (nature_string == "Quirky")  return AdvNature::Quirky;  
    return AdvNature::Any;
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
    const BaseStats& BASE_STATS
){
    level_up_observed_pokemon(pokemon, stats, evyield);

    while (true){
        // in the worst case (the new stats are the problem), start over
        if (pokemon.level.size() == 0){
            filters.ivs = { {0,31}, {0,31}, {0,31}, {0,31}, {0,31}, {0,31} };
            return;
        }

        AdvRngFilters new_filters = observation_to_filters(pokemon, BASE_STATS);

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

double get_seed_calibration_frames(
    const RngCalibrationHistory& HISTORY, 
    const std::vector<uint16_t>& SEED_VALUES, 
    const int16_t& SEED_POSITION
){
    double sum = 0;
    uint16_t len = 0;
    for (size_t i=0; i<HISTORY.results.size(); i++){
        uint16_t seed = HISTORY.results[i].seed;
        int16_t position = -1;
        for (size_t j=0; j<SEED_VALUES.size(); j++){
            if (seed == SEED_VALUES[j]){
                position = int16_t(j);
                break;
            }
        }
        if (position < 0){
            continue;
        }
        double calibration = HISTORY.seed_calibrations[i];
        double offset = SEED_POSITION - position + calibration;
        sum += offset;
        len++;
    }

    if (len == 0){
        return 0;
    }

    double average_offset = sum / len;
    return average_offset;
}

double get_advances_calibration_frames(const RngCalibrationHistory& CALIBRATION_HISTORY, const uint64_t& ADVANCES){
    double sum = 0;
    uint16_t len = 0;
    for (size_t i=0; i<CALIBRATION_HISTORY.results.size(); i++){
        uint64_t advance = CALIBRATION_HISTORY.results[i].advance;
        double calibration = CALIBRATION_HISTORY.advance_calibrations[i];
        double offset = calibration + ADVANCES - advance;
        sum += offset;
        len++;
    }

    if (len == 0){
        return 0;
    }

    double average_offset = sum / len;
    return average_offset;
}


bool update_history(
    ConsoleHandle& console,
    RngAdvanceHistory& ADVANCE_HISTORY,
    RngCalibrationHistory& CALIBRATION_HISTORY, 
    const uint16_t& MAX_HISTORY_LENGTH,
    const double& SEED_CALIBRATION_FRAMES,
    const double& ADVANCES_CALIBRATION,
    const double& CONTINUE_SCREEN_ADJUSTMENT,
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

    if (search_hits.size() == 1){
        console.log("Updating calibrations...");
        CALIBRATION_HISTORY.seed_calibrations.emplace_back(SEED_CALIBRATION_FRAMES);
        CALIBRATION_HISTORY.advance_calibrations.emplace_back(ADVANCES_CALIBRATION);
        CALIBRATION_HISTORY.continue_screen_adjustments.emplace_back(CONTINUE_SCREEN_ADJUSTMENT);
        CALIBRATION_HISTORY.results.emplace_back(search_hits[0]);
        if (CALIBRATION_HISTORY.results.size() > MAX_HISTORY_LENGTH){
            CALIBRATION_HISTORY.seed_calibrations.erase(CALIBRATION_HISTORY.seed_calibrations.begin());
            CALIBRATION_HISTORY.advance_calibrations.erase(CALIBRATION_HISTORY.advance_calibrations.begin());
            CALIBRATION_HISTORY.continue_screen_adjustments.erase(CALIBRATION_HISTORY.continue_screen_adjustments.begin());
            CALIBRATION_HISTORY.results.erase(CALIBRATION_HISTORY.results.begin());
        }
        ADVANCE_HISTORY.results.clear();
        ADVANCE_HISTORY.seed_calibrations.clear();
        return true;
    }
    
    std::vector<uint64_t> advances;
    std::vector<AdvRngState> hits;
    for(auto hit : search_hits) {
        advances.emplace_back(hit.advance);
        hits.emplace_back(hit);
    }
    
    // get unique advances
    std::sort(advances.begin(), advances.end());
    std::vector<uint64_t>::iterator iter;
    iter = std::unique(advances.begin(), advances.begin() + advances.size());
    advances.resize(std::distance(advances.begin(), iter));

    ADVANCE_HISTORY.seed_calibrations.emplace_back(SEED_CALIBRATION_FRAMES);
    ADVANCE_HISTORY.results.emplace_back(hits);
    
    // check advance history for repeated values
    std::vector<uint64_t> counts;
    uint64_t best = 0;
    uint64_t mode = 0;
    bool tie = false;
    for (uint64_t& adv : advances){
        uint64_t count = 0;
        for (auto& res : ADVANCE_HISTORY.results){
            for (auto& state : res){
                if (std::abs(int64_t(state.advance) - int64_t(adv)) <= advance_radius){
                    count++;
                    break; // only count one possible hit from each attempt
                }
            }
        }
        if (count > best){
            mode = adv;
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


    // add the closest possibility to the advances mode for each attempt to the calibration history
    console.log("Inferred hits from previous " + std::to_string(ADVANCE_HISTORY.results.size()) + " attempts: ");
    for (size_t i=0; i<ADVANCE_HISTORY.results.size(); i++){
        auto& res = ADVANCE_HISTORY.results[i];
        auto& seed_calibration = ADVANCE_HISTORY.seed_calibrations[i];
        AdvRngState most_likely_hit;
        int64_t lowest_dist = INTMAX_MAX;
        for (auto& state : res){
            int64_t state_dist = std::abs(int64_t(state.advance) - int64_t(mode));
            if (state_dist < lowest_dist){
                most_likely_hit = state;
                lowest_dist = state_dist;
            }
        }
        CALIBRATION_HISTORY.seed_calibrations.emplace_back(seed_calibration);
        CALIBRATION_HISTORY.advance_calibrations.emplace_back(ADVANCES_CALIBRATION);
        CALIBRATION_HISTORY.continue_screen_adjustments.emplace_back(CONTINUE_SCREEN_ADJUSTMENT);
        CALIBRATION_HISTORY.results.emplace_back(most_likely_hit);
        console.log("   " + std::to_string(most_likely_hit.seed) + " / " + std::to_string(most_likely_hit.advance));
    }


    while (CALIBRATION_HISTORY.results.size() > MAX_HISTORY_LENGTH){
        CALIBRATION_HISTORY.seed_calibrations.erase(CALIBRATION_HISTORY.seed_calibrations.begin());
        CALIBRATION_HISTORY.advance_calibrations.erase(CALIBRATION_HISTORY.advance_calibrations.begin());
        CALIBRATION_HISTORY.continue_screen_adjustments.erase(CALIBRATION_HISTORY.continue_screen_adjustments.begin());
        CALIBRATION_HISTORY.results.erase(CALIBRATION_HISTORY.results.begin());
    }

    ADVANCE_HISTORY.results.clear();
    ADVANCE_HISTORY.seed_calibrations.clear();

    return true;
}



bool are_indistinguishable(AdvPokemonResult res1, AdvPokemonResult res2){
    return (
        res1.nature == res2.nature &&
        res1.gender == res2.gender &&
        // res1.ability == res2.ability &&
        res1.ivs.hp == res2.ivs.hp &&
        res1.ivs.attack == res2.ivs.attack &&
        res1.ivs.defense == res2.ivs.defense &&
        res1.ivs.spatk == res2.ivs.spatk &&
        res1.ivs.spdef == res2.ivs.spdef &&
        res1.ivs.speed == res2.ivs.speed
    );
}

bool are_indistinguishable(AdvWildPokemonResult res1, AdvWildPokemonResult res2){
    return (
        res1.species == res2.species &&
        res1.level == res2.level &&
        res1.nature == res2.nature &&
        res1.gender == res2.gender &&
        // res1.ability == res2.ability &&
        res1.ivs.hp == res2.ivs.hp &&
        res1.ivs.attack == res2.ivs.attack &&
        res1.ivs.defense == res2.ivs.defense &&
        res1.ivs.spatk == res2.ivs.spatk &&
        res1.ivs.spdef == res2.ivs.spdef &&
        res1.ivs.speed == res2.ivs.speed
    );
}

bool all_indistinguishable(std::vector<AdvRngState> hits, AdvRngSearcher& searcher){
    if (hits.size() < 2){
        return true;
    }
    searcher.state = hits[0];
    AdvPokemonResult first_result = searcher.generate_pokemon();
    for (size_t i=1; i<hits.size(); i++){
        searcher.state = hits[i];
        AdvPokemonResult other_result = searcher.generate_pokemon();
        if (!are_indistinguishable(first_result, other_result)){
            return false;
        }
    }
    return true;
}

bool all_indistinguishable(std::vector<AdvRngState> hits, AdvRngWildSearcher& searcher, bool super_rod){
    if (hits.size() < 2){
        return true;
    }
    searcher.state = hits[0];
    AdvWildPokemonResult first_result = searcher.generate_pokemon(super_rod);

    for (size_t i=1; i<hits.size(); i++){
        searcher.state = hits[i];
        AdvWildPokemonResult other_result = searcher.generate_pokemon(super_rod);
        if (!are_indistinguishable(first_result, other_result)){
            return false;
        }
    }
    return true;
}



}
}
}
