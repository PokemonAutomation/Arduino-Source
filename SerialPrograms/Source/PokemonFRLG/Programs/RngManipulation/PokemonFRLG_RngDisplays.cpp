/*  RNG Displays
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include <vector>
#include <iostream>
#include <sstream>
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
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

std::string RngTargetDisplay::result_to_string(const AdvPokemonResult& pokemon, const int16_t& gender_threshold){
    std::string out;
    out += gender_to_string(gender_from_gender_value(pokemon.gender, gender_threshold)) + " ";
    out += nature_to_string(pokemon.nature) + " ";
    out +=  std::to_string(pokemon.ivs.hp) + "/" +
            std::to_string(pokemon.ivs.attack) + "/" +
            std::to_string(pokemon.ivs.defense) + "/" +
            std::to_string(pokemon.ivs.spatk) + "/" +
            std::to_string(pokemon.ivs.spdef) + "/" +
            std::to_string(pokemon.ivs.speed) + " ";
    return out;
}   

std::string RngTargetDisplay::result_to_string(const AdvWildPokemonResult& pokemon, const int16_t& gender_threshold){
    std::string out;
    out += get_pokemon_name(pokemon.species).display_name() + " ";
    out += "Lv" + std::to_string(pokemon.level) + " ";
    out += gender_to_string(gender_from_gender_value(pokemon.gender, gender_threshold)) + " ";
    out += nature_to_string(pokemon.nature) + " ";
    out +=  std::to_string(pokemon.ivs.hp) + "/" +
            std::to_string(pokemon.ivs.attack) + "/" +
            std::to_string(pokemon.ivs.defense) + "/" +
            std::to_string(pokemon.ivs.spatk) + "/" +
            std::to_string(pokemon.ivs.spdef) + "/" +
            std::to_string(pokemon.ivs.speed) + " ";
    return out;
}   

void RngTargetDisplay::set_target(const AdvPokemonResult& pokemon, const int16_t& gender_threshold){
    set(result_to_string(pokemon, gender_threshold));
}
void RngTargetDisplay::set_target(const AdvWildPokemonResult& pokemon, const int16_t& gender_threshold){
    set(result_to_string(pokemon, gender_threshold));
}


RngFilterDisplay::RngFilterDisplay()
    : GroupOption("Observed Stats", LockMode::READ_ONLY)
    , species(false, "<b>Species:</b>", LockMode::READ_ONLY, "-", "")
    , gender(false, "<b>Gender:</b>", LockMode::READ_ONLY, "-", "")
    , nature(false, "<b>Nature:</b>", LockMode::READ_ONLY, "-", "")
    , level(false, "<b>Level:</b>", LockMode::READ_ONLY, "-", "")
    , hp(false, "<b>HP IV:</b>", LockMode::READ_ONLY, "-", "")
    , atk(false, "<b>Attack IV:</b>", LockMode::READ_ONLY, "-", "")
    , def(false, "<b>Defense IV:</b>", LockMode::READ_ONLY, "-", "")
    , spatk(false, "<b>Special Attack IV:</b>", LockMode::READ_ONLY, "-", "")
    , spdef(false, "<b>Special Defense IV:</b>", LockMode::READ_ONLY, "-", "")
    , speed(false, "<b>Speed IV:</b>", LockMode::READ_ONLY, "-", "")
{   
    PA_ADD_STATIC(species);
    PA_ADD_STATIC(gender);
    PA_ADD_STATIC(nature);
    PA_ADD_STATIC(level);
    PA_ADD_STATIC(hp);
    PA_ADD_STATIC(atk);
    PA_ADD_STATIC(def);
    PA_ADD_STATIC(spatk);
    PA_ADD_STATIC(spdef);
    PA_ADD_STATIC(speed);
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

void RngFilterDisplay::set(const AdvRngFilters& filter){
    species.set(get_pokemon_name(filter.species).display_name());
    gender.set(gender_to_string(filter.gender));
    nature.set(nature_to_string(filter.nature));
    level.set(std::to_string(filter.level));
    hp.set(get_range_string(filter.ivs.hp));
    atk.set(get_range_string(filter.ivs.attack));
    def.set(get_range_string(filter.ivs.defense));
    spatk.set(get_range_string(filter.ivs.spatk));
    spdef.set(get_range_string(filter.ivs.spdef));
    speed.set(get_range_string(filter.ivs.speed));
}

void RngFilterDisplay::reset(){
    species.set("-");
    gender.set("-");
    nature.set("-");
    level.set("-");
    hp.set("-");
    atk.set("-");
    def.set("-");
    spatk.set("-");
    spdef.set("-");
    speed.set("-");

}

RngCalibrationDisplay::RngCalibrationDisplay()
    : GroupOption("RNG Calibration", LockMode::READ_ONLY)
    , seed_calibration("<b>Seed Calibration (ms):</b>", LockMode::LOCK_WHILE_RUNNING, 0)
    , csf_calibration("<b>Continue Screen Frames Calibration:</b>", LockMode::LOCK_WHILE_RUNNING, 0.0)
    , advances_calibration("<b>In-Game Advances Calibration:</b>", LockMode::LOCK_WHILE_RUNNING, 0.0)
    , hits(false, "<b>Seeds/Advances:</b>", LockMode::READ_ONLY, "-", "")
{
    PA_ADD_OPTION(seed_calibration);
    PA_ADD_OPTION(csf_calibration);
    PA_ADD_OPTION(advances_calibration);
    PA_ADD_STATIC(hits);
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

void RngCalibrationDisplay::set_calibrations(const RngCalibrations& calibrations){
    seed_calibration.set(int64_t(std::round(calibrations.seed_offset * FRLG_FRAME_DURATION)));
    csf_calibration.set(calibrations.csf_offset);
    advances_calibration.set(calibrations.ingame_offset);
}

void RngCalibrationDisplay::set_hits(const std::vector<AdvRngState>& rng_states){
    hits.set(get_hits_string(rng_states));
}

void RngCalibrationDisplay::reset_hits(){
    hits.set("-");
}

}
}
}