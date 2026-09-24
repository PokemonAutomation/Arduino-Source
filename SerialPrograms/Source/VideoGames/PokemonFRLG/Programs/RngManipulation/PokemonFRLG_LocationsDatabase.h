/*  Locations Database
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_LocationsDatabase_H
#define PokemonAutomation_PokemonFRLG_LocationsDatabase_H

#include "CommonTools/Options/StringSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


StringSelectDatabase make_locations_database(const char* json_file_slugs);



}
}
}
#endif
