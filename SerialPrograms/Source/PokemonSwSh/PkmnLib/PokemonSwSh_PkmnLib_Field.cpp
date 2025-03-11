/*  PkmnLib Field
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include "PokemonSwSh_PkmnLib_Field.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace papkmnlib{


//  Hack to apply default field since we have no field detection.
void Field::set_default_field(const std::string& boss){
    static std::map<std::string, std::pair<Weather, Terrain>> map{
        {"kyogre", {Weather::RAIN, Terrain::ELECTRIC}},
        {"groudon", {Weather::SUN, Terrain::NONE}},
        {"palkia", {Weather::RAIN, Terrain::NONE}},
    };
    auto iter = map.find(boss);
    if (iter == map.end()){
        return;
    }
    m_current_weather = iter->second.first;
    m_current_terrain = iter->second.second;
}


}
}
}
}
