/*  Adv RNG
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef Pokemon_AdvRng_H
#define Pokemon_AdvRng_H

#include <stdint.h>
#include <utility>
#include <vector>
#include <stdexcept>
#include "Pokemon_StatsCalculation.h"

namespace PokemonAutomation{
namespace Pokemon{

struct AdvIvGroup{
    uint8_t iv0;
    uint8_t iv1;
    uint8_t iv2;
};

struct AdvIVs{
    uint8_t hp = 0;
    uint8_t attack = 0;
    uint8_t defense = 0;
    uint8_t spatk = 0;
    uint8_t spdef = 0;
    uint8_t speed = 0;

    uint8_t& operator[](int index){
        switch (index){
        case 0: 
            return hp;
        case 1:
            return attack;
        case 2:
            return defense;
        case 3:
            return speed;
        case 4:
            return spatk;
        case 5:
            return spdef;
        default:
            throw std::runtime_error("Invalid IVs index. Please report this as a bug.");
        }
    }
};

enum class AdvNature{
    Hardy,
    Lonely,
    Brave,
    Adamant,
    Naughty,
    Bold,
    Docile,
    Relaxed,
    Impish,
    Lax,
    Timid,
    Hasty,
    Serious,
    Jolly,
    Naive,
    Modest,
    Mild,
    Quiet,
    Bashful,
    Rash,
    Calm,
    Gentle,
    Sassy,
    Careful,
    Quirky,
    Any
};

enum class AdvGender{
    Male,
    Female,
    Any
};

enum class AdvAbility{
    Zero,
    One,
    Any
};

enum class AdvShinyType{
    Normal,
    Star,
    Square,
    Any
};

enum class AdvRngMethod{
    Method1, // egg normal
    Method2, // egg split
    Method3, // egg alternate
    Method4, // egg mixed
    Any
};

enum class AdvEggCompatibility{
    low,
    medium,
    high
};

struct AdvRngState{
    uint16_t seed;
    uint64_t advance;
    AdvRngMethod method;
    uint32_t s0;
    uint32_t s1;
    uint32_t s2;
    uint32_t s3;
    uint32_t s4;
    uint32_t s5;

    bool operator<(const AdvRngState& rhs) const noexcept{
        return this->advance < rhs.advance;
    }

    bool operator==(const AdvRngState& rhs) const noexcept{
        return (
            this->advance == rhs.advance
            && this->seed == rhs.seed\
        );
    }
};

struct AdvEncounterSlot{
    std::string species;
    uint8_t minlevel;
    uint8_t maxlevel;
};

struct AdvPokemonResult{
    uint32_t pid;
    uint8_t gender;
    AdvNature nature;
    AdvAbility ability;
    AdvIVs ivs;
};

struct AdvWildPokemonResult{
    std::string species;
    uint32_t pid;
    uint8_t gender;
    AdvNature nature;
    AdvAbility ability;
    AdvIVs ivs;
    uint8_t slot;
    uint8_t level;
};

struct AdvEggResult{
    uint32_t pid;
    uint8_t gender;
    AdvNature nature;
    AdvAbility ability;
    AdvIVs ivs;
    AdvIVs inherited_ivs;
};

struct AdvObservedPokemon{
    std::string species;
    AdvGender gender;
    AdvNature nature;
    AdvAbility ability;
    std::vector<uint8_t> level;
    std::vector<StatReads> stats;
    std::vector<EVs> evs;
    AdvShinyType shiny;
};

struct AdvRngFilters{
    std::string species;
    uint8_t level;
    AdvGender gender;
    AdvNature nature;
    AdvAbility ability;
    IvRanges ivs;
    AdvShinyType shiny;
    AdvRngMethod method;
};

// updates the AdvObservedPokemon with info from leveling up
// assumes levels are earned sequentially
// input EVs are the ones earned since the last level up, not the total
void level_up_observed_pokemon(AdvObservedPokemon& pokemon, const StatReads& newstats, const EVs& evyield);

// returns the appropriate NatureAdjustments for an AdvNature
Pokemon::NatureAdjustments nature_to_adjustment(AdvNature nature);

Pokemon::AdvNature string_to_nature(const std::string& nature_string);
std::string nature_to_string(const AdvNature& nature);

std::string gender_to_string(const AdvGender& gender);

AdvGender gender_from_gender_value(uint8_t gender_value, int16_t threshold);

// returns search filters that correspond with observed stats
AdvRngFilters observation_to_filters(const AdvObservedPokemon& observation, const BaseStats& basestats, AdvRngMethod method = AdvRngMethod::Method1);

AdvPokemonResult egg_to_pokemon(
    AdvEggResult& egg_result,
    AdvIVs& parentA_ivs, 
    AdvIVs& parentB_ivs
);

class AdvRngSearcher{
public:
    uint16_t seed;
    AdvRngState state;
    bool roaming;

    AdvRngSearcher(uint16_t seed, AdvRngState state, bool roaming = false);
    AdvRngSearcher(uint16_t seed, uint64_t min_advances, AdvRngMethod method = AdvRngMethod::Method1, bool roaming = false);

    void set_seed(uint16_t seed);
    void set_state_advances(uint64_t advances);
    void advance_state();

    AdvPokemonResult generate_pokemon();

    std::vector<AdvRngState> search(
        AdvRngFilters& target,
        const std::vector<uint16_t>& seeds,
        uint64_t min_advances,
        uint64_t max_advances,
        int16_t gender_threshold = 126,
        uint16_t tid_xor_sid = 0
    );

private:
    void search_advance_range(
        std::vector<AdvRngState>& hits,
        AdvRngFilters& target,
        uint64_t min_advances,
        uint64_t max_advances,
        int16_t gender_threshold,
        uint16_t tid_xor_sid
    );
};

class AdvRngWildSearcher{
public:
    uint16_t seed;
    AdvRngState state;
    const std::vector<AdvEncounterSlot>& encounter_slots;

    AdvRngWildSearcher(uint16_t seed, AdvRngState state, const std::vector<AdvEncounterSlot>& encounter_slots);
    AdvRngWildSearcher(uint16_t seed, uint64_t min_advances, const std::vector<AdvEncounterSlot>& encounter_slots, AdvRngMethod method = AdvRngMethod::Any);

    void set_seed(uint16_t seed);
    void set_state_advances(uint64_t advances);
    void advance_state();

    AdvWildPokemonResult generate_pokemon(bool super_rod = false);

    std::vector<AdvRngState> search(
        AdvRngFilters& target,
        const std::vector<uint16_t>& seeds,
        uint64_t min_advances,
        uint64_t max_advances,
        int16_t gender_threshold = 126,
        bool super_rod = false,
        uint16_t tid_xor_sid = 0
    );

private:
    void search_advance_range(
        std::vector<AdvRngState>& hits,
        AdvRngFilters& target,
        uint64_t min_advances,
        uint64_t max_advances,
        int16_t gender_threshold,
        bool super_rod,
        uint16_t tid_xor_sid
    );
};


class AdvRngEggSearcher{
public:
    uint16_t held_seed;
    AdvRngState held_state;

    uint16_t pickup_seed;
    AdvRngState pickup_state;

    AdvRngEggSearcher(uint16_t held_seed, AdvRngState held_state, uint16_t pickup_seed, AdvRngState pickup_state);
    AdvRngEggSearcher(
        uint16_t held_seed, uint64_t min_seed_advances, 
        uint16_t pickup_seed, uint64_t min_pickup_advances, 
        AdvRngMethod method = AdvRngMethod::Any
    );

    void set_held_seed(uint16_t seed);
    void set_held_state_advances(uint64_t advances);
    void advance_held_state();

    void set_pickup_seed(uint16_t seed);
    void set_pickup_state_advances(uint64_t advances);
    void advance_pickup_state();

    AdvEggResult generate_egg();
    AdvPokemonResult generate_pokemon(AdvIVs& parentA_ivs, AdvIVs& parentB_ivs);

    std::vector<std::pair<AdvRngState, AdvRngState>> search(
        AdvRngFilters& target,
        const std::vector<uint16_t>& held_seeds,
        uint64_t min_held_advances,
        uint64_t max_held_advances,
        const std::vector<uint16_t>& pickup_seeds,
        uint64_t min_pickup_advances,
        uint64_t max_pickup_advances,
        AdvIVs& parentA_ivs,
        AdvIVs& parentB_ivs,
        AdvEggCompatibility compatibility,
        int16_t gender_threshold = 126,
        uint16_t tid_xor_sid = 0
    );

private:

    void search_held_advances_range(
        std::vector<std::pair<AdvRngState, AdvRngState>>& hits,
        AdvRngFilters& target,
        uint64_t min_held_advances,
        uint64_t max_held_advances,
        const std::vector<uint16_t>& pickup_seeds,
        uint64_t min_pickup_advances,
        uint64_t max_pickup_advances,
        AdvIVs& parentA_ivs,
        AdvIVs& parentB_ivs,
        AdvEggCompatibility compatibility,
        int16_t gender_threshold,
        uint16_t tid_xor_sid
    );

    void search_pickups(
        std::vector<std::pair<AdvRngState, AdvRngState>>& hits,
        AdvRngFilters& target,
        uint16_t held_pid_half,
        const std::vector<uint16_t>& pickup_seeds,
        uint64_t min_pickup_advances,
        uint64_t max_pickup_advances,
        AdvIVs& parentA_ivs,
        AdvIVs& parentB_ivs,
        int16_t gender_threshold,
        uint16_t tid_xor_sid
    );

    void search_pickup_advances_range(
        std::vector<std::pair<AdvRngState, AdvRngState>>& hits,
        AdvRngFilters& target,
        uint16_t held_pid_half,
        uint64_t min_pickup_advances,
        uint64_t max_pickup_advances,
        AdvIVs& parentA_ivs,
        AdvIVs& parentB_ivs,
        int16_t gender_threshold,
        uint16_t tid_xor_sid
    );
};


}
}
#endif
