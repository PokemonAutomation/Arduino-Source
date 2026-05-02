/*  Adv RNG
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cstddef>
#include <algorithm>
#include "Pokemon_AdvRng.h"

namespace PokemonAutomation{
namespace Pokemon{

void level_up_observed_pokemon(AdvObservedPokemon& pokemon, const StatReads& newstats, const EVs& evyield){
    uint8_t newlevel = pokemon.level.back() + 1;
    pokemon.level.emplace_back(newlevel);

    pokemon.stats.emplace_back(newstats);

    EVs old_evs = pokemon.evs.back();
    EVs new_evs;
    new_evs.hp      = old_evs.hp      + evyield.hp;
    new_evs.attack  = old_evs.attack  + evyield.attack;
    new_evs.defense = old_evs.defense + evyield.defense;
    new_evs.spatk   = old_evs.spatk   + evyield.spatk;
    new_evs.spdef   = old_evs.spdef   + evyield.spdef;
    new_evs.speed   = old_evs.speed   + evyield.speed;
    pokemon.evs.emplace_back(new_evs);
}

uint32_t increment_internal_rng_state(uint32_t& state){
    return state * 0x41c64e6d + 0x6073;
}

AdvRngState rngstate_from_internal_state(uint16_t seed, uint64_t advances, uint32_t& state, AdvRngMethod method){
    uint32_t s0 = state;
    uint32_t s1 = increment_internal_rng_state(s0);
    uint32_t s2 = increment_internal_rng_state(s1);
    uint32_t s3 = increment_internal_rng_state(s2);
    uint32_t s4 = increment_internal_rng_state(s3);

    return  {seed, advances, method, s0, s1, s2, s3, s4};
}

AdvRngState rngstate_from_seed(uint16_t seed, uint64_t advances, AdvRngMethod method){
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

AdvGender gender_from_gender_value(uint8_t gender_value, int16_t threshold){
    return (gender_value <= threshold) ? AdvGender::Female : AdvGender::Male;
}

AdvNature nature_from_pid(uint32_t& pid){
    return AdvNature (pid % 25);
}

AdvAbility ability_from_pid(uint32_t& pid){
    return AdvAbility (pid % 2);
}

AdvIvGroup iv_group_from_state(uint32_t& state){
    AdvIvGroup ivgroup;
    uint32_t remainingbits = state >> 16;

    ivgroup.iv0 = remainingbits & 0x1f;
    remainingbits = remainingbits >> 5;
    ivgroup.iv1 = remainingbits & 0x1f;
    remainingbits = remainingbits >> 5;
    ivgroup.iv2 = remainingbits & 0x1f;

    return ivgroup;
}

AdvPokemonResult pokemon_from_state(AdvRngState& state, uint32_t pid, AdvNature nature){
    uint8_t gender = gender_value_from_pid(pid);
    AdvAbility ability = ability_from_pid(pid);

    AdvIvGroup ivgroup1;
    AdvIvGroup ivgroup2;
    switch(state.method){

    case AdvRngMethod::Method2:
        ivgroup1 = iv_group_from_state(state.s3);
        ivgroup2 = iv_group_from_state(state.s4);
        break;
    case AdvRngMethod::Method4:
        ivgroup1 = iv_group_from_state(state.s2);
        ivgroup2 = iv_group_from_state(state.s4);
        break;
    case AdvRngMethod::Method1:
    default:
        ivgroup1 = iv_group_from_state(state.s2);
        ivgroup2 = iv_group_from_state(state.s3);
        break;
    }

    AdvIVs ivs;
    ivs.hp = ivgroup1.iv0;
    ivs.attack = ivgroup1.iv1;
    ivs.defense = ivgroup1.iv2;
    ivs.speed = ivgroup2.iv0;
    ivs.spatk = ivgroup2.iv1;
    ivs.spdef = ivgroup2.iv2;

    return {pid, gender, nature, ability, ivs};
}

AdvPokemonResult pokemon_from_state(AdvRngState& state){
    uint32_t pid = pid_from_states(state.s0, state.s1);
    AdvNature nature = nature_from_pid(pid);
    return pokemon_from_state(state, pid, nature);
}

AdvPokemonResult reroll_wild_pokemon(uint8_t nature, uint16_t seed, uint64_t advances, uint32_t state, AdvRngMethod method){
    uint32_t lowstate = state;
    uint32_t highstate = increment_internal_rng_state(state);
    while (true){
        uint32_t pid = pid_from_states(lowstate, highstate);
        if ((pid % 25) == nature){
            AdvRngState rngstate = rngstate_from_internal_state(seed, advances, lowstate, method);
            return pokemon_from_state(rngstate, pid, AdvNature(nature));
        }else{
            lowstate = increment_internal_rng_state(highstate);
            highstate = increment_internal_rng_state(lowstate);
        }
    }
}

uint8_t slot_number_from_roll(uint8_t roll, size_t size, bool super_rod) {
    switch(size){
        case 12:    // grass
        if (roll < 20){
           return 0;
        }else if(roll < 40){
           return 1;
        }else if(roll < 50){
           return 2;
        }else if(roll < 60){
           return 3;
        }else if(roll < 70){
           return 4;
        }else if(roll < 80){
           return 5;
        }else if(roll < 85){
           return 6;
        }else if(roll < 90){
           return 7;
        }else if(roll < 94){
           return 8;
        }else if(roll < 98){
           return 9;
        }else if(roll < 99){
           return 10;
        }else{
           return 11;
        }
    case 5:     
        if (super_rod){ // super rod
            if (roll < 40){
               return 0;
                }else if(roll < 70){
               return 1;
                }else if(roll < 85){
               return 2;
                }else if(roll < 95){
               return 3;
                }else{
               return 4;
                }
        }else{
            if (roll < 60){ // surfing and rock smash
               return 0;
                }else if(roll < 90){
               return 1;
                }else if(roll < 95){
               return 2;
                }else if(roll < 99){
               return 3;
                }else{
               return 4;
                }
        }
    case 3: // good rod
        if (roll < 60){
           return 0;
        }else if(roll < 80){
           return 1;
        }else{
           return 2;
        }
    case 2: // old rod
        if (roll < 70){
           return 0;
        }else{
           return 1;
        }
    default:
        return 0;
    }
}

AdvWildPokemonResult wild_pokemon_from_state(AdvRngState state, std::vector<AdvEncounterSlot> slots, bool super_rod){

    uint8_t slot_roll = (state.s0 >> 16) % 100;
    uint16_t level_roll = state.s1 >> 16;
    uint8_t nature = (state.s2 >> 16) % 25;

    uint8_t slot_num = slot_number_from_roll(slot_roll, slots.size(), super_rod);
    AdvEncounterSlot slot = slots[slot_num];

    uint8_t diff = slot.maxlevel - slot.minlevel;
    if (slot.maxlevel < slot.minlevel) {
        diff = 0;
    }
    uint8_t level = slot.minlevel + (level_roll % (diff + 1));

    AdvPokemonResult temp_poke = reroll_wild_pokemon(nature, state.seed, state.advance, state.s3, state.method);

    return {
        slot.species,
        temp_poke.pid,
        temp_poke.gender,
        temp_poke.nature,
        temp_poke.ability,
        temp_poke.ivs,
        slot_num,
        level,
    };
}

AdvShinyType shiny_type_from_pid(uint32_t pid, uint16_t tid_xor_sid){
    uint16_t pid0 = pid >> 16;
    uint16_t pid1 = pid & 0xffff;
    uint16_t pid_xor (pid0 ^ pid1);

    if (pid_xor == tid_xor_sid){
        return AdvShinyType::Square;
    }else if ((pid_xor ^ tid_xor_sid) < 8){
        return AdvShinyType::Star;
    }else{
        return AdvShinyType::Normal;
    }
}


bool check_for_match(AdvPokemonResult res, AdvRngFilters target, int16_t gender_threshold, uint16_t tid_xor_sid){
    return (target.nature == AdvNature::Any || res.nature == target.nature)
        && (target.ability == AdvAbility::Any || res.ability == target.ability)
        && (target.gender == AdvGender::Any || gender_from_gender_value(res.gender, gender_threshold) == target.gender)
        && (target.shiny == AdvShinyType::Any || shiny_type_from_pid(res.pid, tid_xor_sid) == target.shiny)
        && ((target.ivs.hp.low <= res.ivs.hp) && (target.ivs.hp.high >= res.ivs.hp))
        && ((target.ivs.attack.low <= res.ivs.attack) && (target.ivs.attack.high >= res.ivs.attack))
        && ((target.ivs.defense.low <= res.ivs.defense) && (target.ivs.defense.high >= res.ivs.defense))
        && ((target.ivs.spatk.low <= res.ivs.spatk) && (target.ivs.spatk.high >= res.ivs.spatk))
        && ((target.ivs.spdef.low <= res.ivs.spdef) && (target.ivs.spdef.high >= res.ivs.spdef))
        && ((target.ivs.speed.low <= res.ivs.speed) && (target.ivs.speed.high >= res.ivs.speed));
}

bool check_for_match(AdvWildPokemonResult res, AdvRngFilters target, int16_t gender_threshold, uint16_t tid_xor_sid){
    return (target.species == res.species)
        && (target.level == res.level)
        && (target.nature == AdvNature::Any || res.nature == target.nature)
        && (target.ability == AdvAbility::Any || res.ability == target.ability)
        && (target.gender == AdvGender::Any || gender_from_gender_value(res.gender, gender_threshold) == target.gender)
        && (target.shiny == AdvShinyType::Any || shiny_type_from_pid(res.pid, tid_xor_sid) == target.shiny)
        && ((target.ivs.hp.low <= res.ivs.hp) && (target.ivs.hp.high >= res.ivs.hp))
        && ((target.ivs.attack.low <= res.ivs.attack) && (target.ivs.attack.high >= res.ivs.attack))
        && ((target.ivs.defense.low <= res.ivs.defense) && (target.ivs.defense.high >= res.ivs.defense))
        && ((target.ivs.spatk.low <= res.ivs.spatk) && (target.ivs.spatk.high >= res.ivs.spatk))
        && ((target.ivs.spdef.low <= res.ivs.spdef) && (target.ivs.spdef.high >= res.ivs.spdef))
        && ((target.ivs.speed.low <= res.ivs.speed) && (target.ivs.speed.high >= res.ivs.speed));
}

AdvRngSearcher::AdvRngSearcher(uint16_t seed, AdvRngState state)
    : seed(seed)
    , state(state)
{}

AdvRngSearcher::AdvRngSearcher(uint16_t seed, uint64_t min_advances, AdvRngMethod method)
    : seed(seed)
    , state(rngstate_from_seed(seed, min_advances, method))
{}

void AdvRngSearcher::advance_state(){
    advance_rng_state(state);
}

void AdvRngSearcher::set_seed(uint16_t newseed){
    seed = newseed;
    state = rngstate_from_seed(seed, 0, state.method);
}

void AdvRngSearcher::set_state_advances(uint64_t advances){
    state = rngstate_from_seed(seed, advances, state.method);
}

AdvPokemonResult AdvRngSearcher::generate_pokemon(){
    return pokemon_from_state(state);
}

void AdvRngSearcher::search_advance_range(
    std::vector<AdvRngState>& hits,
    AdvRngFilters& target,
    uint64_t min_advances,
    uint64_t max_advances,
    int16_t gender_threshold,
    uint16_t tid_xor_sid
){
    for (uint8_t m=0; m<3; m++){
        set_state_advances(min_advances);

        AdvRngMethod method;
        switch (m){
        case 1:
            method = AdvRngMethod::Method2;
            break;
        case 2:
            method = AdvRngMethod::Method4;
            break;
        case 0:
        default:
            method = AdvRngMethod::Method1;
            break;
        }

        if ((target.method != AdvRngMethod::Any) && (target.method != method)){
            continue;
        }else{
            state.method = method;
        }

        for (uint64_t a=min_advances; a<max_advances; a++){
            AdvPokemonResult res = pokemon_from_state(state);
            bool match = check_for_match(res, target, gender_threshold, tid_xor_sid);
            if (match){
               hits.emplace_back(state);
            }
            advance_state();
        }
    }
}

std::vector<AdvRngState> AdvRngSearcher::search(
    AdvRngFilters& target,
    const std::vector<uint16_t>& seeds,
    uint64_t min_advances,
    uint64_t max_advances,
    int16_t gender_threshold,
    uint16_t tid_xor_sid
){
    std::vector<AdvRngState> hits;
    for (uint16_t seed : seeds){
        set_seed(seed);
        search_advance_range(hits, target, min_advances, max_advances, gender_threshold, tid_xor_sid);
    }
    return hits;
}


AdvRngWildSearcher::AdvRngWildSearcher(uint16_t seed, AdvRngState state, const std::vector<AdvEncounterSlot>& encounter_slots)
    : seed(seed)
    , state(state)
    , encounter_slots(encounter_slots)
{}

AdvRngWildSearcher::AdvRngWildSearcher(uint16_t seed, uint64_t min_advances, const std::vector<AdvEncounterSlot>& encounter_slots, AdvRngMethod method)
    : seed(seed)
    , state(rngstate_from_seed(seed, min_advances, method))
    , encounter_slots(encounter_slots)
{}

void AdvRngWildSearcher::advance_state(){
    advance_rng_state(state);
}

void AdvRngWildSearcher::set_seed(uint16_t newseed){
    seed = newseed;
    state = rngstate_from_seed(seed, 0, state.method);
}

void AdvRngWildSearcher::set_state_advances(uint64_t advances){
    state = rngstate_from_seed(seed, advances, state.method);
}

AdvWildPokemonResult AdvRngWildSearcher::generate_pokemon(bool super_rod){
    return wild_pokemon_from_state(state, encounter_slots, super_rod);
}

void AdvRngWildSearcher::search_advance_range(
    std::vector<AdvRngState>& hits,
    AdvRngFilters& target,
    uint64_t min_advances,
    uint64_t max_advances,
    int16_t gender_threshold,
    bool super_rod,
    uint16_t tid_xor_sid
){
    for (uint8_t m=0; m<3; m++){
        set_state_advances(min_advances);

        AdvRngMethod method;
        switch (m){
        case 1:
            method = AdvRngMethod::Method2;
            break;
        case 2:
            method = AdvRngMethod::Method4;
            break;
        case 0:
        default:
            method = AdvRngMethod::Method1;
            break;
        }

        if ((target.method != AdvRngMethod::Any) && (target.method != method)){
            continue;
        }else{
            state.method = method;
        }

        for (uint64_t a=min_advances; a<max_advances; a++){
            AdvWildPokemonResult res = wild_pokemon_from_state(state, encounter_slots, super_rod);
            bool match = check_for_match(res, target, gender_threshold, tid_xor_sid);
            if (match){
                hits.emplace_back(state);
            }
            advance_state();
        }
    }
}

std::vector<AdvRngState> AdvRngWildSearcher::search(
    AdvRngFilters& target,
    const std::vector<uint16_t>& seeds,
    uint64_t min_advances,
    uint64_t max_advances,
    int16_t gender_threshold,
    bool super_rod,
    uint16_t tid_xor_sid
){
    std::vector<AdvRngState> hits;
    for (uint16_t seed : seeds){
        set_seed(seed);
        search_advance_range(hits, target, min_advances, max_advances, gender_threshold, super_rod, tid_xor_sid);
    }
    return hits;
}


Pokemon::NatureAdjustments nature_to_adjustment(AdvNature nature){
    NatureAdjustments ret;
    ret.attack = NatureAdjustment::NEUTRAL;
    ret.defense = NatureAdjustment::NEUTRAL;
    ret.spatk = NatureAdjustment::NEUTRAL;
    ret.spdef = NatureAdjustment::NEUTRAL;
    ret.speed = NatureAdjustment::NEUTRAL;

    switch (nature){
    case AdvNature::Bashful:
    case AdvNature::Docile:
    case AdvNature::Hardy:
    case AdvNature::Quirky:
    case AdvNature::Serious:
        return ret;

    case AdvNature::Bold:
        ret.attack = NatureAdjustment::NEGATIVE;
        ret.defense = NatureAdjustment::POSITIVE;
        return ret;
    case AdvNature::Modest:
        ret.attack = NatureAdjustment::NEGATIVE;
        ret.spatk = NatureAdjustment::POSITIVE;
        return ret;
    case AdvNature::Calm:
        ret.attack = NatureAdjustment::NEGATIVE;
        ret.spdef = NatureAdjustment::POSITIVE;
        return ret;
    case AdvNature::Timid:
        ret.attack = NatureAdjustment::NEGATIVE;
        ret.speed = NatureAdjustment::POSITIVE;
        return ret;

    case AdvNature::Lonely:
        ret.defense = NatureAdjustment::NEGATIVE;
        ret.attack = NatureAdjustment::POSITIVE;
        return ret;
    case AdvNature::Mild:
        ret.defense = NatureAdjustment::NEGATIVE;
        ret.spatk = NatureAdjustment::POSITIVE;
        return ret;
    case AdvNature::Gentle:
        ret.defense = NatureAdjustment::NEGATIVE;
        ret.spdef = NatureAdjustment::POSITIVE;
        return ret;
    case AdvNature::Hasty:
        ret.defense = NatureAdjustment::NEGATIVE;
        ret.speed = NatureAdjustment::POSITIVE;
        return ret;

    case AdvNature::Adamant:
        ret.spatk = NatureAdjustment::NEGATIVE;
        ret.attack = NatureAdjustment::POSITIVE;
        return ret;
    case AdvNature::Impish:
        ret.spatk = NatureAdjustment::NEGATIVE;
        ret.defense = NatureAdjustment::POSITIVE;
        return ret;
    case AdvNature::Careful:
        ret.spatk = NatureAdjustment::NEGATIVE;
        ret.spdef = NatureAdjustment::POSITIVE;
        return ret;
    case AdvNature::Jolly:
        ret.spatk = NatureAdjustment::NEGATIVE;
        ret.speed = NatureAdjustment::POSITIVE;
        return ret;

    case AdvNature::Naughty:
        ret.spdef = NatureAdjustment::NEGATIVE;
        ret.attack = NatureAdjustment::POSITIVE;
        return ret;
    case AdvNature::Lax:
        ret.spdef = NatureAdjustment::NEGATIVE;
        ret.defense = NatureAdjustment::POSITIVE;
        return ret;
    case AdvNature::Rash:
        ret.spdef = NatureAdjustment::NEGATIVE;
        ret.spatk = NatureAdjustment::POSITIVE;
        return ret;
    case AdvNature::Naive:
        ret.spdef = NatureAdjustment::NEGATIVE;
        ret.speed = NatureAdjustment::POSITIVE;
        return ret;

    case AdvNature::Brave:
        ret.speed = NatureAdjustment::NEGATIVE;
        ret.attack = NatureAdjustment::POSITIVE;
        return ret;
    case AdvNature::Relaxed:
        ret.speed = NatureAdjustment::NEGATIVE;
        ret.defense = NatureAdjustment::POSITIVE;
        return ret;
    case AdvNature::Quiet:
        ret.speed = NatureAdjustment::NEGATIVE;
        ret.spatk = NatureAdjustment::POSITIVE;
        return ret;
    case AdvNature::Sassy:
        ret.speed = NatureAdjustment::NEGATIVE;
        ret.spdef = NatureAdjustment::POSITIVE;
        return ret;

    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unknown Nature: " + std::to_string((int)nature));
    }
}

void shrink_iv_range(IvRange& mutated_range, IvRange& fixed_range){
    mutated_range.low  = std::max(mutated_range.low,  fixed_range.low);
    mutated_range.high = std::min(mutated_range.high, fixed_range.high);
}

void shrink_iv_ranges(IvRanges& mutated_ranges, IvRanges& fixed_ranges){
    shrink_iv_range(mutated_ranges.hp,      fixed_ranges.hp);
    shrink_iv_range(mutated_ranges.attack,  fixed_ranges.attack);
    shrink_iv_range(mutated_ranges.defense, fixed_ranges.defense);
    shrink_iv_range(mutated_ranges.spatk,   fixed_ranges.spatk);
    shrink_iv_range(mutated_ranges.spdef,   fixed_ranges.spdef);
    shrink_iv_range(mutated_ranges.speed,   fixed_ranges.speed);
}

AdvRngFilters observation_to_filters(const AdvObservedPokemon& observation, const BaseStats& basestats, AdvRngMethod method){
    IvRanges filter_iv_ranges = {{0,31},{0,31},{0,31},{0,31},{0,31},{0,31}};
    for (size_t i=0; i<observation.level.size(); i++){
        uint8_t lv = observation.level[i];
        StatReads sts = observation.stats[i];
        EVs ev = observation.evs[i];
        NatureAdjustments nat = nature_to_adjustment(observation.nature);

        IvRanges iv_ranges_at_level = calc_iv_ranges(basestats, lv, ev, sts, nat);
        shrink_iv_ranges(filter_iv_ranges, iv_ranges_at_level);
    }

    return AdvRngFilters { 
        observation.species,
        observation.level[0],
        observation.gender,
        observation.nature,
        observation.ability,
        filter_iv_ranges,
        observation.shiny,
        method
    };
}


}
}
