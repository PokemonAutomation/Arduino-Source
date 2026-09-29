/*  Pokemon Legends Arceus Locations
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include "Common/Cpp/Exceptions.h"
#include "PokemonLA_Locations.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



const char* MAP_REGION_NAMES[] = {
    "None",
    "Jubilife Village",
    "Obsidian Fieldlands",
    "Crimson Mirelands",
    "Cobalt Coastlands",
    "Coronet Highlands",
    "Alabaster Icelands",
    "Ancient Retreat",
};

const char* WILD_REGION_SHORT_NAMES[] = {
    "Fieldlands",
    "Mirelands",
    "Coastlands",
    "Highlands",
    "Icelands",
};

bool is_wild_land(MapRegion region){
    return region == MapRegion::FIELDLANDS || region == MapRegion::MIRELANDS || region == MapRegion::COASTLANDS
           || region == MapRegion::HIGHLANDS || region == MapRegion::ICELANDS;
}

Camp map_region_default_camp(MapRegion region){
    switch (region){
    case MapRegion::FIELDLANDS:
        return Camp::FIELDLANDS_FIELDLANDS;
    case MapRegion::MIRELANDS:
        return Camp::MIRELANDS_MIRELANDS;
    case MapRegion::COASTLANDS:
        return Camp::COASTLANDS_BEACHSIDE;
    case MapRegion::HIGHLANDS:
        return Camp::HIGHLANDS_HIGHLANDS;
    case MapRegion::ICELANDS:
        return Camp::ICELANDS_SNOWFIELDS;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid region.");
    }

}

}
}
}
