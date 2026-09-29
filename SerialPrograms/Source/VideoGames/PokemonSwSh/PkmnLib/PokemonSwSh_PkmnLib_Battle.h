/*  PkmnLib Battle
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef _PokemonAutomation_PokemonSwSh_PkmnLib_Battle_H
#define _PokemonAutomation_PokemonSwSh_PkmnLib_Battle_H

#include "PokemonSwSh_PkmnLib_Field.h"
#include "PokemonSwSh_PkmnLib_Pokemon.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace papkmnlib{


void calc_damage_range(
    uint8_t power,
    uint8_t level, uint16_t attack, uint16_t defense, double modifier,
    uint16_t& lower_bound, uint16_t& upper_bound
);


double damage_score(
    const Pokemon& attacker, const Pokemon& defender,
    size_t moveIdx, const Field& field, bool multipleTargets = false
);


}
}
}
}
#endif
