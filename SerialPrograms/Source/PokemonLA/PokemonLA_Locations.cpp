/*  Pokemon Legends Arceus Locations
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include "PokemonLA_Locations.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


const char* WARP_SPOT_NAMES[] = {
    "Obsidian Fieldlands - Fieldlands Camp",
    "Obsidian Fieldlands - Heights Camp",
    "Obsidian Fieldlands - Grandtree Arena",
    "Crimson Mirelands - Mirelands Camp",
    "Crimson Mirelands - Bogbound Camp",
    "Crimson Mirelands - Diamond Settlement",
    "Crimson Mirelands - Brava Arena",
    "Cobalt Coastlands - Beachside Camp",
    "Cobalt Coastlands - Coastlands Camp",
    "Cobalt Coastlands - Molten Arena",
    "Coronet Highlands - Highlands Camp",
    "Coronet Highlands - Mountain Camp",
    "Coronet Highlands - Summit Camp",
    "Coronet Highlands - Moonview Arena",
    "Alabaster Icelands - Snowfields Camp",
    "Alabaster Icelands - Icepeak Camp",
    "Alabaster Icelands - Pearl Settlement",
    "Alabaster Icelands - Icepeak Arena",
};
WarpSpot warpspot_name_to_enum(const std::string& name){
    static const std::map<std::string, WarpSpot> map{
        {WARP_SPOT_NAMES[ 0], WarpSpot::FIELDLANDS_FIELDLANDS},
        {WARP_SPOT_NAMES[ 1], WarpSpot::FIELDLANDS_HEIGHTS},
        {WARP_SPOT_NAMES[ 2], WarpSpot::FIELDLANDS_GRANDTREE_ARENA},
        {WARP_SPOT_NAMES[ 3], WarpSpot::MIRELANDS_MIRELANDS},
        {WARP_SPOT_NAMES[ 4], WarpSpot::MIRELANDS_BOGBOUND},
        {WARP_SPOT_NAMES[ 5], WarpSpot::MIRELANDS_DIAMOND_SETTLEMENT},
        {WARP_SPOT_NAMES[ 6], WarpSpot::MIRELANDS_BRAVA_ARENA},
        {WARP_SPOT_NAMES[ 7], WarpSpot::COASTLANDS_BEACHSIDE},
        {WARP_SPOT_NAMES[ 8], WarpSpot::COASTLANDS_COASTLANDS},
        {WARP_SPOT_NAMES[ 9], WarpSpot::COASTLANDS_MOLTEN_ARENA},
        {WARP_SPOT_NAMES[10], WarpSpot::HIGHLANDS_HIGHLANDS},
        {WARP_SPOT_NAMES[11], WarpSpot::HIGHLANDS_MOUNTAIN},
        {WARP_SPOT_NAMES[12], WarpSpot::HIGHLANDS_SUMMIT},
        {WARP_SPOT_NAMES[13], WarpSpot::HIGHLANDS_MOONVIEW_ARENA},
        {WARP_SPOT_NAMES[14], WarpSpot::ICELANDS_SNOWFIELDS},
        {WARP_SPOT_NAMES[15], WarpSpot::ICELANDS_ICEPEAK},
        {WARP_SPOT_NAMES[16], WarpSpot::ICELANDS_PEARL_SETTLEMENT},
        {WARP_SPOT_NAMES[17], WarpSpot::ICELANDS_ICEPEAK_ARENA},
    };
    auto iter = map.find(name);
    if (iter != map.end()){
        return iter->second;
    }
    return WarpSpot::END_LIST;
}




}
}
}
