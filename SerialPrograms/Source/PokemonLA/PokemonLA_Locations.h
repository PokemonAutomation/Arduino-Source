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


enum class WarpSpot{
    FIELDLANDS_FIELDLANDS,
    FIELDLANDS_HEIGHTS,
    FIELDLANDS_GRANDTREE_ARENA,
    MIRELANDS_MIRELANDS,
    MIRELANDS_BOGBOUND,
    MIRELANDS_DIAMOND_SETTLEMENT,
    MIRELANDS_BRAVA_ARENA,
    COASTLANDS_BEACHSIDE,
    COASTLANDS_COASTLANDS,
    COASTLANDS_MOLTEN_ARENA,
    HIGHLANDS_HIGHLANDS,
    HIGHLANDS_MOUNTAIN,
    HIGHLANDS_SUMMIT,
    HIGHLANDS_MOONVIEW_ARENA,
    ICELANDS_SNOWFIELDS,
    ICELANDS_ICEPEAK,
    ICELANDS_PEARL_SETTLEMENT,
    ICELANDS_ICEPEAK_ARENA,
    END_LIST,
};
extern const char* WARP_SPOT_NAMES[];
WarpSpot warpspot_name_to_enum(const std::string& name);



}
}
}
#endif
