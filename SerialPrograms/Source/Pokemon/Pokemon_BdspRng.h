/*  BDSP RNG
 *
 *  From: https://github.com/PokemonAutomation/
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


uint64_t bdsp_splitmix64(uint64_t seed);


inline uint64_t bdsp_sign_extend_seed(uint32_t seed){
    return (uint64_t)(int64_t)(int32_t)seed;
}

class XoroshiroBDSP{
public:
    explicit XoroshiroBDSP(uint64_t seed);

    uint64_t next(){ return m_rng.next(); }
    uint32_t next_uint(uint32_t maximum){ return (uint32_t)((m_rng.next() >> 32) % maximum); }

private:
    Xoroshiro128Plus m_rng;
};


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
    uint32_t display_tid = 0;

    uint16_t tsv() const{ return (uint16_t)(tid ^ sid); }
    std::string to_string() const;
};


const char* bdsp_nature_name(uint8_t nature);
NatureCheckerValue bdsp_nature_to_checker_value(uint8_t nature);

//  psv == tsv is a square shiny; a difference under 16 is a star
BdspShiny bdsp_get_shiny(uint32_t pid, uint16_t tsv);
bool bdsp_is_shiny(uint32_t pid, uint16_t tsv);


const uint8_t BDSP_NO_SYNCHRONIZE = 0xff;

struct BdspStaticTemplate{
    std::string species;
    uint8_t level = 1;
    uint8_t guaranteed_ivs = 0;
    uint8_t ability_kind = 3;
    uint8_t gender_ratio = 255;
    bool shiny_locked = false;
    bool roamer = false;
};

BdspPokemonResult bdsp_generate_static(
    Xorshift128 rng, const BdspStaticTemplate& tmpl,
    uint16_t tsv, uint8_t synchronize_nature = BDSP_NO_SYNCHRONIZE
);
BdspPokemonResult bdsp_generate_roamer(
    Xorshift128 rng, const BdspStaticTemplate& tmpl,
    uint16_t tsv, uint8_t synchronize_nature = BDSP_NO_SYNCHRONIZE
);
BdspPokemonResult bdsp_generate(
    Xorshift128 rng, const BdspStaticTemplate& tmpl,
    uint16_t tsv, uint8_t synchronize_nature = BDSP_NO_SYNCHRONIZE
);

BdspIdResult bdsp_generate_id(Xorshift128 rng);


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

    BdspPokemonResult generate(uint64_t advances) const;

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
