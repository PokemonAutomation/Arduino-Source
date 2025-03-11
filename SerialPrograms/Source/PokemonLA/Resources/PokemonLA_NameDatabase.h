/*  Name Database
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_NameDatabase_H
#define PokemonAutomation_PokemonLA_NameDatabase_H

#include "CommonTools/Options/StringSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


StringSelectDatabase make_name_database(const std::vector<std::string>& slugs);
StringSelectDatabase make_name_database(const char* json_file_slugs);

const StringSelectDatabase& ALL_POKEMON_NAMES();




}
}
}
#endif
