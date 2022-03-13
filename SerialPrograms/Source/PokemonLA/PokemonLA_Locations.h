/*  Pokemon Legends Arceus Locations
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_Locations_H
#define PokemonAutomation_PokemonLA_Locations_H

#include <string>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



enum class MapRegion{
    NONE,
    JUBILIFE,
    FIELDLANDS,
    MIRELANDS,
    COASTLANDS,
    HIGHLANDS,
    ICELANDS,
    RETREAT,
};
extern const char* MAP_REGION_NAMES[];


enum class Camp{
    FIELDLANDS_FIELDLANDS,
    FIELDLANDS_HEIGHTS,
    MIRELANDS_MIRELANDS,
    MIRELANDS_BOGBOUND,
    COASTLANDS_BEACHSIDE,
    COASTLANDS_COASTLANDS,
    HIGHLANDS_HIGHLANDS,
    HIGHLANDS_MOUNTAIN,
    HIGHLANDS_SUMMIT,
    ICELANDS_SNOWFIELDS,
    ICELANDS_ICEPEAK,
};




}
}
}
#endif
