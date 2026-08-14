/*  BDSP RNG Displays
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include "Common/Cpp/PrettyPrint.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonBDSP_RngDisplays.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Pokemon;


const char* const NOT_SET = "-";


static std::string duration_to_string(double seconds){
    if (!(seconds >= 0.0)){
        return NOT_SET;
    }
    uint64_t total = (uint64_t)(seconds + 0.5);
    if (total < 60){
        return std::to_string(total) + "s";
    }
    return std::to_string(total / 60) + "m " + std::to_string(total % 60) + "s";
}

static std::string milliseconds_to_string(double seconds){
    return std::to_string((int64_t)(seconds * 1000.0 + (seconds < 0 ? -0.5 : 0.5))) + " ms";
}


RngStateDisplay::RngStateDisplay()
    : GroupOption("RNG State", LockMode::READ_ONLY)
    , state(false, "<b>State:</b>", LockMode::READ_ONLY, NOT_SET, "")
    , pokefinder_seeds(false, "<b>PokeFinder Seeds:</b>", LockMode::READ_ONLY, NOT_SET, "")
    , advances(false, "<b>Current Advance:</b>", LockMode::READ_ONLY, NOT_SET, "")
    , confidence(false, "<b>Confidence:</b>", LockMode::READ_ONLY, NOT_SET, "")
{
    PA_ADD_STATIC(state);
    PA_ADD_STATIC(pokefinder_seeds);
    PA_ADD_STATIC(advances);
    PA_ADD_STATIC(confidence);
}

void RngStateDisplay::set_state(const Xorshift128State& value, uint64_t advance_count){
    state.set(value.to_string());

    uint64_t seed0 = 0;
    uint64_t seed1 = 0;
    xorshift128_state_to_seed_pair(value, seed0, seed1);
    pokefinder_seeds.set(tostr_hex_padded(16, seed0) + " / " + tostr_hex_padded(16, seed1));

    set_advances(advance_count);
}
void RngStateDisplay::set_advances(uint64_t advance_count){
    advances.set(tostr_u_commas((int64_t)advance_count));
}

void RngStateDisplay::set_confidence_unique(){
    confidence.set("Unique");
}

void RngStateDisplay::reset(){
    state.set(NOT_SET);
    pokefinder_seeds.set(NOT_SET);
    advances.set(NOT_SET);
    confidence.set(NOT_SET);
}



BlinkCollectionDisplay::BlinkCollectionDisplay(bool blinks_only)
    : GroupOption("Blink Collection", LockMode::READ_ONLY)
    , progress(false, "<b>Blinks:</b>", LockMode::READ_ONLY, NOT_SET, "")
{
    PA_ADD_STATIC(progress);
}

void BlinkCollectionDisplay::set_progress(size_t collected, size_t wanted){
    progress.set(std::to_string(collected) + " / " + std::to_string(wanted));
}
void BlinkCollectionDisplay::set_note(const std::string& text){
    progress.set(text);
}

void BlinkCollectionDisplay::reset(){
    progress.set(NOT_SET);
}


RngTargetDisplay::RngTargetDisplay()
    : GroupOption("Target", LockMode::READ_ONLY)
    , target_advance(false, "<b>Target Advance:</b>", LockMode::READ_ONLY, NOT_SET, "")
    , details(false, "<b>Details:</b>", LockMode::READ_ONLY, NOT_SET, "")
    , correction(false, "<b>Timing Correction:</b>", LockMode::READ_ONLY, NOT_SET, "")
{
    PA_ADD_STATIC(target_advance);
    PA_ADD_STATIC(details);
    PA_ADD_STATIC(correction);
}


static std::string poke_info_str(const BdspPokemonResult& pokemon){
    std::string ret;
    if (pokemon.shiny != BdspShiny::None){
        ret += bdsp_shiny_name(pokemon.shiny);
        ret += " ";
    }
    switch (pokemon.gender){
    case BdspGender::Male:   ret += UNICODE_MALE + " "; break;
    case BdspGender::Female: ret += UNICODE_FEMALE + " "; break;
    default: break;
    }
    ret += bdsp_nature_name(pokemon.nature);
    ret += " Ability" + std::to_string(pokemon.ability);
    ret += " " + pokemon.ivs.to_string();
    return ret;
}

void RngTargetDisplay::set_target(const BdspPokemonResult& pokemon, uint64_t advance_count){
    target_advance.set(tostr_u_commas((int64_t)advance_count));
    details.set(poke_info_str(pokemon));
}

void RngTargetDisplay::set_note(const std::string& reason){
    target_advance.set("—");
    details.set(reason);
}
void RngTargetDisplay::set_correction(int64_t advances){
    correction.set(std::to_string(advances) + " advance(s)");
}
void RngTargetDisplay::reset(){
    target_advance.set(NOT_SET);
    details.set(NOT_SET);
    correction.set(NOT_SET);
}




}
}
}
