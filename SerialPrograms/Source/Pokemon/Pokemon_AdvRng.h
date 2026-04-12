/*  Advance RNG
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_AdvRng_H
#define PokemonAutomation_PokemonSwSh_AdvRng_H

#include <stdint.h>
#include <utility>
#include <vector>
#include "Pokemon_StatsCalculation.h"

namespace PokemonAutomation{
namespace Pokemon{

struct IvGroup{
    uint8_t iv0;
    uint8_t iv1;
    uint8_t iv2;
};

struct IVs{
    uint8_t hp = 0;
    uint8_t attack = 0;
    uint8_t defense = 0;
    uint8_t spatk = 0;
    uint8_t spdef = 0;
    uint8_t speed = 0;
};

enum class Nature{
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

enum class Gender{
    Male,
    Female,
    Any
};

enum class Ability{
    Zero,
    One,
    Any
};

enum class ShinyType{
    Normal,
    Star,
    Square,
    Any
};


struct AdvRngState{
    AdvRngState(uint16_t seed, uint64_t advance, uint32_t s0, uint32_t s1, uint32_t s2, uint32_t s3, uint32_t s4);
    uint16_t seed;
    uint64_t advance;
    uint32_t s0;
    uint32_t s1;
    uint32_t s2;
    uint32_t s3;
    uint32_t s4;
};

enum class RngMethod{
    Method1,
    Method2,
    Method4,
    Any
};

struct AdvPokemonResult{
    AdvPokemonResult(uint32_t pid, uint16_t gender, Nature nature, Ability ability, IVs ivs);
    uint32_t pid;
    uint8_t gender;
    Nature nature;
    Ability ability;
    IVs ivs;
};

struct AdvRngFilters{
    Gender gender;
    Nature nature;
    Ability ability;
    IvRanges ivs;
    ShinyType shiny;
    RngMethod method;
};

class AdvRng{
public:
    uint16_t seed;
    AdvRngState state;

    std::map<AdvRngState, AdvPokemonResult> search(AdvRngFilters& target, std::vector<uint16_t>& seeds, uint64_t min_advances, uint64_t max_advances, uint16_t tid_xor_sid = 0, uint8_t gender_threshold = 126);
private:
    AdvRng(uint16_t seed, AdvRngState state);
    AdvRng(uint16_t seed, uint64_t min_advances);

    void set_seed(uint16_t seed);
    void set_state_advances(uint64_t advances);
    void advance_state();
    void search_advance_range(std::map<AdvRngState, AdvPokemonResult>& hits, AdvRngFilters& target, uint64_t min_advances, uint64_t max_advances, uint16_t tid_xor_sid, uint8_t gender_threshold);
};

}
}
#endif
