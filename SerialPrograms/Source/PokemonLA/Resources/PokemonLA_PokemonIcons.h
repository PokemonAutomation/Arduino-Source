/*  Pokemon LA Icons
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_PokemonIcons_H
#define PokemonAutomation_PokemonLA_PokemonIcons_H

#include <string>
#include <map>
#include <QIcon>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

// All pokemon sprites in LA
const std::map<std::string, QIcon>& ALL_POKEMON_ICONS();

}
}
}
#endif
