/*  Adv RNG
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cstddef>
#include "Pokemon_AdvRng.h"

namespace PokemonAutomation{
namespace Pokemon{

uint32_t increment_internal_rng_state(uint32_t& state){
    return state * 0x41c64e6d + 0x6073;
}

AdvRngState rngstate_from_internal_state(uint16_t seed, uint64_t advances, uint32_t& state, RngMethod method){
    uint32_t s0 = state;
    uint32_t s1 = increment_internal_rng_state(s0);
    uint32_t s2 = increment_internal_rng_state(s1);
    uint32_t s3 = increment_internal_rng_state(s2);
    uint32_t s4 = increment_internal_rng_state(s3);

    return  {seed, advances, method, s0, s1, s2, s3, s4};
}

AdvRngState rngstate_from_seed(uint16_t& seed, uint64_t advances, RngMethod method){
    uint32_t state = seed;
    state = increment_internal_rng_state(state);
    for (uint64_t i=0; i<advances; i++){ 
        state = increment_internal_rng_state(state);
    }

    return rngstate_from_internal_state(seed, advances, state, method);
}

void advance_rng_state(AdvRngState& state){
    state.advance++;
    state.s0 = state.s1;
    state.s1 = state.s2;
    state.s2 = state.s3;
    state.s3 = state.s4;
    state.s4 = increment_internal_rng_state(state.s4);
}

uint32_t pid_from_states(uint32_t& s0, uint32_t& s1){
    return (s1 & 0xffff0000) + (s0 >> 16);
}

uint8_t gender_value_from_pid(uint32_t& pid){
    return pid & 0xff;
}

Gender gender_from_gender_value(uint8_t gender_value, uint8_t threshold){
    return (gender_value <= threshold) ? Gender::Female : Gender::Male;
}

Nature nature_from_pid(uint32_t& pid){
    return Nature (pid % 25);
}

Ability ability_from_pid(uint32_t& pid){
    return Ability (pid % 2);
}

IvGroup iv_group_from_state(uint32_t& state){
    IvGroup ivgroup;
    uint32_t remainingbits = state;

    ivgroup.iv0 = (remainingbits & 0xfffff) % 32;
    remainingbits = remainingbits >> 5;
    ivgroup.iv1 = (remainingbits & 0xfffff) % 32;
    remainingbits = remainingbits >> 5;
    ivgroup.iv2 = (remainingbits & 0xfffff) % 32;
    remainingbits = remainingbits >> 5;

    return ivgroup;
}

AdvPokemonResult pokemon_from_state(AdvRngState& state, RngMethod method = RngMethod::Method1){
    uint32_t pid = pid_from_states(state.s0, state.s1);
    uint8_t gender = gender_value_from_pid(pid);
    Nature nature = nature_from_pid(pid);
    Ability ability = ability_from_pid(pid);

    IvGroup ivgroup1;
    IvGroup ivgroup2;
    switch(method){

    case RngMethod::Method2:
        ivgroup1 = iv_group_from_state(state.s3);
        ivgroup2 = iv_group_from_state(state.s4);
        break;
    case RngMethod::Method4:
        ivgroup1 = iv_group_from_state(state.s2);
        ivgroup2 = iv_group_from_state(state.s4);
        break;
    case RngMethod::Method1:
    default:
        ivgroup1 = iv_group_from_state(state.s2);
        ivgroup2 = iv_group_from_state(state.s3);
        break;
    }

    IVs ivs;
    ivs.hp = ivgroup1.iv0;
    ivs.attack = ivgroup1.iv1;
    ivs.defense = ivgroup1.iv2;
    ivs.spatk = ivgroup2.iv0;
    ivs.spdef = ivgroup2.iv1;
    ivs.speed = ivgroup2.iv2;

    return {pid, gender, nature, ability, ivs};
}

ShinyType shiny_type_from_pid(uint32_t pid, uint16_t tid_xor_sid){
    uint16_t pid0 = pid >> 16;
    uint16_t pid1 = pid & 0xffff;
    uint16_t pid_xor (pid0 ^ pid1);

    if (pid_xor == tid_xor_sid){
        return ShinyType::Square;
    }else if ((pid_xor ^ tid_xor_sid) < 8){
        return ShinyType::Star;
    }else{
        return ShinyType::Normal;
    }
}


bool check_for_match(AdvPokemonResult res, AdvRngFilters target, uint16_t tid_xor_sid, uint8_t gender_threshold){
    return (target.nature == Nature::Any || res.nature == target.nature)
        && (target.ability == Ability::Any || res.ability == target.ability)
        && (target.gender == Gender::Any || gender_from_gender_value(res.gender, gender_threshold) == target.gender)
        && (target.shiny == ShinyType::Any || shiny_type_from_pid(res.pid, tid_xor_sid) == target.shiny)
        && (target.ivs.hp.low <= res.ivs.hp && target.ivs.hp.high >= res.ivs.hp)
        && (target.ivs.attack.low <= res.ivs.attack && target.ivs.attack.high >= res.ivs.attack)
        && (target.ivs.defense.low <= res.ivs.defense && target.ivs.defense.high >= res.ivs.defense)
        && (target.ivs.spatk.low <= res.ivs.spatk && target.ivs.spatk.high >= res.ivs.spatk)
        && (target.ivs.spdef.low <= res.ivs.spdef && target.ivs.spdef.high >= res.ivs.spdef)
        && (target.ivs.speed.low <= res.ivs.speed && target.ivs.speed.high >= res.ivs.speed);
}



AdvRng::AdvRng(uint16_t seed, AdvRngState state)
    : seed(seed)
    , state(state)
{}

AdvRng::AdvRng(uint16_t seed, uint64_t min_advances, RngMethod method)
    : seed(seed)
    , state(rngstate_from_seed(seed, min_advances, method))
{}

void AdvRng::advance_state(){
    advance_rng_state(state);
}

void AdvRng::set_seed(uint16_t newseed){
    seed = newseed;
    state = rngstate_from_seed(seed, 0, state.method);
}

void AdvRng::set_state_advances(uint64_t advances){
    state = rngstate_from_seed(seed, advances, state.method);
}

void AdvRng::search_advance_range(
    std::map<AdvRngState, AdvPokemonResult>& hits,
    AdvRngFilters& target,
    uint64_t min_advances,
    uint64_t max_advances,
    uint16_t tid_xor_sid,
    uint8_t gender_threshold
){
    for (uint8_t m=0; m<3; m++){
        set_state_advances(min_advances);

        RngMethod method;
        switch (m){
        case 1:
            method = RngMethod::Method2;
            break;
        case 2:
            method = RngMethod::Method4;
            break;
        case 0:
        default:
            method = RngMethod::Method1;
            break;
        }

        if ((target.method != RngMethod::Any) && (target.method != method)){
            continue;
        }

        for (uint64_t a=min_advances; a<max_advances; a++){
            AdvPokemonResult res = pokemon_from_state(state, method);
            bool match = check_for_match(res, target, tid_xor_sid, gender_threshold);
            if (match){
                hits[state] = res;
            }
        }
    }
}

std::map<AdvRngState, AdvPokemonResult> AdvRng::search(
    AdvRngFilters& target,
    std::vector<uint16_t>& seeds,
    uint64_t min_advances,
    uint64_t max_advances,
    uint16_t tid_xor_sid,
    uint8_t gender_threshold
){
    std::map<AdvRngState, AdvPokemonResult> hits;
    for (uint16_t seed : seeds){
        set_seed(seed);
        search_advance_range(hits, target, min_advances, max_advances, tid_xor_sid, gender_threshold);
    }
    return hits;
}


}
}
