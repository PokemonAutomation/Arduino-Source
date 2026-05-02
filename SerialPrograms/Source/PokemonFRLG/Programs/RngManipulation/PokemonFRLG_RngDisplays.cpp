/*  RNG Displays
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include <vector>
#include <iostream>
#include <sstream>
#include "PokemonFRLG_RngDisplays.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

using namespace Pokemon;

SidHelperDisplay::SidHelperDisplay()
    : GroupOption("TID/SID Results", LockMode::READ_ONLY)
    , tid(false, "<b>Trainer ID:</b>", LockMode::READ_ONLY, "-", "")
    , sids("<b>SID Possibilities:</b>", LockMode::READ_ONLY, "-", "")
{
    PA_ADD_STATIC(tid);
    PA_ADD_STATIC(sids);
}
std::string SidHelperDisplay::get_sids_string(const std::vector<std::pair<std::string, std::string>>& sid_messages){
    std::string sid_str = "";
    for (auto& str_pair : sid_messages){
        sid_str += str_pair.first + ": ";
        sid_str += str_pair.second + "\n";
    }
    return sid_str;
}
void SidHelperDisplay::set(uint16_t trainerId, const std::vector<std::pair<std::string, std::string>>& sid_messages){
    tid.set(std::to_string(trainerId));
    sids.set(get_sids_string(sid_messages));
}


RngFilterDisplay::RngFilterDisplay()
    : GroupOption("Observed Stats", LockMode::READ_ONLY)
    , hp(false, "<b>HP IV:</b>", LockMode::READ_ONLY, "-", "")
    , atk(false, "<b>Attack IV:</b>", LockMode::READ_ONLY, "-", "")
    , def(false, "<b>Defense IV:</b>", LockMode::READ_ONLY, "-", "")
    , spatk(false, "<b>Special Attack IV:</b>", LockMode::READ_ONLY, "-", "")
    , spdef(false, "<b>Special Defense IV:</b>", LockMode::READ_ONLY, "-", "")
    , speed(false, "<b>Speed IV:</b>", LockMode::READ_ONLY, "-", "")
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

void RngFilterDisplay::reset(){
    hp.set("-");
    atk.set("-");
    def.set("-");
    spatk.set("-");
    spdef.set("-");
    speed.set("-");
    gender.set("-");
    nature.set("-");
}

RngCalibrationDisplay::RngCalibrationDisplay()
    : GroupOption("RNG Calibration", LockMode::READ_ONLY)
    , seed_calibration("<b>Seed Calibration (ms):</b>", LockMode::LOCK_WHILE_RUNNING, 0)
    , csf_calibration("<b>Continue Screen Frames Calibration:</b>", LockMode::LOCK_WHILE_RUNNING, 0.0)
    , advances_calibration("<b>In-Game Advances Calibration:</b>", LockMode::LOCK_WHILE_RUNNING, 0.0)
    , hits(false, "<b>Seeds/Advances:</b>", LockMode::READ_ONLY, "-", "")
{
    PA_ADD_STATIC(seed_calibration);
    PA_ADD_STATIC(csf_calibration);
    PA_ADD_STATIC(advances_calibration);
    PA_ADD_STATIC(hits);
}

std::vector<AdvRngState> RngCalibrationDisplay::get_rng_states_from_map(const std::map<AdvRngState,AdvPokemonResult>& hits_map){
    std::vector<AdvRngState> rng_states;
    for(std::map<AdvRngState,AdvPokemonResult>::const_iterator it = hits_map.begin(); it != hits_map.end(); ++it) {
        rng_states.emplace_back(it->first);
    }
    return rng_states;
}

std::string RngCalibrationDisplay::get_hits_string(const std::vector<AdvRngState>& rng_states){
    std::string hits_string;
    for (size_t i=0; i<rng_states.size(); i++){
        if (i > 0){
            hits_string += ", ";
        }
        AdvRngState hit = rng_states[i];
        uint16_t seed = hit.seed;
        std::ostringstream s;
        s << std::hex << seed;
        hits_string += s.str();
        hits_string += "/";
        hits_string += std::to_string(hit.advance);
    }
    if (hits_string.size() == 0){
        hits_string += "No matches found";
    }
    return hits_string;
}
std::string RngCalibrationDisplay::get_hits_string(const std::map<AdvRngState, AdvPokemonResult>& hits_map){
    return get_hits_string(get_rng_states_from_map(hits_map));
}

void RngCalibrationDisplay::set(
    double s_calibration, 
    double c_calibration, 
    double a_calibration, 
    std::vector<AdvRngState>& rng_states
){
    seed_calibration.set(int64_t(std::round(s_calibration)));
    csf_calibration.set(c_calibration);
    advances_calibration.set(a_calibration);
    hits.set(get_hits_string(rng_states));
}
void RngCalibrationDisplay::set(    
    double s_calibration, 
    double c_calibration, 
    double a_calibration, 
    const std::map<AdvRngState, AdvPokemonResult>& hits_map
){
    std::vector<AdvRngState> rng_states = get_rng_states_from_map(hits_map);
    set(s_calibration, c_calibration, a_calibration, rng_states);
}

void RngCalibrationDisplay::reset(){
    // seed_calibration.set(0.0);
    // csf_calibration.set(0.0);
    // advances_calibration.set(0.0);
    hits.set("-");
}

}
}
}