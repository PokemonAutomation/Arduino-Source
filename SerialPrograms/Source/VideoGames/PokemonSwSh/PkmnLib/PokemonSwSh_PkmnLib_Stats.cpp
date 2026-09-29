/*  PkmnLib Stats
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include "PokemonSwSh_PkmnLib_Stats.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace papkmnlib{


uint16_t calc_hp(uint16_t baseHP, uint16_t hpIV, uint16_t hpEV, uint16_t level){
    // note: the formula includes flooring, but uint16 math just floors it
    // automatically, nothing to worry about here

    uint16_t fraction_term = (((2 * baseHP + hpIV + (hpEV / 4)) * level) / 100);

    return fraction_term + level + 10;
}

uint16_t calc_stat(uint16_t baseStat, uint16_t IV, uint16_t EV, uint16_t level, char nature){
    double nat_mod = 1.0;

    // switch up based on the nature value
    switch (nature){
    case 'p':
        nat_mod = 1.1;
        break;
    case 'n':
        nat_mod = 0.9;
        break;
    default:
        nat_mod = 1.0;
        break;
    }

    // note, there are quite a few floors, but that happens with int math anyway
    uint16_t fraction_term = (((2 * baseStat + IV + (EV / 4)) * level) / 100) + 5;

    // return and make sure we cast it to an int on output
    return (uint16_t)((double)fraction_term * nat_mod);
}




}
}
}
}
