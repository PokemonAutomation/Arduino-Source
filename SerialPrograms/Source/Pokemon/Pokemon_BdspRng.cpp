/*  BDSP RNG
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <array>
#include <utility>
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Pokemon_BdspRng.h"

namespace PokemonAutomation{
namespace Pokemon{


//  Big enough that a generation can never read past the values the window holds.
//  The size has no effect on results; it is purely a cache.
const size_t STATIC_SEARCH_WINDOW = 64;

//  How often a long search checks whether the user has stopped the program.
const uint64_t CANCEL_CHECK_INTERVAL = 65536;



//
//  Xoroshiro128+, BDSP flavor
//

uint64_t bdsp_splitmix64(uint64_t seed){
    seed = 0xBF58476D1CE4E5B9 * (seed ^ (seed >> 30));
    seed = 0x94D049BB133111EB * (seed ^ (seed >> 27));
    return seed ^ (seed >> 31);
}

XoroshiroBDSP::XoroshiroBDSP(uint64_t seed)
    : m_rng(
        bdsp_splitmix64(seed + 0x9E3779B97F4A7C15),
        bdsp_splitmix64(seed + 0x3C6EF372FE94F82A)
    )
{}



//
//  Results
//

const char* bdsp_shiny_name(BdspShiny shiny){
    switch (shiny){
    case BdspShiny::None:   return "Not Shiny";
    case BdspShiny::Star:   return "Star Shiny";
    case BdspShiny::Square: return "Square Shiny";
    }
    return "?";
}
const char* bdsp_gender_name(BdspGender gender){
    switch (gender){
    case BdspGender::Male:       return "Male";
    case BdspGender::Female:     return "Female";
    case BdspGender::Genderless: return "Genderless";
    }
    return "?";
}

uint8_t& BdspIVs::operator[](size_t index){
    switch (index){
    case 0: return hp;
    case 1: return attack;
    case 2: return defense;
    case 3: return spatk;
    case 4: return spdef;
    case 5: return speed;
    }
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "BdspIVs: Index out of range.");
}
uint8_t BdspIVs::operator[](size_t index) const{
    return const_cast<BdspIVs&>(*this)[index];
}
std::string BdspIVs::to_string() const{
    return std::to_string(hp)
        + "/" + std::to_string(attack)
        + "/" + std::to_string(defense)
        + "/" + std::to_string(spatk)
        + "/" + std::to_string(spdef)
        + "/" + std::to_string(speed);
}

std::string BdspPokemonResult::to_string() const{
    std::string ret;
    ret += "IVs: " + ivs.to_string();
    ret += ", Nature: " + std::string(bdsp_nature_name(nature));
    ret += ", Ability: " + std::to_string(ability);
    ret += ", " + std::string(bdsp_gender_name(gender));
    ret += ", " + std::string(bdsp_shiny_name(shiny));
    ret += ", PID: 0x" + tostr_hex_padded(8, pid);
    ret += ", EC: 0x" + tostr_hex_padded(8, ec);
    return ret;
}

std::string BdspIdResult::to_string() const{
    return "TID: " + std::to_string(tid)
        + ", SID: " + std::to_string(sid)
        + ", Displayed TID: " + tostr_padded(6, display_tid)
        + ", TSV: " + std::to_string(tsv());
}


//  The game's own nature order. NatureCheckerValue uses a different one.
static const std::array<const char*, 25> NATURE_NAMES{
    "Hardy",   "Lonely",  "Brave",   "Adamant", "Naughty",
    "Bold",    "Docile",  "Relaxed", "Impish",  "Lax",
    "Timid",   "Hasty",   "Serious", "Jolly",   "Naive",
    "Modest",  "Mild",    "Quiet",   "Bashful", "Rash",
    "Calm",    "Gentle",  "Sassy",   "Careful", "Quirky",
};
static const std::array<NatureCheckerValue, 25> NATURE_CHECKER_VALUES{
    NatureCheckerValue::Hardy,   NatureCheckerValue::Lonely,  NatureCheckerValue::Brave,
    NatureCheckerValue::Adamant, NatureCheckerValue::Naughty, NatureCheckerValue::Bold,
    NatureCheckerValue::Docile,  NatureCheckerValue::Relaxed, NatureCheckerValue::Impish,
    NatureCheckerValue::Lax,     NatureCheckerValue::Timid,   NatureCheckerValue::Hasty,
    NatureCheckerValue::Serious, NatureCheckerValue::Jolly,   NatureCheckerValue::Naive,
    NatureCheckerValue::Modest,  NatureCheckerValue::Mild,    NatureCheckerValue::Quiet,
    NatureCheckerValue::Bashful, NatureCheckerValue::Rash,    NatureCheckerValue::Calm,
    NatureCheckerValue::Gentle,  NatureCheckerValue::Sassy,   NatureCheckerValue::Careful,
    NatureCheckerValue::Quirky,
};

const char* bdsp_nature_name(uint8_t nature){
    return nature < NATURE_NAMES.size() ? NATURE_NAMES[nature] : "?";
}
NatureCheckerValue bdsp_nature_to_checker_value(uint8_t nature){
    return nature < NATURE_CHECKER_VALUES.size()
        ? NATURE_CHECKER_VALUES[nature]
        : NatureCheckerValue::UnableToDetect;
}


BdspShiny bdsp_get_shiny(uint32_t pid, uint16_t tsv){
    uint16_t psv = (uint16_t)((pid >> 16) ^ (pid & 0xffff));
    if (psv == tsv){
        return BdspShiny::Square;
    }
    if ((tsv ^ psv) < 16){
        return BdspShiny::Star;
    }
    return BdspShiny::None;
}
bool bdsp_is_shiny(uint32_t pid, uint16_t tsv){
    uint16_t psv = (uint16_t)((pid >> 16) ^ (pid & 0xffff));
    return (tsv ^ psv) < 16;
}



//
//  Generation
//

namespace{

//  Reads straight off a generator. Used for one-off generation.
struct DirectSource{
    Xorshift128& rng;
    uint32_t next(){ return bdsp_gen_transform(rng.next()); }
    uint32_t next(uint32_t modulo){ return next() % modulo; }
};

//  Reads off a sliding window. Used when searching a range of advances, where
//  consecutive advances re-read almost all of the same values.
struct WindowSource{
    Xorshift128List<STATIC_SEARCH_WINDOW>& list;
    uint32_t next(){ return list.next_gen(); }
    uint32_t next(uint32_t modulo){ return list.next_gen() % modulo; }
};

}


//  Force the PID to be non-shiny against the player's own TSV.
static void force_not_shiny(uint32_t& pid, uint16_t tsv){
    if (bdsp_is_shiny(pid, tsv)){
        pid ^= 0x10000000;
    }
}
//  Rebuild the PID so that it is shiny against the player's own TSV, in the same
//  way (star vs. square) that it was shiny against the rolled TSV.
static void force_shiny(uint32_t& pid, uint16_t tsv, BdspShiny shiny){
    if (bdsp_get_shiny(pid, tsv) == shiny){
        return;
    }
    uint16_t high = (uint16_t)((pid & 0xffff) ^ tsv ^ (2 - (uint8_t)shiny));
    pid = ((uint32_t)high << 16) | (pid & 0xffff);
}

//  The encounter rolls its own TSV, decides shininess against that, and only then
//  is the PID reshaped to produce the same outcome against the player's real TSV.
//  This is why a shiny frame stays shiny no matter whose save file is used.
static BdspShiny resolve_shiny(uint32_t& pid, uint32_t sidtid, uint16_t tsv, bool shiny_locked){
    if (shiny_locked){
        force_not_shiny(pid, tsv);
        return BdspShiny::None;
    }

    BdspShiny shiny = bdsp_get_shiny(pid, (uint16_t)((sidtid >> 16) ^ (sidtid & 0xffff)));
    if (shiny != BdspShiny::None){
        force_shiny(pid, tsv, shiny);
    }else{
        force_not_shiny(pid, tsv);
    }
    return shiny;
}


template <typename Source>
static BdspPokemonResult generate_static_impl(
    Source&& source, const BdspStaticTemplate& tmpl,
    uint16_t tsv, uint8_t synchronize_nature
){
    BdspPokemonResult result;
    result.level = tmpl.level;

    result.ec = source.next();
    uint32_t sidtid = source.next();
    result.pid = source.next();

    result.shiny = resolve_shiny(result.pid, sidtid, tsv, tmpl.shiny_locked);

    //  Guaranteed perfect IVs first. The roll picks a slot and is rerolled if that
    //  slot is already taken, so the number of calls here is not fixed.
    const uint8_t UNSET = 255;
    BdspIVs ivs;
    for (size_t c = 0; c < 6; c++){
        ivs[c] = UNSET;
    }
    for (uint8_t c = 0; c < tmpl.guaranteed_ivs;){
        uint8_t index = (uint8_t)source.next(6);
        if (ivs[index] == UNSET){
            ivs[index] = 31;
            c++;
        }
    }
    for (size_t c = 0; c < 6; c++){
        if (ivs[c] == UNSET){
            ivs[c] = (uint8_t)source.next(32);
        }
    }
    result.ivs = ivs;

    switch (tmpl.ability_kind){
    case 0:
    case 1:
        result.ability = tmpl.ability_kind;
        break;
    case 2:
        //  Hidden ability. The roll still happens, its value is just ignored.
        result.ability = 2;
        source.next();
        break;
    default:
        result.ability = (uint8_t)source.next(2);
        break;
    }

    switch (tmpl.gender_ratio){
    case 255:
        result.gender = BdspGender::Genderless;
        break;
    case 254:
        result.gender = BdspGender::Female;
        break;
    case 0:
        result.gender = BdspGender::Male;
        break;
    default:
        result.gender = source.next(253) + 1 < tmpl.gender_ratio
            ? BdspGender::Female
            : BdspGender::Male;
        break;
    }

    //  A Synchronize lead skips the nature roll entirely rather than overriding
    //  its result, so it shifts everything after it.
    result.nature = synchronize_nature != BDSP_NO_SYNCHRONIZE
        ? synchronize_nature
        : (uint8_t)source.next(25);

    result.height = (uint8_t)source.next(129);
    result.height += (uint8_t)source.next(128);
    result.weight = (uint8_t)source.next(129);
    result.weight += (uint8_t)source.next(128);

    return result;
}


BdspPokemonResult bdsp_generate_static(
    Xorshift128 rng, const BdspStaticTemplate& tmpl,
    uint16_t tsv, uint8_t synchronize_nature
){
    return generate_static_impl(DirectSource{rng}, tmpl, tsv, synchronize_nature);
}

BdspPokemonResult bdsp_generate_roamer(
    Xorshift128 rng, const BdspStaticTemplate& tmpl,
    uint16_t tsv, uint8_t synchronize_nature
){
    BdspPokemonResult result;
    result.level = tmpl.level;

    //  A roamer takes exactly one value from the main RNG. Everything else comes
    //  from a Xoroshiro seeded with it, which is why roamers cost one advance
    //  regardless of how much they generate.
    result.ec = bdsp_gen_transform(rng.next());
    XoroshiroBDSP roamer(result.ec);

    uint32_t sidtid = roamer.next_uint(0xffffffff);
    result.pid = roamer.next_uint(0xffffffff);

    //  Both roamers (Cresselia, Mesprit) are shiny-capable.
    result.shiny = resolve_shiny(result.pid, sidtid, tsv, false);

    const uint8_t UNSET = 255;
    BdspIVs ivs;
    for (size_t c = 0; c < 6; c++){
        ivs[c] = UNSET;
    }
    for (uint8_t c = 0; c < tmpl.guaranteed_ivs;){
        uint8_t index = (uint8_t)roamer.next_uint(6);
        if (ivs[index] == UNSET){
            ivs[index] = 31;
            c++;
        }
    }
    for (size_t c = 0; c < 6; c++){
        if (ivs[c] == UNSET){
            ivs[c] = (uint8_t)roamer.next_uint(32);
        }
    }
    result.ivs = ivs;

    //  Neither roamer can have a hidden ability, so this path has no equivalent of
    //  the ability_kind switch above.
    result.ability = (uint8_t)roamer.next_uint(2);

    result.nature = synchronize_nature != BDSP_NO_SYNCHRONIZE
        ? synchronize_nature
        : (uint8_t)roamer.next_uint(25);

    result.height = (uint8_t)roamer.next_uint(129);
    result.height += (uint8_t)roamer.next_uint(128);
    result.weight = (uint8_t)roamer.next_uint(129);
    result.weight += (uint8_t)roamer.next_uint(128);

    //  Roamer gender is fixed per species and is never rolled.
    result.gender = tmpl.gender_ratio == 254
        ? BdspGender::Female
        : BdspGender::Genderless;

    return result;
}

BdspPokemonResult bdsp_generate(
    Xorshift128 rng, const BdspStaticTemplate& tmpl,
    uint16_t tsv, uint8_t synchronize_nature
){
    return tmpl.roamer
        ? bdsp_generate_roamer(rng, tmpl, tsv, synchronize_nature)
        : bdsp_generate_static(rng, tmpl, tsv, synchronize_nature);
}


BdspIdResult bdsp_generate_id(Xorshift128 rng){
    BdspIdResult result;
    do{
        result.sidtid = bdsp_gen_transform(rng.next());
    }while (result.sidtid == 0);

    result.tid = (uint16_t)(result.sidtid & 0xffff);
    result.sid = (uint16_t)(result.sidtid >> 16);
    result.display_tid = result.sidtid % 1000000;
    return result;
}



//
//  Filtering and searching
//

static bool iv_in_range(const IvRange& range, uint8_t iv){
    if (range.low >= 0 && iv < (uint8_t)range.low){
        return false;
    }
    if (range.high >= 0 && iv > (uint8_t)range.high){
        return false;
    }
    return true;
}


BdspStaticSearcher::BdspStaticSearcher(
    const Xorshift128State& base_state,
    BdspStaticTemplate tmpl,
    uint16_t tsv,
    uint8_t synchronize_nature
)
    : m_base_state(base_state)
    , m_template(std::move(tmpl))
    , m_tsv(tsv)
    , m_synchronize_nature(synchronize_nature)
{}

BdspPokemonResult BdspStaticSearcher::generate(uint64_t advances) const{
    Xorshift128 rng(m_base_state);
    rng.advance(advances);
    return bdsp_generate(rng, m_template, m_tsv, m_synchronize_nature);
}

std::vector<BdspRngHit> BdspStaticSearcher::scan(
    uint64_t min_advances, uint64_t max_advances,
    const std::function<bool(const BdspPokemonResult&)>& accept,
    bool stop_at_first,
    Cancellable* cancellable
) const{
    std::vector<BdspRngHit> hits;
    if (min_advances > max_advances){
        return hits;
    }

    //  Tracks the state at the start of each advance so that a hit can report it.
    Xorshift128 rng(m_base_state);
    rng.advance(min_advances);

    //  A roamer reseeds a Xoroshiro on every advance, so there is nothing to
    //  cache and the sliding window would only get in the way.
    if (m_template.roamer){
        for (uint64_t advances = min_advances; advances <= max_advances; advances++){
            if (cancellable != nullptr && (advances % CANCEL_CHECK_INTERVAL) == 0){
                cancellable->throw_if_cancelled();
            }
            BdspPokemonResult result = bdsp_generate_roamer(
                rng, m_template, m_tsv, m_synchronize_nature
            );
            if (accept(result)){
                hits.emplace_back(BdspRngHit{advances, rng.state(), result});
                if (stop_at_first){
                    return hits;
                }
            }
            rng.next();
        }
        return hits;
    }

    Xorshift128List<STATIC_SEARCH_WINDOW> list(rng);
    for (uint64_t advances = min_advances; advances <= max_advances; advances++, list.advance_state()){
        if (cancellable != nullptr && (advances % CANCEL_CHECK_INTERVAL) == 0){
            cancellable->throw_if_cancelled();
        }
        BdspPokemonResult result = generate_static_impl(
            WindowSource{list}, m_template, m_tsv, m_synchronize_nature
        );
        if (accept(result)){
            hits.emplace_back(BdspRngHit{advances, rng.state(), result});
            if (stop_at_first){
                return hits;
            }
        }
        rng.next();
    }
    return hits;
}





}
}
