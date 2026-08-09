/*  BDSP RNG Calibration
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <vector>
#include "PokemonBDSP_RngCalibration.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Pokemon;


NatureAdjustment bdsp_nature_adjustment(uint8_t nature, size_t stat_index){
    if (nature >= 25 || stat_index >= 5){
        return NatureAdjustment::NEUTRAL;
    }
    size_t raised = nature / 5;
    size_t lowered = nature % 5;
    if (raised == lowered){
        //  Hardy, Docile, Serious, Bashful, Quirky.
        return NatureAdjustment::NEUTRAL;
    }
    if (stat_index == raised){
        return NatureAdjustment::POSITIVE;
    }
    if (stat_index == lowered){
        return NatureAdjustment::NEGATIVE;
    }
    return NatureAdjustment::NEUTRAL;
}


//  The nature index orders stats [Atk, Def, Spe, SpA, SpD];
static const size_t NATURE_INDEX_ATTACK  = 0;
static const size_t NATURE_INDEX_DEFENSE = 1;
static const size_t NATURE_INDEX_SPEED   = 2;
static const size_t NATURE_INDEX_SPATK   = 3;
static const size_t NATURE_INDEX_SPDEF   = 4;


StatReads bdsp_expected_stats(
    const BdspPokemonResult& generated,
    const BaseStats& base_stats,
    uint8_t level
){
    StatReads ret;
    ret.hp = (int16_t)calc_stats_hp(base_stats.hp, level, generated.ivs.hp, 0);
    ret.attack = (int16_t)calc_stats_nonhp(
        base_stats.attack, level, generated.ivs.attack, 0,
        bdsp_nature_adjustment(generated.nature, NATURE_INDEX_ATTACK)
    );
    ret.defense = (int16_t)calc_stats_nonhp(
        base_stats.defense, level, generated.ivs.defense, 0,
        bdsp_nature_adjustment(generated.nature, NATURE_INDEX_DEFENSE)
    );
    ret.spatk = (int16_t)calc_stats_nonhp(
        base_stats.spatk, level, generated.ivs.spatk, 0,
        bdsp_nature_adjustment(generated.nature, NATURE_INDEX_SPATK)
    );
    ret.spdef = (int16_t)calc_stats_nonhp(
        base_stats.spdef, level, generated.ivs.spdef, 0,
        bdsp_nature_adjustment(generated.nature, NATURE_INDEX_SPDEF)
    );
    ret.speed = (int16_t)calc_stats_nonhp(
        base_stats.speed, level, generated.ivs.speed, 0,
        bdsp_nature_adjustment(generated.nature, NATURE_INDEX_SPEED)
    );
    return ret;
}


bool advances_between(
    const Xorshift128State& from,
    const Xorshift128State& to,
    uint64_t search_max,
    uint64_t& advances
){
    Xorshift128 rng(from);
    for (uint64_t c = 0; c <= search_max; c++){
        if (rng.state() == to){
            advances = c;
            return true;
        }
        rng.next();
    }
    return false;
}


const BaseStats& starter_base_stats(BdspStarter starter){
    //  Order: HP, Atk, Def, SpA, SpD, Spe.
    static const BaseStats TURTWIG {55, 68, 64, 45, 55, 31};
    static const BaseStats CHIMCHAR{44, 58, 44, 58, 44, 61};
    static const BaseStats PIPLUP  {53, 51, 53, 61, 56, 40};
    switch (starter){
    case BdspStarter::Turtwig:  return TURTWIG;
    case BdspStarter::Chimchar: return CHIMCHAR;
    case BdspStarter::Piplup:   return PIPLUP;
    }
    return TURTWIG;
}


//  A reading of -1 was not taken, so it rules nothing out.
static bool stat_fits(int16_t observed, int16_t expected){
    return observed < 0 || observed == expected;
}


bool consistent_with(
    const BdspPokemonResult& generated,
    const BdspObservedStarter& observed
){
    if (observed.nature != NatureCheckerValue::UnableToDetect
        && bdsp_nature_to_checker_value(generated.nature) != observed.nature
    ){
        return false;
    }
    if (observed.gender_known && generated.gender != observed.gender){
        return false;
    }
    if (observed.shiny_known && (generated.shiny != BdspShiny::None) != observed.shiny){
        return false;
    }

    StatReads expected = bdsp_expected_stats(generated, observed.base_stats, observed.level);
    return stat_fits(observed.stats.hp,      expected.hp)
        && stat_fits(observed.stats.attack,  expected.attack)
        && stat_fits(observed.stats.defense, expected.defense)
        && stat_fits(observed.stats.spatk,   expected.spatk)
        && stat_fits(observed.stats.spdef,   expected.spdef)
        && stat_fits(observed.stats.speed,   expected.speed);
}


BdspHitIdentification identify_hit_advance(
    const Xorshift128State& state,
    const BdspStaticTemplate& tmpl,
    uint64_t intended_advance,
    uint64_t radius,
    const BdspObservedStarter& observed
){
    BdspHitIdentification ret;

    uint64_t first = intended_advance > radius ? intended_advance - radius : 0;
    uint64_t last = intended_advance + radius;

    BdspStaticSearcher searcher(state, tmpl, 0);

    std::vector<BdspRngHit> matches = searcher.scan(
        first, last,
        [&](const BdspPokemonResult& candidate){ return consistent_with(candidate, observed); }
    );
    ret.candidates = matches.size();

    if (ret.candidates == 0){
        ret.failure_reason = "no advance within " + std::to_string(radius)
            + " of " + std::to_string(intended_advance)
            + " produces that Pokemon. Either the recovered state was wrong, or the press "
            "missed by more than the search covered.";
        return ret;
    }
    if (ret.candidates > 1){
        ret.failure_reason = std::to_string(ret.candidates)
            + " advances fit what was read (";
        for (size_t c = 0; c < matches.size(); c++){
            ret.failure_reason += (c == 0 ? "" : ", ") + std::to_string(matches[c].advances);
        }
        ret.failure_reason += "), so which one was hit cannot be told apart here.";
        return ret;
    }

    ret.success = true;
    ret.advance = matches[0].advances;
    ret.offset = (int64_t)ret.advance - (int64_t)intended_advance;
    return ret;
}




}
}
}
