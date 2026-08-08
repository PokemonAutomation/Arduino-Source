/*  BDSP RNG
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Pokemon generation for Brilliant Diamond / Shining Pearl, ported from
 *  PokeFinder's Gen 8 generators.
 *
 *  The main game RNG is xorshift128 (see Pokemon_Xorshift128.h). Most values are
 *  read through bdsp_gen_transform(). Some encounters then seed a separate
 *  Xoroshiro128+ from a single xorshift128 output and generate everything else
 *  from that.
 *
 *  Only the static/roamer and trainer-ID paths are implemented. Wild and egg
 *  generation additionally need per-species data and encounter tables, which
 *  this project does not have for BDSP yet.
 *
 */

#ifndef PokemonAutomation_Pokemon_BdspRng_H
#define PokemonAutomation_Pokemon_BdspRng_H

#include <stddef.h>
#include <stdint.h>
#include <functional>
#include <string>
#include <vector>
#include "Pokemon_NatureChecker.h"
#include "Pokemon_StatsCalculation.h"
#include "Pokemon_Xoroshiro128Plus.h"
#include "Pokemon_Xorshift128.h"

namespace PokemonAutomation{
    class Cancellable;
namespace Pokemon{


//
//  Xoroshiro128+, BDSP flavor
//

uint64_t bdsp_splitmix64(uint64_t seed);

//  Sign-extend a 32-bit RNG output to 64 bits. Egg generation does this before
//  seeding; roamer generation does not.
inline uint64_t bdsp_sign_extend_seed(uint32_t seed){
    return (uint64_t)(int64_t)(int32_t)seed;
}

//  Same core as Xoroshiro128Plus, but seeded through splitmix and bounded
//  differently. Do not substitute Xoroshiro128Plus::nextInt() here: it uses
//  power-of-two rejection sampling, which is what Sword/Shield does, not BDSP.
class XoroshiroBDSP{
public:
    explicit XoroshiroBDSP(uint64_t seed);

    uint64_t next(){ return m_rng.next(); }
    uint32_t next_uint(uint32_t maximum){ return (uint32_t)((m_rng.next() >> 32) % maximum); }

private:
    Xoroshiro128Plus m_rng;
};



//
//  Results
//

enum class BdspShiny : uint8_t{
    None = 0,
    Star = 1,
    Square = 2,
};
const char* bdsp_shiny_name(BdspShiny shiny);

enum class BdspGender : uint8_t{
    Male = 0,
    Female = 1,
    Genderless = 2,
};
const char* bdsp_gender_name(BdspGender gender);

struct BdspIVs{
    //  BDSP's order, which is also PokeFinder's: HP, Atk, Def, SpA, SpD, Spe.
    //  This is NOT the Gen 3 order used by AdvIVs in Pokemon_AdvRng.h, which puts
    //  Speed at index 3. Do not unify them.
    uint8_t hp = 0;
    uint8_t attack = 0;
    uint8_t defense = 0;
    uint8_t spatk = 0;
    uint8_t spdef = 0;
    uint8_t speed = 0;

    uint8_t& operator[](size_t index);
    uint8_t operator[](size_t index) const;

    std::string to_string() const;
};

struct BdspPokemonResult{
    uint32_t ec = 0;
    uint32_t pid = 0;
    BdspShiny shiny = BdspShiny::None;
    BdspIVs ivs;
    uint8_t ability = 0;
    BdspGender gender = BdspGender::Genderless;
    uint8_t nature = 0;     //  Game index, 0-24. See bdsp_nature_name().
    uint8_t level = 1;
    uint8_t height = 0;
    uint8_t weight = 0;

    std::string to_string() const;
};

struct BdspIdResult{
    uint32_t sidtid = 0;
    uint16_t tid = 0;
    uint16_t sid = 0;
    //  The 6-digit number the trainer card shows under "ID No.". Derived from the
    //  whole 32-bit draw, so it is neither the TID nor the SID.
    uint32_t display_tid = 0;

