/*  Pokemon Sword/Shield Type Matchup
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_TypeMatchup_H
#define PokemonAutomation_PokemonSwSh_TypeMatchup_H

#include "Pokemon/Pokemon_Types.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Pokemon;


double type_multiplier(PokemonType attack_type, PokemonType defend_type0, PokemonType defend_type1);



}
}
}
#endif
