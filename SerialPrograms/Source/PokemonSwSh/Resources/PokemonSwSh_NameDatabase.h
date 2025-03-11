/*  Name Database
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_NameDatabase_H
#define PokemonAutomation_PokemonSwSh_NameDatabase_H

#include <set>
#include "CommonTools/Options/StringSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

const std::set<std::string>& ALL_POKEMON_SLUGS();

StringSelectDatabase make_name_database(const std::vector<std::string>& slugs);
StringSelectDatabase make_name_database(const char* json_file_slugs);

const StringSelectDatabase& COMBINED_DEX_NAMES();




}
}
}
#endif
