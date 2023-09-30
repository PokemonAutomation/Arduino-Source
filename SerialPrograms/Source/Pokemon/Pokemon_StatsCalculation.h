/*  Stats Calculation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_StatsCalculation_H
#define PokemonAutomation_Pokemon_StatsCalculation_H

#include <stdint.h>

namespace PokemonAutomation{
namespace Pokemon{



enum class NatureAdjustment{
    NEUTRAL,
    POSITIVE,
    NEGATIVE,
};

uint16_t calc_stats_hp(uint8_t base_stat, uint8_t level, uint8_t iv, uint8_t ev);
uint16_t calc_stats_nonhp(uint8_t base_stat, uint8_t level, uint8_t iv, uint8_t ev, NatureAdjustment nature);


bool calc_iv_range(
    uint8_t& low_iv, uint8_t& high_iv,
    bool is_hp, uint8_t base_stat, uint8_t level, uint8_t ev,
    uint16_t stat, NatureAdjustment nature
);




}
}
#endif