    uint16_t tsv() const{ return (uint16_t)(tid ^ sid); }
    std::string to_string() const;
};


//  Natures are indexed in the game's own order (0 = Hardy), which is not the
//  order of NatureCheckerValue. Convert before comparing against anything from
//  the UI or from an IV/nature reader.
const char* bdsp_nature_name(uint8_t nature);
NatureCheckerValue bdsp_nature_to_checker_value(uint8_t nature);

//  psv == tsv is a square shiny; a difference under 16 is a star.
BdspShiny bdsp_get_shiny(uint32_t pid, uint16_t tsv);
bool bdsp_is_shiny(uint32_t pid, uint16_t tsv);



//
//  Generation
//

//  Passed as "synchronize_nature" when no Synchronize lead is active.
const uint8_t BDSP_NO_SYNCHRONIZE = 0xff;

struct BdspStaticTemplate{
    std::string species;
    uint8_t level = 1;

    //  How many IVs are forced to 31 before the rest are rolled.
    uint8_t guaranteed_ivs = 0;

    //  0 or 1 forces that ability slot. 2 is the hidden ability, which still
    //  burns one roll. 3 rolls between the two normal abilities.
    uint8_t ability_kind = 3;

    //  255 = genderless, 254 = always female, 0 = always male. Otherwise a roll
    //  must come in under this value to be female.
    uint8_t gender_ratio = 255;

    //  Shiny-locked encounters still roll a PID, then force it non-shiny.
    bool shiny_locked = false;

    //  Roamers (Cresselia, Mesprit) take one value from the main RNG as the EC,
    //  then generate everything else from a Xoroshiro seeded with it.
    bool roamer = false;
};

//  Each of these takes the generator positioned at the target advance. The copy
//  is deliberate: callers keep their own position.
BdspPokemonResult bdsp_generate_static(
    Xorshift128 rng, const BdspStaticTemplate& tmpl,
    uint16_t tsv, uint8_t synchronize_nature = BDSP_NO_SYNCHRONIZE
);
BdspPokemonResult bdsp_generate_roamer(
    Xorshift128 rng, const BdspStaticTemplate& tmpl,
    uint16_t tsv, uint8_t synchronize_nature = BDSP_NO_SYNCHRONIZE
);
//  Dispatches on tmpl.roamer.
BdspPokemonResult bdsp_generate(
    Xorshift128 rng, const BdspStaticTemplate& tmpl,
    uint16_t tsv, uint8_t synchronize_nature = BDSP_NO_SYNCHRONIZE
);

BdspIdResult bdsp_generate_id(Xorshift128 rng);



//
//  Filtering and searching
//

struct BdspRngHit{
    uint64_t advances = 0;
    Xorshift128State state;
    BdspPokemonResult result;
};


class BdspStaticSearcher{
public:
    BdspStaticSearcher(
        const Xorshift128State& base_state,
        BdspStaticTemplate tmpl,
        uint16_t tsv,
        uint8_t synchronize_nature = BDSP_NO_SYNCHRONIZE
    );

    const BdspStaticTemplate& pokemon_template() const{ return m_template; }

    //  The Pokemon produced "advances" steps after the base state.
    //
    //  This jumps from the base state on every call, which is the right thing for one
    //  advance and the wrong thing for a range: use scan() to sweep one, or the cost
    //  is a fresh jump per advance rather than a single step.
    BdspPokemonResult generate(uint64_t advances) const;

    //  Every advance in [min_advances, max_advances] that "accept" accepts, earliest
    //  first, or just the first of them if "stop_at_first".
    //
    //  Walks forward one advance at a time through a sliding window, so sweeping a
    //  whole search window costs about what a single generate() does. The predicate
    //  is the caller's because programs filter through the shared UI tables, and a
    //  second filter type here would only be those tables written out twice.
    std::vector<BdspRngHit> scan(
        uint64_t min_advances, uint64_t max_advances,
        const std::function<bool(const BdspPokemonResult&)>& accept,
        bool stop_at_first = false,
        Cancellable* cancellable = nullptr
    ) const;

private:
    Xorshift128State m_base_state;
    BdspStaticTemplate m_template;
    uint16_t m_tsv;
    uint8_t m_synchronize_nature;
};


}
}
#endif
