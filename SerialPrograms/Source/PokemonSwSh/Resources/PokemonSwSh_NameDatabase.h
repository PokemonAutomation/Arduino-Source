/*  Name Database
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_NameDatabase_H
#define PokemonAutomation_PokemonSwSh_NameDatabase_H

#include "CommonFramework/Options/StringSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


StringSelectDatabase make_name_database(const std::vector<std::string>& slugs);
StringSelectDatabase make_name_database(const char* json_file_slugs);

const StringSelectDatabase& COMBINED_DEX_NAMES();




}
}
}
#endif
