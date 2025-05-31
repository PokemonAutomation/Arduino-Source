/*  Stats Calculation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Pokemon_StatsCalculation.h"

namespace PokemonAutomation{
namespace Pokemon{


#if 0
NatureAdjustments get_nature_adjustments(NatureCheckerValue nature){
    NatureAdjustments ret;
    ret.attack = NatureAdjustment::NEUTRAL;
    ret.defense = NatureAdjustment::NEUTRAL;
    ret.spatk = NatureAdjustment::NEUTRAL;
    ret.spdef = NatureAdjustment::NEUTRAL;
    ret.speed = NatureAdjustment::NEUTRAL;

    switch (nature){
    case NatureCheckerValue::UnableToDetect:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Cannot query invalid nature.");

    case NatureCheckerValue::Neutral:
    case NatureCheckerValue::Bashful:
    case NatureCheckerValue::Docile:
    case NatureCheckerValue::Hardy:
    case NatureCheckerValue::Quirky:
    case NatureCheckerValue::Serious:
        return ret;

    case NatureCheckerValue::Bold:
        ret.attack = NatureAdjustment::NEGATIVE;
        ret.defense = NatureAdjustment::POSITIVE;
        return ret;
    case NatureCheckerValue::Modest:
        ret.attack = NatureAdjustment::NEGATIVE;
        ret.spatk = NatureAdjustment::POSITIVE;
        return ret;
    case NatureCheckerValue::Calm:
        ret.attack = NatureAdjustment::NEGATIVE;
        ret.spdef = NatureAdjustment::POSITIVE;
        return ret;
    case NatureCheckerValue::Timid:
        ret.attack = NatureAdjustment::NEGATIVE;
        ret.speed = NatureAdjustment::POSITIVE;
        return ret;

    case NatureCheckerValue::Lonely:
        ret.defense = NatureAdjustment::NEGATIVE;
        ret.attack = NatureAdjustment::POSITIVE;
        return ret;
    case NatureCheckerValue::Mild:
        ret.defense = NatureAdjustment::NEGATIVE;
        ret.spatk = NatureAdjustment::POSITIVE;
        return ret;
    case NatureCheckerValue::Gentle:
        ret.defense = NatureAdjustment::NEGATIVE;
        ret.spdef = NatureAdjustment::POSITIVE;
        return ret;
    case NatureCheckerValue::Hasty:
        ret.defense = NatureAdjustment::NEGATIVE;
        ret.speed = NatureAdjustment::POSITIVE;
        return ret;

    case NatureCheckerValue::Adamant:
        ret.spatk = NatureAdjustment::NEGATIVE;
        ret.attack = NatureAdjustment::POSITIVE;
        return ret;
    case NatureCheckerValue::Impish:
        ret.spatk = NatureAdjustment::NEGATIVE;
        ret.defense = NatureAdjustment::POSITIVE;
        return ret;
    case NatureCheckerValue::Careful:
        ret.spatk = NatureAdjustment::NEGATIVE;
        ret.spdef = NatureAdjustment::POSITIVE;
        return ret;
    case NatureCheckerValue::Jolly:
        ret.spatk = NatureAdjustment::NEGATIVE;
        ret.speed = NatureAdjustment::POSITIVE;
        return ret;

    case NatureCheckerValue::Naughty:
        ret.spdef = NatureAdjustment::NEGATIVE;
        ret.attack = NatureAdjustment::POSITIVE;
        return ret;
    case NatureCheckerValue::Lax:
        ret.spdef = NatureAdjustment::NEGATIVE;
        ret.defense = NatureAdjustment::POSITIVE;
        return ret;
    case NatureCheckerValue::Rash:
        ret.spdef = NatureAdjustment::NEGATIVE;
        ret.spatk = NatureAdjustment::POSITIVE;
        return ret;
    case NatureCheckerValue::Naive:
        ret.spdef = NatureAdjustment::NEGATIVE;
        ret.speed = NatureAdjustment::POSITIVE;
        return ret;

    case NatureCheckerValue::Brave:
        ret.speed = NatureAdjustment::NEGATIVE;
        ret.attack = NatureAdjustment::POSITIVE;
        return ret;
    case NatureCheckerValue::Relaxed:
        ret.speed = NatureAdjustment::NEGATIVE;
        ret.defense = NatureAdjustment::POSITIVE;
        return ret;
    case NatureCheckerValue::Quiet:
        ret.speed = NatureAdjustment::NEGATIVE;
        ret.spatk = NatureAdjustment::POSITIVE;
        return ret;
    case NatureCheckerValue::Sassy:
        ret.speed = NatureAdjustment::NEGATIVE;
        ret.spdef = NatureAdjustment::POSITIVE;
        return ret;

    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unknown Nature: " + std::to_string((int)nature));
    }
}
#endif


uint16_t calc_stats_hp(uint8_t base_stat, uint8_t level, uint8_t iv, uint8_t ev){
    uint16_t stat = (uint16_t)2 * base_stat + iv + ev/4;
    stat *= level;
    stat /= 100;
    stat += level;
    stat += 10;
    return stat;
}
uint16_t calc_stats_nonhp(uint8_t base_stat, uint8_t level, uint8_t iv, uint8_t ev, NatureAdjustment nature){
    uint16_t stat = (uint16_t)2 * base_stat + iv + ev/4;
    stat *= level;
    stat /= 100;
    stat += 5;
    switch (nature){
    case NatureAdjustment::NEUTRAL:
        break;
    case NatureAdjustment::POSITIVE:
        stat *= 110;
        stat /= 100;
        break;
    case NatureAdjustment::NEGATIVE:
        stat *= 90;
        stat /= 100;
        break;
    }
    return stat;
}


#if 0
bool calc_iv_range(
    uint8_t& low_iv, uint8_t& high_iv,
    bool is_hp, uint8_t base_stat, uint8_t level, uint8_t ev,
    uint16_t stat, NatureAdjustment nature
){
    bool set = false;
    for (uint8_t iv = 0; iv < 32; iv++){
        uint16_t current_stat = is_hp
            ? calc_stats_hp(base_stat, level, iv, ev)
            : calc_stats_nonhp(base_stat, level, iv, ev, nature);

        if (stat != current_stat){
            continue;
        }

        if (!set){
            set = true;
            low_iv = iv;
            high_iv = iv;
            continue;
        }

        if (low_iv > iv){
            low_iv = iv;
        }
        if (high_iv < iv){
            high_iv = iv;
        }
    }

    return set;
}
#endif
IvRange calc_iv_range(
    bool is_hp, uint8_t base_stat, uint8_t level, uint8_t ev,
    uint16_t stat, NatureAdjustment nature
){
    IvRange ret;

    bool set = false;
    for (uint8_t iv = 0; iv < 32; iv++){
        uint16_t current_stat = is_hp
            ? calc_stats_hp(base_stat, level, iv, ev)
            : calc_stats_nonhp(base_stat, level, iv, ev, nature);

        if (stat != current_stat){
            continue;
        }

        if (!set){
            set = true;
            ret.low = iv;
            ret.high = iv;
            continue;
        }

        if ((uint8_t)ret.low > iv){
            ret.low = iv;
        }
        if ((uint8_t)ret.high < iv){
            ret.high = iv;
        }
    }

    return ret;
}


IvRanges calc_iv_ranges(
    const BaseStats& base_stats, uint8_t level, const EVs& evs,
    const StatReads& actual_stats, const NatureAdjustments& natures
){
    IvRanges ret;

    if (actual_stats.hp >= 10){
        ret.hp = calc_iv_range(true, base_stats.hp, level, evs.hp, actual_stats.hp, NatureAdjustment::NEUTRAL);
    }
    if (actual_stats.attack >= 5){
        ret.attack = calc_iv_range(false, base_stats.attack, level, evs.attack, actual_stats.attack, natures.attack);
    }
    if (actual_stats.defense >= 5){
        ret.defense = calc_iv_range(false, base_stats.defense, level, evs.defense, actual_stats.defense, natures.defense);
    }
    if (actual_stats.spatk >= 5){
        ret.spatk = calc_iv_range(false, base_stats.spatk, level, evs.spatk, actual_stats.spatk, natures.spatk);
    }
    if (actual_stats.spdef >= 5){
        ret.spdef = calc_iv_range(false, base_stats.spdef, level, evs.spdef, actual_stats.spdef, natures.spdef);
    }
    if (actual_stats.speed >= 5){
        ret.speed = calc_iv_range(false, base_stats.speed, level, evs.speed, actual_stats.speed, natures.speed);
    }

    return ret;
}







}
}
