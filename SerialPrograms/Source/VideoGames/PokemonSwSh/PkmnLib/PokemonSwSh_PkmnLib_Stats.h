/*  PkmnLib Stats
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef _PokemonAutomation_PokemonSwSh_PkmnLib_Stats_H
#define _PokemonAutomation_PokemonSwSh_PkmnLib_Stats_H

#include <stdint.h>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace papkmnlib{


uint16_t calc_hp(uint16_t baseHP, uint16_t hpIV, uint16_t hpEV, uint16_t level);
uint16_t calc_stat(uint16_t baseStat, uint16_t IV, uint16_t EV, uint16_t level, char nature = 'u');


}
}
}
}
#endif
