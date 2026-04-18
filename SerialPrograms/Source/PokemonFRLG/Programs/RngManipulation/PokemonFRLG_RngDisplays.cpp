/*  RNG Displays
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <vector>
#include <iostream>
#include <sstream>
#include "PokemonFRLG_RngDisplays.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

using namespace Pokemon;

RngFilterDisplay::RngFilterDisplay()
    : GroupOption("Observed Stats", LockMode::READ_ONLY)
    , hp(false, "<b>HP:</b>", LockMode::READ_ONLY, "-", "")
    , atk(false, "<b>Attack:</b>", LockMode::READ_ONLY, "-", "")
    , def(false, "<b>Defense:</b>", LockMode::READ_ONLY, "-", "")
    , spatk(false, "<b>Special Attack:</b>", LockMode::READ_ONLY, "-", "")
    , spdef(false, "<b>Special Defense:</b>", LockMode::READ_ONLY, "-", "")
    , speed(false, "<b>Speed:</b>", LockMode::READ_ONLY, "-", "")
    , gender(false, "<b>Gender:</b>", LockMode::READ_ONLY, "-", "")
    , nature(false, "<b>Nature:</b>", LockMode::READ_ONLY, "-", "")
{
    PA_ADD_STATIC(hp);
    PA_ADD_STATIC(atk);
    PA_ADD_STATIC(def);
    PA_ADD_STATIC(spatk);
    PA_ADD_STATIC(spdef);
    PA_ADD_STATIC(speed);
    PA_ADD_STATIC(gender);
    PA_ADD_STATIC(nature);

}
std::string RngFilterDisplay::get_range_string(const IvRange& range){
    if (range.low < 0 || range.high < 0){
        return "(invalid or unable to read)";
    }
    if (range.low == range.high){
        return std::to_string(range.low);
    }
    return std::to_string(range.low) + " - " + std::to_string(range.high);
}
std::string RngFilterDisplay::get_gender_string(const AdvGender& gender){
    switch (gender){
    case AdvGender::Male:
        return "Male";
    case AdvGender::Female:
        return "Female";
    default:
        return "Any";
    }
}
std::string RngFilterDisplay::get_nature_string(const AdvNature& nature){
    switch (nature){
    case AdvNature::Hardy:
        return "Hardy";
    case AdvNature::Lonely:
        return "Lonely";
    case AdvNature::Brave:
        return "Brave";
    case AdvNature::Adamant:
        return "Adamant";
    case AdvNature::Naughty:
        return "Naughty";
    case AdvNature::Bold:
        return "Bold";
    case AdvNature::Docile:
        return "Docile";
    case AdvNature::Relaxed:
        return "Relaxed";
    case AdvNature::Impish:
        return "Impish";
    case AdvNature::Lax:
        return "Lax";
    case AdvNature::Timid:
        return "Timid";
    case AdvNature::Hasty:
        return "Hasty";
    case AdvNature::Serious:
        return "Serious";
    case AdvNature::Jolly:
        return "Jolly";
    case AdvNature::Naive:
        return "Naive";
    case AdvNature::Modest:
        return "Modest";
    case AdvNature::Mild:
        return "Mild";
    case AdvNature::Quiet:
        return "Quiet";
    case AdvNature::Bashful:
        return "Bashful";
    case AdvNature::Rash:
        return "Rash";
    case AdvNature::Calm:
        return "Calm";
    case AdvNature::Gentle:
        return "Gentle";
    case AdvNature::Sassy:
        return "Sassy";
    case AdvNature::Careful:
        return "Careful";
    case AdvNature::Quirky:
        return "Quirky";
    default:
        return "Any";
    }
}
void RngFilterDisplay::set(const AdvRngFilters& filter){
    hp.set(get_range_string(filter.ivs.hp));
    atk.set(get_range_string(filter.ivs.attack));
    def.set(get_range_string(filter.ivs.defense));
    spatk.set(get_range_string(filter.ivs.spatk));
    spdef.set(get_range_string(filter.ivs.spdef));
    speed.set(get_range_string(filter.ivs.speed));
    gender.set(get_gender_string(filter.gender));
    nature.set(get_nature_string(filter.nature));
}

PossibleHitsDisplay::PossibleHitsDisplay()
    : GroupOption("Possible Hits", LockMode::READ_ONLY)
    , seeds(false, "<b>Seeds:</b>", LockMode::READ_ONLY, "-", "")
    , advances(false, "<b>Advances:</b>", LockMode::READ_ONLY, "-", "")
{
    PA_ADD_STATIC(seeds);
    PA_ADD_STATIC(advances);
}

std::vector<AdvRngState> PossibleHitsDisplay::get_rng_states_from_map(std::map<AdvRngState,AdvPokemonResult>& hits_map){
    std::vector<AdvRngState> rng_states;
    for(std::map<AdvRngState,AdvPokemonResult>::iterator it = hits_map.begin(); it != hits_map.end(); ++it) {
        rng_states.emplace_back(it->first);
    }
    return rng_states;
}

std::string PossibleHitsDisplay::get_seeds_string(const std::vector<AdvRngState>& rng_states){
    std::string seeds_string;
    for (size_t i=0; i<rng_states.size(); i++){
        if (i > 0){
            seeds_string += ", ";
        }
        uint16_t seed = rng_states[i].seed;
        std::ostringstream s;
        s << std::hex << seed;
        seeds_string += s.str();
    }
    return seeds_string;
}
std::string PossibleHitsDisplay::get_seeds_string(std::map<AdvRngState, AdvPokemonResult>& hits_map){
    return get_seeds_string(get_rng_states_from_map(hits_map));
}

std::string PossibleHitsDisplay::get_advances_string(const std::vector<AdvRngState>& rng_states){
    std::string advances_string;
    for (size_t i=0; i<rng_states.size(); i++){
        if (i > 0){
            advances_string += ", ";
        }
        uint64_t adv = rng_states[i].advance;
        advances_string += std::to_string(adv);
    }
    return advances_string;
}
std::string PossibleHitsDisplay::get_advances_string(std::map<AdvRngState, AdvPokemonResult>& hits_map){
    return get_advances_string(get_rng_states_from_map(hits_map));
}

void PossibleHitsDisplay::set(const std::vector<AdvRngState>& rng_states){
    seeds.set(get_seeds_string(rng_states));
    advances.set(get_advances_string(rng_states));
}
void PossibleHitsDisplay::set(std::map<AdvRngState, AdvPokemonResult>& hits_map){
    std::vector<AdvRngState> rng_states = get_rng_states_from_map(hits_map);
    seeds.set(get_seeds_string(rng_states));
    advances.set(get_advances_string(rng_states));
}

}
}
}