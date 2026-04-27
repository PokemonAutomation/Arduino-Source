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
    Method1,
    Method2,
    Method4,
    Any
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

    bool operator<(const AdvRngState& rhs) const noexcept{
        return this->advance < rhs.advance;
    }
};

struct AdvPokemonResult{
    uint32_t pid;
    uint8_t gender;
    AdvNature nature;
    AdvAbility ability;
    AdvIVs ivs;
};

struct AdvObservedPokemon{
    AdvGender gender;
    AdvNature nature;
    AdvAbility ability;
    std::vector<uint8_t> level;
    std::vector<StatReads> stats;
    std::vector<EVs> evs;
    AdvShinyType shiny;
};

struct AdvRngFilters{
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

// returns search filters that correspond with observed stats
AdvRngFilters observation_to_filters(const AdvObservedPokemon& observation, const BaseStats& basestats, AdvRngMethod method = AdvRngMethod::Method1);

class AdvRngSearcher{
public:
    uint16_t seed;
    AdvRngState state;

    AdvRngSearcher(uint16_t seed, AdvRngState state);
    AdvRngSearcher(uint16_t seed, uint64_t min_advances, AdvRngMethod method = AdvRngMethod::Method1);

    void set_seed(uint16_t seed);
    void set_state_advances(uint64_t advances);
    void advance_state();

    AdvPokemonResult generate_pokemon();

    std::map<AdvRngState, AdvPokemonResult> search(
        AdvRngFilters& target,
        const std::vector<uint16_t>& seeds,
        uint64_t min_advances,
        uint64_t max_advances,
        int16_t gender_threshold = 126,
        uint16_t tid_xor_sid = 0
    );

    void refine_search(
        std::map<AdvRngState, AdvPokemonResult>& map,
        AdvRngFilters& target,
        int16_t gender_threshold = 126,
        uint16_t tid_xor_sid = 0
    );

private:
    void search_advance_range(
        std::map<AdvRngState, AdvPokemonResult>& hits,
        AdvRngFilters& target,
        uint64_t min_advances,
        uint64_t max_advances,
        int16_t gender_threshold,
        uint16_t tid_xor_sid
    );
};



}
}
#endif
