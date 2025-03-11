/*  Stats Calculation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pokemon_StatsCalculation_H
#define PokemonAutomation_Pokemon_StatsCalculation_H

#include <stdint.h>
#include "Pokemon_NatureChecker.h"

namespace PokemonAutomation{
namespace Pokemon{



struct BaseStats{
    uint8_t hp;
    uint8_t attack;
    uint8_t defense;
    uint8_t spatk;
    uint8_t spdef;
    uint8_t speed;
};
struct EVs{
    uint8_t hp = 0;
    uint8_t attack = 0;
    uint8_t defense = 0;
    uint8_t spatk = 0;
    uint8_t spdef = 0;
    uint8_t speed = 0;
};


struct StatReads{
    int16_t hp = -1;
    int16_t attack = -1;
    int16_t defense = -1;
    int16_t spatk = -1;
    int16_t spdef = -1;
    int16_t speed = -1;
};

struct IvRange{
    int8_t low = -1;
    int8_t high = -1;
};
struct IvRanges{
    IvRange hp;
    IvRange attack;
    IvRange defense;
    IvRange spatk;
    IvRange spdef;
    IvRange speed;
};


enum class NatureAdjustment{
    NEUTRAL,
    NEGATIVE,
    POSITIVE,
};
struct NatureAdjustments{
    NatureAdjustment attack;
    NatureAdjustment defense;
    NatureAdjustment spatk;
    NatureAdjustment spdef;
    NatureAdjustment speed;
};
//NatureAdjustments get_nature_adjustments(NatureCheckerValue nature);




uint16_t calc_stats_hp(uint8_t base_stat, uint8_t level, uint8_t iv, uint8_t ev);
uint16_t calc_stats_nonhp(uint8_t base_stat, uint8_t level, uint8_t iv, uint8_t ev, NatureAdjustment nature);


#if 0
bool calc_iv_range(
    uint8_t& low_iv, uint8_t& high_iv,
    bool is_hp, uint8_t base_stat, uint8_t level, uint8_t ev,
    uint16_t stat, NatureAdjustment nature
);
#endif
IvRange calc_iv_range(
    bool is_hp, uint8_t base_stat, uint8_t level, uint8_t ev,
    uint16_t stat, NatureAdjustment nature
);


IvRanges calc_iv_ranges(
    const BaseStats& base_stats, uint8_t level, const EVs& evs,
    const StatReads& actual_stats, const NatureAdjustments& natures
);



}
}
#endif
