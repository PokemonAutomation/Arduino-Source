/*  Stats Calculation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Pokemon_StatsCalculation.h"

namespace PokemonAutomation{
namespace Pokemon{



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





}
}
